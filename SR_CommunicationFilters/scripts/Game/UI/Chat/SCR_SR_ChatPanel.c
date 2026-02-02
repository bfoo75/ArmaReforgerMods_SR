modded class SCR_ChatPanel
{
	override protected void SendMessage()
	{
		SCR_ChatComponent chatComponent = GetChatComponent();
		if (!chatComponent || !m_ActiveChannel)
			return;
		
		bool IsGlobalChannel = m_ActiveChannel.Type() == BaseChatChannel && m_ActiveChannel.GetName() == "Global";
		
		if (!m_ActiveChannel.IsAvailable(chatComponent) || IsGlobalChannel)
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
