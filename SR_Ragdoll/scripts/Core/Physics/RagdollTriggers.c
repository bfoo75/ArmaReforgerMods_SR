[ComponentEditorProps(description: "Triggers for ragdolling the player.")]
class RagdollTriggersComponentClass : ScriptComponentClass {}

class RagdollTrigger
{
	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, params: "0 50 0.1", desc: "Damage required before ragdolling may occur.")]
	float MinimumRequiredDamage;
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0.0 1.0 .05", desc: "Liklihood of ragdolling when the minimum damage is applied to the Hit Zone (0-1)")]
	float OddsToTrigger;
	[Attribute()]
	ECharacterHitZoneGroup HitZoneGroup;
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.EditBox, params: "0.001 2000 10", desc: "Ragdoll force multiplier")]
	float ForceMultiplier;
}

class RagdollTriggersComponent : ScriptComponent
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enable leg-shot ragdoll system")]
	protected bool m_bEnabled;
	[Attribute(defvalue: "0.85", uiwidget: UIWidgets.Slider, params: "0 500 0.05", desc: "How long before the player regains control from being ragdolled (seconds)")]
	protected float RagdollDurationSeconds;
	[Attribute(defvalue: "2.0", uiwidget: UIWidgets.Slider, params: "0 10 0.1", desc: "How long before the ragdoll can be triggered again after the player regains control (seconds)")]
	protected float RagdollImmunitySeconds;
	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, params: "0.1 20.0 0.1", desc: "Ragdoll effect radius")]
	protected float RagdollRadius;
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, params: "0.01 0.5 0.01", desc: "Delay before enabling ragdoll (seconds)")]
	protected float RagdollDelay;
	[Attribute()]
	array<RagdollTrigger> RagdollTriggers;

	protected float                                ragdollImmunityUntil;

	protected ChimeraCharacter 						character;
	protected SCR_CharacterDamageManagerComponent  	damageManager;
	protected SCR_CharacterControllerComponent		characterController;
	protected SCR_CharacterAnimationComponent		animationComponent;
	protected CompartmentAccessComponent 			compartmentAccessComponent;
	protected CharacterCommandHandlerComponent		commandHandlerComponent;	
	protected TimeAndWeatherManagerEntity			timeAndWeatherManagerEntity;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!owner)
		{
			return;
		}

		ragdollImmunityUntil = GetEngineTimeSafe();
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		character = ChimeraCharacter.Cast(GetOwner());
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
		
		World world = Owner.GetWorld();
		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(w);
		
		characterController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		animationController = SCR_CharacterAnimationComponent.Cast(owner.FindComponent(SCR_CharacterAnimationComponent));
		compartmentComponent = CompartmentAccessComponent.Cast(Owner.FindComponent(CompartmentAccessComponent));
		commandHandlerComponent = CharacterCommandHandlerComponent.Cast(Owner.FindComponent(CharacterCommandHandlerComponent));
		
		if (!characterController || !animationController || !compartmentComponent || !commandHandlerComponent)
		{
			Print("[RagdollTriggers] Failed initialize dependent components!", LogLevel.WARNING);
			return;
		}
		
		damageManager.GetOnDamage().Insert(OnAnyDamage);
	}

	override void OnDelete(IEntity owner)
	{
		if (DamageManager)
		{
			damageManager.GetOnDamage().Remove(OnAnyDamage);
		}
	
		GetGame().GetCallqueue().Remove(ApplyRagdoll);
		GetGame().GetCallqueue().Remove(RecoverFromRagdoll);
	
		super.OnDelete(owner);
	}

	protected RagdollTrigger GetRagdollTrigger(HitZone struckHitZone)
	{
		if (!struckHitZone)
		{
			return null;
		}
		
		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(struckHitZone);
		if (!characterHitZOne)
		{
			return null;
		}
		
		ECharacterHitZoneGroup group = characterHitZone.GetHitZoneGroup();
		foreach (RagdollTrigger trigger : RagdollTriggers)
		{
			if (trigger.HitZoneGroup == group)
			{
				return trigger;
			}
		}
		
		return null;
	}

	protected float GetEngineTimeSafe()
	{
		if (!m_pOwner)
			return 0.0;

		World w = m_pOwner.GetWorld();
		ChimeraWorld cw = ChimeraWorld.CastFrom(w);
		if (!cw)
			return 0.0;

		TimeAndWeatherManagerEntity mgr = cw.GetTimeAndWeatherManager();
		if (!mgr)
			return 0.0;

		return mgr.GetEngineTime();
	}

	protected void OnAnyDamage(BaseDamageContext damageContext)
	{
		if (!damageContext)
		{
			PrintFormat("[RagdollTriggers] Failed to get a DamageContext!  This should never happen!");
			return;
		}
			
		float engineTime = GetEngineTimeSafe();
		if (engineTime < ragdollImmunityUntil)
		{
			return;
		}

		HitZone struckHitZone = damageContext.struckHitZone;
		if (!struckHitZone && damageContext.colliderID != 0)
		{
			HitZoneContainerComponent HitZoneContainer = HitZoneContainerComponent.Cast(DamageManager);
			if (HitZoneContainer)
			{
				struckHitZone = HitZoneContainer.GetHitZoneByColliderID(damageContext.colliderID);
			}
		}

		RagdollTrigger Trigger = GetRagdollTrigger(struckHitZone);
		if (!Trigger ||
			(Trigger.MinimumRequiredDamage >= damageContext.damageValue) ||
			(Trigger.OddsToTrigger >= Math.RandomFloat(0.0, 1.0))
			)
		{
			return;
		}
		
		if (CharacterController.GetLifeState() != ECharacterLifeState.ALIVE)
		{
			return;
		}
		
		if (AnimationController.IsRagdollActive())
		{
			return;
		}
		
		if (compartmentAccessComponent && (compartmentAccessComponent.IsInCompartment() || compartmentAccessComponent.GetVehicleIn()))
		{
			return;
		}
		
		
		TriggerRagdoll(engineTime, struckHitZone, damageContext);
	}

	protected void TriggerRagdoll(float engineTime, HitZone struckHitZone, BaseDamageContext damageContext)
	{		
		m_fNextAllowedTime = engineTime + m_fCooldownSeconds;

		vector LocalHitPosition = m_pOwner.CoordToLocal(damageContext.hitPosition);
		vector LocalHitDirection = m_pOwner.VectorToLocal(damageContext.hitDirection);

		if (LocalHitDirection.LengthSq() < 0.00001)
		{
			LocalHitDirection = "0 -1 0";
		}
		else
		{
			LocalHitDirection = LocalHitDirection.Normalized();
		}

		float ImpactForce = Math.Max(damageContext.damageValue * m_fRagdollForce, 100.0);

		m_pAnimComponent.AddRagdollEffectorDamage(
			LocalHitPosition,
			LocalHitDirection,
			ImpactForce,
			m_fRagdollRadius,
			m_fRagdollSeconds
		);

		int delayMs = Math.Max(Math.Round(m_fRagdollDelay * 1000.0), 10);
		int recoverMs = Math.Clamp(Math.Round(m_fRagdollSeconds * 1000.0), 100, 6000);

		GetGame().GetCallqueue().CallLater(ApplyRagdoll, delayMs, false);
		GetGame().GetCallqueue().CallLater(RecoverFromRagdoll, recoverMs, false);
	}

	protected void ApplyRagdoll()
	{
		AnimationController.Ragdoll();
		AnimationController.SetWeaponNoFireTime(m_fRagdollSeconds * 0.75);
	}

	protected void RecoverFromRagdoll()
	{
		AnimationController.RefreshRagdoll(0.0);

		commandHandlerComponent.AlignNewTurns();
		commandHandlerComponent.CancelItemUse();	
	}

	// -------------------- Public API --------------------
	bool  GetEnabled()                  { return m_bEnabled; }
	void  SetEnabled(bool e)            { m_bEnabled = e; }

	float GetMinDamage()                { return m_fMinDamage; }
	void  SetMinDamage(float v)         { if (v < 0.0) v = 0.0; m_fMinDamage = v; }

	float GetRagdollSeconds()           { return m_fRagdollSeconds; }
	void  SetRagdollSeconds(float v)
	{
		if (v < 0.2) v = 0.2;
		if (v > 10.0) v = 10.0;
		m_fRagdollSeconds = v;
	}

	float GetCooldownSeconds()          { return m_fCooldownSeconds; }
	void  SetCooldownSeconds(float v)
	{
		if (v < 0.0) v = 0.0;
		if (v > 10.0) v = 10.0;
		m_fCooldownSeconds = v;
	}

	float GetRagdollForce()             { return m_fRagdollForce; }
	void  SetRagdollForce(float v)
	{
		if (v < 0.1) v = 0.1;
		m_fRagdollForce = v;
	}

	float GetRagdollRadius()            { return m_fRagdollRadius; }
	void  SetRagdollRadius(float v)
	{
		if (v < 0.1) v = 0.1;
		m_fRagdollRadius = v;
	}

	float GetRagdollDelay()             { return m_fRagdollDelay; }
	void  SetRagdollDelay(float v)
	{
		if (v < 0.01) v = 0.01;
		if (v > 0.5)  v = 0.5;
		m_fRagdollDelay = v;
	}

	float GetTriggerPercentage()        { return m_fTriggerPercentage; }
	void  SetTriggerPercentage(float v)
	{
		if (v < 0.0)   v = 0.0;
		if (v > 100.0) v = 100.0;
		m_fTriggerPercentage = v;
	}

	bool  GetDebug()                    { return m_bDebug; }
	void  SetDebug(bool d)              { m_bDebug = d; }
}