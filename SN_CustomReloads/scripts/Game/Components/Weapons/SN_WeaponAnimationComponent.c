[EntityEditorProps(category: "GameScripted/Weapons")]
class SN_WeaponAnimationComponentClass : WeaponAnimationComponentClass {}

class SN_WeaponAnimationComponent : WeaponAnimationComponent
{
	protected AnimationEventID m_SpawnMainMagCopy;
	protected AnimationEventID m_DestroyMainMagCopy;
	protected AnimationTagID m_TagWeaponReload;
	
	protected BaseWeaponComponent m_WeaponComp;
	protected IEntity m_VisualMag;
	protected AttachmentMainMagCopy m_VisualSlot;
	protected bool m_ReloadActive;
	
	//───────────────────────────────────────────────
	// Constructor
	//───────────────────────────────────────────────
	private void SN_WeaponAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_WeaponComp = BaseWeaponComponent.Cast(ent.FindComponent(BaseWeaponComponent));

		m_SpawnMainMagCopy   = GameAnimationUtils.RegisterAnimationEvent("EV_SpawnMainMagCopy");
		m_DestroyMainMagCopy = GameAnimationUtils.RegisterAnimationEvent("EV_DestroyMainMagCopy");
		//Vanilla reload animation tag
		m_TagWeaponReload = GameAnimationUtils.RegisterAnimationTag("TagWeaponReload");

		SN_WeaponComponent weaponStateComp = SN_WeaponComponent.Cast(ent.FindComponent(SN_WeaponComponent));
		if (weaponStateComp)
			weaponStateComp.GetOnWeaponStateChanged().Insert(OnWeaponStateChanged);
	}

	//───────────────────────────────────────────────
	// Weapon became inactive → destroy
	//───────────────────────────────────────────────
	protected void OnWeaponStateChanged(WeaponComponent weapon, bool active)
	{
		if (!active && m_ReloadActive)
			DestroyMainMagCopy();
	}

	//───────────────────────────────────────────────
	// Animation events
	//───────────────────────────────────────────────
	override event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		if (animEventType == m_SpawnMainMagCopy)
			SpawnMainMagCopy();
		
		if (animEventType == m_DestroyMainMagCopy)
			DestroyMainMagCopy();
	}
	
	override protected bool OnProcessAnimOutput(IEntity owner, float ts)
	{
		super.OnProcessAnimOutput(owner, ts);
	
		// watchdog animation graph : reload canceled before destroy anim event
		if (m_ReloadActive && !IsAnimationTag(m_TagWeaponReload))
		{
			DestroyMainMagCopy();
		}
	
		return false;
	}

	//───────────────────────────────────────────────
	// Spawn visual magazine
	//───────────────────────────────────────────────
	void SpawnMainMagCopy()
	{
		if (!Replication.IsServer())
			return;
		
		DestroyMainMagCopy();
		m_ReloadActive = true;

		BaseWeaponComponent weaponComp = m_WeaponComp;
		if (!weaponComp) return;

		BaseMagazineComponent currentMag = weaponComp.GetCurrentMagazine();
		if (!currentMag) return;

		IEntity magEntity = currentMag.GetOwner();
		if (!magEntity) return;

		// Find replicated slot 
		array<AttachmentSlotComponent> slots = {};
		weaponComp.GetAttachments(slots);
		
		AttachmentMainMagCopy visualSlot = null;
		foreach (AttachmentSlotComponent slot : slots)
		{
			visualSlot = AttachmentMainMagCopy.Cast(slot);
			if (visualSlot)
				break;
		}
		
		if (!visualSlot)
		{
			m_ReloadActive = false;
			return;
		}

		// Spawn prefab copy
		EntityPrefabData data = magEntity.GetPrefabData();
		if (!data) return;

		ResourceName prefab = data.GetPrefabName();
		if (prefab.IsEmpty()) return;

		Resource res = Resource.Load(prefab);
		if (!res)
		{
			m_ReloadActive = false;
			return;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		m_VisualMag = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (!m_VisualMag)
		{
			m_ReloadActive = false;
			return;
		}

		m_VisualSlot = visualSlot;
		m_VisualSlot.SetAttachment(m_VisualMag);
	}
	
	//───────────────────────────────────────────────
	// Destroy visual magazine
	//───────────────────────────────────────────────
	void DestroyMainMagCopy()
	{
		if (!Replication.IsServer())
			return;
	
		m_ReloadActive = false;
		
		if (!m_VisualMag)
			return;
	
		if (m_VisualSlot)
			m_VisualSlot.SetAttachment(null);
	
		SCR_EntityHelper.DeleteEntityAndChildren(m_VisualMag);
		m_VisualMag = null;
		m_VisualSlot = null;
	}
}
