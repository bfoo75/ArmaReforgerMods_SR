modded class SCR_ChatPanel
{
	override protected void SendMessage()
	{
		SCR_ChatComponent chatComponent = GetChatComponent();
		if (!chatComponent || !m_ActiveChannel)
		{
			return;
		}
		
		bool isAdmin = false;
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController != null)	
		{
			int playerId = playerController.GetPlayerId();
			isAdmin = SCR_CharacterHelper.GetPlayerControlType(playerId) == SCR_ECharacterControlType.UNLIMITED_EDITOR;
		}
		
		bool IsGlobalChannel = m_ActiveChannel.Type() == BaseChatChannel && m_ActiveChannel.GetName() == "Global";
		
		if (!m_ActiveChannel.IsAvailable(chatComponent) || (IsGlobalChannel && !isAdmin))
		{
			SCR_ChatMessageStyle style = this.GetChannelStyle(m_ActiveChannel);
			SCR_ChatPanelManager.GetInstance().ShowHelpMessage(STR_CHANNEL_DISABLED);
		}
		else
		{
			super.SendMessage();
		}
	}
}
