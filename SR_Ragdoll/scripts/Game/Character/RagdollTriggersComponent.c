[ComponentEditorProps(description: "Triggers for ragdolling the player.")]
class RagdollTriggersComponentClass : ScriptComponentClass {}

[BaseContainerProps()]
class RagdollTrigger : Managed
{
	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, params: "0 50 0.1", desc: "Damage required before ragdolling may occur.")]
	float MinimumRequiredDamage;
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0.0 1.0 .05", desc: "Liklihood of ragdolling when the minimum damage is applied to the Hit Zone (0-1)")]
	float OddsToTrigger;
	[Attribute(typename.EnumToString(ECharacterHitZoneGroup, ECharacterHitZoneGroup.LEFTLEG), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup), desc: "Hit zone group that can trigger ragdolling")]
	ECharacterHitZoneGroup HitZoneGroup;
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.EditBox, params: "0.001 2000000 10", desc: "Ragdoll force multiplier")]
	float ForceMultiplier;
}

class RagdollTriggersComponent : ScriptComponent
{
	[Attribute(defvalue: "0.85", uiwidget: UIWidgets.Slider, params: "0 500 0.05", desc: "How long before the player regains control from being ragdolled (seconds)")]
	protected float RagdollDurationSeconds;
	[Attribute(defvalue: "2.0", uiwidget: UIWidgets.Slider, params: "0 10 0.1", desc: "How long before the ragdoll can be triggered again after the player regains control (seconds)")]
	protected float RagdollImmunitySeconds;
	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, params: "0.1 20.0 0.1", desc: "Ragdoll effect radius")]
	protected float RagdollRadius;
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, params: "0.01 0.5 0.01", desc: "Delay before enabling ragdoll (seconds)")]
	protected float RagdollDelay;
	[Attribute("", UIWidgets.Object)]
	ref array<ref RagdollTrigger> RagdollTriggers;

	protected float									ragdollImmunityUntil;

	protected IEntity 								myOwner;
	protected ChimeraCharacter 						character;
	protected SCR_CharacterDamageManagerComponent  	damageManager;
	protected SCR_CharacterControllerComponent		characterController;
	protected SCR_CharacterAnimationComponent		animationComponent;
	protected CompartmentAccessComponent 			compartmentAccessComponent;
	protected CharacterCommandHandlerComponent		commandHandlerComponent;	
	protected TimeAndWeatherManagerEntity			timeAndWeatherManagerEntity;
	protected ScriptCallQueue 						callQueue;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		myOwner = owner;
		character = ChimeraCharacter.Cast(owner);
		if (!character)
		{
			Print("[RagdollTriggers] Failed to find ChimeraCharacter", LogLevel.WARNING);
			return;
		}

		damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageManager)
		{
			Print("[RagdollTriggers] Failed to find DamageManager", LogLevel.WARNING);
			return;
		}
		
		World world = owner.GetWorld();
		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(world);
		if (!chimeraWorld)
		{
			Print("[RagdollTriggers] Failed to find a ChimeraWorld", LogLevel.WARNING);
			return;
		}
		
		timeAndWeatherManagerEntity = chimeraWorld.GetTimeAndWeatherManager();
		
		callQueue = GetGame().GetCallqueue();
		
		if (!timeAndWeatherManagerEntity || !callQueue)
		{
			Print("[RagdollTriggers] Failed to initialize engine dependencies!", LogLevel.WARNING);
			return;
		}
		
		characterController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		animationComponent = SCR_CharacterAnimationComponent.Cast(owner.FindComponent(SCR_CharacterAnimationComponent));
		compartmentAccessComponent = CompartmentAccessComponent.Cast(owner.FindComponent(CompartmentAccessComponent));
		commandHandlerComponent = CharacterCommandHandlerComponent.Cast(owner.FindComponent(CharacterCommandHandlerComponent));
		
		if (!characterController || !animationComponent || !compartmentAccessComponent || !commandHandlerComponent)
		{
			Print("[RagdollTriggers] Failed to initialize dependent components!", LogLevel.WARNING);
			return;
		}
		
		ragdollImmunityUntil = timeAndWeatherManagerEntity.GetEngineTime();
		damageManager.GetOnDamage().Insert(OnAnyDamage);
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void OnDelete(IEntity owner)
	{
		if (damageManager)
		{
			damageManager.GetOnDamage().Remove(OnAnyDamage);
		}
	
		if (callQueue)
		{
			callQueue.Remove(ApplyRagdoll);
			callQueue.Remove(RecoverFromRagdoll);
		}
		
		super.OnDelete(owner);
	}

	protected RagdollTrigger GetRagdollTrigger(ECharacterHitZoneGroup hitZoneGroup)
	{
		foreach (RagdollTrigger trigger : RagdollTriggers)
		{
			if (trigger.HitZoneGroup == hitZoneGroup)
			{
				return trigger;
			}
		}
		
		return null;
	}

	protected ECharacterHitZoneGroup GetDamageContextHitZoneGroup(BaseDamageContext damageContext)
	{
		HitZone struckHitZone = damageContext.struckHitZone;
		if (!struckHitZone && damageContext.colliderID != 0)
		{
			HitZoneContainerComponent HitZoneContainer = HitZoneContainerComponent.Cast(damageManager);
			if (HitZoneContainer)
			{
				struckHitZone = HitZoneContainer.GetHitZoneByColliderID(damageContext.colliderID);
			}
		}
		
		if (!struckHitZone)
		{
			return 0;
		}
		
		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(struckHitZone);
		if (!characterHitZone)
		{
			return 0;
		}
		
		return characterHitZone.GetHitZoneGroup();
	}
	
	protected void OnAnyDamage(BaseDamageContext damageContext)
	{
		if (timeAndWeatherManagerEntity.GetEngineTime() < ragdollImmunityUntil)
		{
			return;
		}

		ECharacterHitZoneGroup hitZoneGroup = GetDamageContextHitZoneGroup(damageContext);
		if (hitZoneGroup == 0)
		{
			return;
		}
		
		RagdollTrigger ragdollTrigger = GetRagdollTrigger(hitZoneGroup);
		if (!ragdollTrigger ||
			(ragdollTrigger.MinimumRequiredDamage >= damageContext.damageValue) ||
			(ragdollTrigger.OddsToTrigger <= Math.RandomFloat(0.0, 1.0))
			)
		{
			return;
		}

		ragdollImmunityUntil = timeAndWeatherManagerEntity.GetEngineTime() + RagdollDelay + RagdollDurationSeconds + RagdollImmunitySeconds;

		callQueue.CallLater(ApplyRagdoll, (int)(RagdollDelay * 1000.0), false, damageContext, ragdollTrigger);
	}

	protected void ApplyRagdoll(BaseDamageContext damageContext, RagdollTrigger ragdollTrigger)
	{
		if (characterController.GetLifeState() != ECharacterLifeState.ALIVE)
		{
			return;
		}
		
		if (animationComponent.IsRagdollActive())
		{
			return;
		}
		
		if (compartmentAccessComponent && (compartmentAccessComponent.IsInCompartment() || compartmentAccessComponent.GetVehicleIn(myOwner)))
		{
			return;
		}
		
		vector LocalHitPosition = myOwner.CoordToLocal(damageContext.hitPosition);
		vector LocalHitDirection = myOwner.VectorToLocal(damageContext.hitDirection);

		if (LocalHitDirection.LengthSq() < 0.00001)
		{
			LocalHitDirection = "0 -1 0";
		}
		else
		{
			LocalHitDirection = LocalHitDirection.Normalized();
		}

		float ImpactForce = damageContext.damageValue * ragdollTrigger.ForceMultiplier;
		
		animationComponent.AddRagdollEffectorDamage(
			LocalHitPosition,
			LocalHitDirection,
			ImpactForce,
			RagdollRadius,
			RagdollDurationSeconds
		);
		
		characterController.Ragdoll();
		characterController.SetWeaponNoFireTime(RagdollDurationSeconds);

		callQueue.CallLater(RecoverFromRagdoll, (int)((RagdollDurationSeconds) * 1000.0), false);
	}

	protected void RecoverFromRagdoll()
	{
		characterController.RefreshRagdoll(0.0);

		commandHandlerComponent.AlignNewTurns();
		commandHandlerComponent.CancelItemUse();	
	}
}