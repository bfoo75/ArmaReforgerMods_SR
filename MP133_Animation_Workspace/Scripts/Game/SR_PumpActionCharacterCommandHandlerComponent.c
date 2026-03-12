modded class SCR_CharacterCommandHandlerComponent
{
	override event bool HandleWeaponReloading(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if (!m_WeaponManager)
			return false;

		BaseWeaponComponent weapon = m_WeaponManager.GetCurrentWeapon();
		if (!weapon)
			return false;

		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		BaseMagazineComponent mag = weapon.GetCurrentMagazine();
		if (!mag)
			return false;

		int currentAmmo = mag.GetAmmoCount();
		int maxAmmo = mag.GetMaxAmmoCount();

		int shellsNeeded = maxAmmo - currentAmmo;

		bool chamberEmpty = !muzzle.IsCurrentBarrelChambered();

		int reloadCode;

		if (chamberEmpty)
		{
			// 40 = charger la chambre
			reloadCode = 40;
		}
		else
		{
			if (shellsNeeded <= 0)
				return false;

			// 41+ = reload tube
			reloadCode = 40 + shellsNeeded;
		}

		pInputCtx.SetReloadWeapon(reloadCode);

		return HandleWeaponReloadingDefault(pInputCtx, pDt, pCurrentCommandID);
	}
}