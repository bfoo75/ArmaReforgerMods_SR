//───────────────────────────────────────────────────────────────
// ATTACHMENT SLOT : MAIN MAG COPY (REPLICATED)
//───────────────────────────────────────────────────────────────

class AttachmentMainMagCopyClass : AttachmentSlotComponentClass {}

class AttachmentMainMagCopy : AttachmentSlotComponent
{
	override bool ShouldSetAttachment(IEntity attachmentEntity)
	{
		return true;
	}
}
