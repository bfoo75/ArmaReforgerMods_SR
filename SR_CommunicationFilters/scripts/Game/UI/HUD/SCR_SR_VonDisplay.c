modded class SCR_VonDisplay
{
	//------------------------------------------------------------------------------------------------
	//! Update transmission data
	//! \param TransmissionData is the subject
	//! \param radioTransceiver is the used transceiver for the transmission
	//! \param IsReceiving is true when receiving transmission, false when transmitting
	//! \param isAdditionalSpeaker is true when all incomming transmission widgets are full
	//! \return false if the transmission is filtered out to not be visible
	override bool UpdateTransmission(TransmissionData data, BaseTransceiver radioTransceiver, int frequency, bool IsReceiving)
	{
		bool isAdmin = false;
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController != null)	
		{
			int playerId = playerController.GetPlayerId();
			isAdmin = SCR_CharacterHelper.GetPlayerControlType(playerId) == SCR_ECharacterControlType.UNLIMITED_EDITOR;
		}
		
		if (!isAdmin && IsReceiving && !radioTransceiver)
		{
			return false;
		}

		return super.UpdateTransmission(data, radioTransceiver, frequency, IsReceiving);
	}
}
