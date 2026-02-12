
modded class SCR_Faction
{
	override void InitializeFaction()
	{
		super.InitializeFaction();
		foreach (ref SCR_CharacterRank rank : m_aRanks)
		{
			array<ResourceName> UnlockedItems = {};
			rank.GetUnlockedItems(UnlockedItems);
			foreach (ResourceName UnlockedItemName : UnlockedItems)
			{
				foreach (ref SCR_EntityCatalog EntityCatalog : m_aEntityCatalogs)
				{
					SCR_EntityCatalogEntry Entry = EntityCatalog.GetEntryWithPrefab(UnlockedItemName);
					if (Entry == null)
					{
						continue;
					}
					
					if (!Entry.HasEntityDataOfType(SCR_ArsenalItem))
					{
						continue;
					}
					
					SCR_ArsenalItem ArsenalItem = SCR_ArsenalItem.Cast(Entry.GetEntityDataOfType(SCR_ArsenalItem));
					ArsenalItem.SetRequiredRank(rank.GetRankID());
				}
			}
		}
	}	
}
