[EntityEditorProps(category: "GameScripted/Weapons")]
class SN_QuickReloads_WeaponAnimationComponentClass : WeaponAnimationComponentClass {}

class SN_QuickReloads_WeaponAnimationComponent : WeaponAnimationComponent
{
	// EVENTS
	protected AnimationEventID m_SNQR_SpawnCurrentMag;
	protected AnimationEventID m_SNQR_SpawnSecondMag;
	protected AnimationEventID m_SNQR_DestroyMags;

	protected AnimationTagID m_TagWeaponReload;

	// DATA
	protected BaseWeaponComponent m_WeaponComp;

	protected IEntity m_CurrentVisualMag;
	protected IEntity m_SecondVisualMag;

	protected SN_QuickReloads_AttachmentCurrentMag m_CurrentSlot;
	protected SN_QuickReloads_AttachmentSecondMag m_SecondSlot;

	protected IEntity m_HiddenMag;
	protected bool m_ReloadActive;

	//───────────────────────────────────────────────
	// CONSTRUCTOR
	//───────────────────────────────────────────────
	private void SN_QuickReloads_WeaponAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_WeaponComp = BaseWeaponComponent.Cast(ent.FindComponent(BaseWeaponComponent));

		m_SNQR_SpawnCurrentMag = GameAnimationUtils.RegisterAnimationEvent("SNQR_SpawnCurrentMag");
		m_SNQR_SpawnSecondMag  = GameAnimationUtils.RegisterAnimationEvent("SNQR_SpawnSecondMag");
		m_SNQR_DestroyMags     = GameAnimationUtils.RegisterAnimationEvent("SNQR_DestroyMags");

		m_TagWeaponReload = GameAnimationUtils.RegisterAnimationTag("TagWeaponReload");

		SN_QuickReloads_WeaponComponent weaponStateComp = SN_QuickReloads_WeaponComponent.Cast(ent.FindComponent(SN_QuickReloads_WeaponComponent));
		if (weaponStateComp)
			weaponStateComp.GetOnWeaponStateChanged().Insert(OnWeaponStateChanged);
	}

	//───────────────────────────────────────────────
	// WEAPON STATE
	//───────────────────────────────────────────────
	protected void OnWeaponStateChanged(WeaponComponent weapon, bool active)
	{
		if (!active && m_ReloadActive)
			DestroyAllMags();
	}

	//───────────────────────────────────────────────
	// ANIMATION EVENTS
	//───────────────────────────────────────────────
	override event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		if (animEventType == m_SNQR_SpawnCurrentMag)
			SpawnCurrentMag();

		if (animEventType == m_SNQR_SpawnSecondMag)
			SpawnSecondMag();

		if (animEventType == m_SNQR_DestroyMags)
			DestroyAllMags();
	}

	override protected bool OnProcessAnimOutput(IEntity owner, float ts)
	{
		super.OnProcessAnimOutput(owner, ts);

		if (m_ReloadActive && !IsAnimationTag(m_TagWeaponReload))
			DestroyAllMags();

		return false;
	}

	//───────────────────────────────────────────────
	// SPAWN CURRENT MAG
	//───────────────────────────────────────────────
	void SpawnCurrentMag()
	{
		DestroyCurrentMag();
		m_ReloadActive = true;

		BaseMagazineComponent currentMag = m_WeaponComp.GetCurrentMagazine();
		if (!currentMag) return;

		IEntity magEntity = currentMag.GetOwner();
		if (!magEntity) return;

		m_HiddenMag = magEntity;
		m_HiddenMag.ClearFlags(EntityFlags.VISIBLE, true);

		m_CurrentVisualMag = SpawnVisualFromEntity(magEntity);
		m_CurrentSlot = GetCurrentSlot();

		if (m_CurrentSlot)
			m_CurrentSlot.SetAttachment(m_CurrentVisualMag);
	}

	//───────────────────────────────────────────────
	// SPAWN SECOND MAG
	//───────────────────────────────────────────────
	void SpawnSecondMag()
	{
		DestroySecondMag();

		BaseMagazineComponent currentMag = m_WeaponComp.GetCurrentMagazine();
		if (!currentMag) return;

		IEntity magEntity = currentMag.GetOwner();
		if (!magEntity) return;

		m_HiddenMag = magEntity;
		m_HiddenMag.ClearFlags(EntityFlags.VISIBLE, true);
		
		m_SecondVisualMag = SpawnVisualFromEntity(magEntity);
		m_SecondSlot = GetSecondSlot();

		if (m_SecondSlot)
			m_SecondSlot.SetAttachment(m_SecondVisualMag);
	}

	//───────────────────────────────────────────────
	// DESTROY
	//───────────────────────────────────────────────
	void DestroyAllMags()
	{
		m_ReloadActive = false;

		DestroyCurrentMag();
		DestroySecondMag();
		RestoreHiddenMag();
	}

	void DestroyCurrentMag()
	{
		if (m_CurrentSlot)
			m_CurrentSlot.SetAttachment(null);

		if (m_CurrentVisualMag)
			SCR_EntityHelper.DeleteEntityAndChildren(m_CurrentVisualMag);

		m_CurrentVisualMag = null;
		m_CurrentSlot = null;
	}

	void DestroySecondMag()
	{
		if (m_SecondSlot)
			m_SecondSlot.SetAttachment(null);

		if (m_SecondVisualMag)
			SCR_EntityHelper.DeleteEntityAndChildren(m_SecondVisualMag);

		m_SecondVisualMag = null;
		m_SecondSlot = null;
	}

	//───────────────────────────────────────────────
	// UTILS
	//───────────────────────────────────────────────
	IEntity SpawnVisualFromEntity(IEntity source)
	{
		EntityPrefabData data = source.GetPrefabData();
		if (!data) return null;

		ResourceName prefab = data.GetPrefabName();
		if (prefab.IsEmpty()) return null;

		Resource res = Resource.Load(prefab);
		if (!res) return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}

	SN_QuickReloads_AttachmentCurrentMag GetCurrentSlot()
	{
		array<AttachmentSlotComponent> slots = {};
		m_WeaponComp.GetAttachments(slots);

		foreach (AttachmentSlotComponent slot : slots)
		{
			SN_QuickReloads_AttachmentCurrentMag s = SN_QuickReloads_AttachmentCurrentMag.Cast(slot);
			if (s) return s;
		}
		return null;
	}

	SN_QuickReloads_AttachmentSecondMag GetSecondSlot()
	{
		array<AttachmentSlotComponent> slots = {};
		m_WeaponComp.GetAttachments(slots);

		foreach (AttachmentSlotComponent slot : slots)
		{
			SN_QuickReloads_AttachmentSecondMag s = SN_QuickReloads_AttachmentSecondMag.Cast(slot);
			if (s) return s;
		}
		return null;
	}

	void RestoreHiddenMag()
	{
		if (m_HiddenMag && !m_HiddenMag.IsDeleted())
		{
			m_HiddenMag.SetFlags(EntityFlags.VISIBLE, true);
			m_HiddenMag = null;
		}
	}
}