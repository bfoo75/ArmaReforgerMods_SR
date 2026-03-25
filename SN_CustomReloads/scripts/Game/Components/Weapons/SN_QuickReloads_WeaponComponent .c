class SN_QuickReloads_WeaponComponentClass : WeaponComponentClass {}

void SN_QuickReloads_WeaponStateChangedMethod(WeaponComponent weapon, bool active);
typedef func SN_QuickReloads_WeaponStateChangedMethod;
typedef ScriptInvokerBase<SN_QuickReloads_WeaponStateChangedMethod> ScriptInvokerSN_QuickReloads_WeaponState;

class SN_QuickReloads_WeaponComponent : WeaponComponent
{
	protected ref ScriptInvokerSN_QuickReloads_WeaponState m_OnWeaponStateChanged;

	ScriptInvokerSN_QuickReloads_WeaponState GetOnWeaponStateChanged()
	{
		if (!m_OnWeaponStateChanged)
			m_OnWeaponStateChanged = new ScriptInvokerSN_QuickReloads_WeaponState();

		return m_OnWeaponStateChanged;
	}

	override event void OnWeaponActive()
	{
		super.OnWeaponActive();
		if (m_OnWeaponStateChanged)
			m_OnWeaponStateChanged.Invoke(this, true);
	}

	override event void OnWeaponInactive()
	{
		super.OnWeaponInactive();
		if (m_OnWeaponStateChanged)
			m_OnWeaponStateChanged.Invoke(this, false);
	}
}