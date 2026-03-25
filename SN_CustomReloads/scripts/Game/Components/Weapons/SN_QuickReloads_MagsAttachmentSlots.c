//───────────────────────────────────────────────────────────────
// ATTACHMENT SLOT : CURRENT MAG
//───────────────────────────────────────────────────────────────

class SN_QuickReloads_AttachmentCurrentMagClass : AttachmentSlotComponentClass {}

class SN_QuickReloads_AttachmentCurrentMag : AttachmentSlotComponent
{
	override bool ShouldSetAttachment(IEntity attachmentEntity)
	{
		return true;
	}
}

//───────────────────────────────────────────────────────────────
// ATTACHMENT SLOT : SECOND MAG
//───────────────────────────────────────────────────────────────

class SN_QuickReloads_AttachmentSecondMagClass : AttachmentSlotComponentClass {}

class SN_QuickReloads_AttachmentSecondMag : AttachmentSlotComponent
{
	override bool ShouldSetAttachment(IEntity attachmentEntity)
	{
		return true;
	}
}