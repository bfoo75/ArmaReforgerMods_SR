class SN_WeaponComponentClass : WeaponComponentClass {}

void SN_WeaponStateChangedMethod(WeaponComponent weapon, bool active);
typedef func SN_WeaponStateChangedMethod;
typedef ScriptInvokerBase<SN_WeaponStateChangedMethod> ScriptInvokerSN_WeaponState;

class SN_WeaponComponent : WeaponComponent
{
	protected ref ScriptInvokerSN_WeaponState m_OnWeaponStateChanged;

	ScriptInvokerSN_WeaponState GetOnWeaponStateChanged()
	{
		if (!m_OnWeaponStateChanged)
			m_OnWeaponStateChanged = new ScriptInvokerSN_WeaponState();

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
