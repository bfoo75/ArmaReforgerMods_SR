
#define SR_DEBUG_RANKCONSTRAINTS

modded class SCR_Faction
{	
	override void Init(IEntity owner)
	{
		SetRankConstraints();
		
		super.Init(owner);
	}
	
	void SetRankConstraints()
	{
		if (!m_aEntityCatalogs)
		{
			Print("Failed to update Rank Unlocks!", LogLevel.ERROR);
			return;
		}
		
		array<SCR_EntityCatalogEntry> allCatalogEntries = {};
		foreach (ref SCR_EntityCatalog EntityCatalog : m_aEntityCatalogs)
		{
			#ifdef SR_DEBUG_RANKCONSTRAINTS
			PrintFormat("Appending EntityCatalog - %1", EntityCatalog);
			#endif
			
			EntityCatalog.AppendRecursiveEntityList(allCatalogEntries);
		}

		#ifdef SR_DEBUG_RANKCONSTRAINTS
		Print("Updating Rank Unlocks...");
		#endif
		
		foreach (ref SCR_CharacterRank rank : m_aRanks)
		{
			#ifdef SR_DEBUG_RANKCONSTRAINTS
			PrintFormat("Rank - %1", rank);
			#endif
			
			array<ResourceName> UnlockedItems = {};
			rank.GetUnlockedItems(UnlockedItems);
			
			foreach (ResourceName UnlockedItemName : UnlockedItems)
			{
				#ifdef SR_DEBUG_RANKCONSTRAINTS
				PrintFormat("Item - %1", UnlockedItemName);
				#endif
				
				SCR_EntityCatalogEntry foundEntry = null;
				foreach (ref SCR_EntityCatalogEntry catalogEntry : allCatalogEntries)
				{
					ResourceName EntryName = catalogEntry.GetPrefab();
					if (EntryName == UnlockedItemName)
					{
						foundEntry = catalogEntry;
						break;
					}
				}
				
				if (!foundEntry)
				{
					PrintFormat("Failed to find CatalogEntry for: %1!", UnlockedItemName, LogLevel.ERROR);
					continue;
				}
				
				SCR_ArsenalItem ArsenalItem = SCR_ArsenalItem.Cast(foundEntry.GetEntityDataOfType(SCR_ArsenalItem));
				SCR_ECharacterRank unlockRankID = rank.GetRankID();
				
				#ifdef SR_DEBUG_RANKCONSTRAINTS
				PrintFormat("Unlock Set - %1 - %2", ArsenalItem, unlockRankID);
				#endif
				
				ArsenalItem.SetRequiredRank(unlockRankID);
			}
		}
	}
}