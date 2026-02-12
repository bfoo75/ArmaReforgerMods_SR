[BaseContainerProps()]
modded class SCR_CharacterRank
{
	//[Attribute(desc: "Prefab of entity", UIWidgets.ResourcePickerThumbnail, params: "et")]
	//protected ResourceName m_sEntityPrefab;
	
	[Attribute(desc: "Prefab of items unlocked by this rank.")]
	protected ref array<ResourceName> m_aUnlockedItems = {};
	
	void GetUnlockedItems(out array<ResourceName> RankUnlockedItems)
	{
		if (!m_aUnlockedItems)
		{
			return;
		}
		
		RankUnlockedItems.Clear();
		RankUnlockedItems.InsertAll(m_aUnlockedItems);
	}
}