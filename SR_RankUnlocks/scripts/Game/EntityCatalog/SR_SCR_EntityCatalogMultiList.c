[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomEntityCatalogCatalog(EEntityCatalogType, "m_eEntityCatalogType", "m_aEntityEntryList", "m_aMultiLists")]
modded class SCR_EntityCatalogMultiList : SCR_EntityCatalog
{
	override int AppendRecursiveEntityList(notnull out array<SCR_EntityCatalogEntry> entityList)
	{
		int ListCount = 0;
		foreach (SCR_EntityCatalogMultiListEntry multiList : m_aMultiLists)
		{
			ListCount += multiList.m_aEntities.Count();
			foreach (SCR_EntityCatalogEntry entry : multiList.m_aEntities)
			{
				if (!entry.IsEnabled())
					continue;
				
				entityList.Insert(entry);
			}
		}
		
		return ListCount;
	}
}