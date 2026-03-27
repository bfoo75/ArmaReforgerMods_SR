[EntityEditorProps(category: "GameScripted/Custom", description: "Base component for handling animation.")]
class SR_UAR15WeaponAnimationComponentClass : WeaponAnimationComponentClass {}

class SR_UAR15WeaponAnimationComponent : WeaponAnimationComponent
{
    protected AnimationEventID m_iDetachMag;
	protected AnimationEventID m_iEjectMag;
	protected IEntity m_WeaponEntity;
	protected IEntity m_DetachedMagEntity;

    private void SR_UAR15WeaponAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
    {
		m_WeaponEntity = ent;
		
        // Enregistre l'événement d'animation
        m_iDetachMag = GameAnimationUtils.RegisterAnimationEvent("Event_DetachMag");
		m_iEjectMag = GameAnimationUtils.RegisterAnimationEvent("Event_EjectMag");
    }

    override event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
    {
        super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

        if (animEventType == m_iDetachMag)
        {
            DetachCurrentMagazine();
        }
		
		if (animEventType == m_iEjectMag)
        {
            EjectCurrentMagazine();
        }
    }

	void DetachCurrentMagazine()
	{
	    const WeaponComponent weaponComponent = WeaponComponent.Cast(m_WeaponEntity.FindComponent(WeaponComponent));
	    if (!weaponComponent) return;
	
	    const BaseMagazineComponent magComponent = weaponComponent.GetCurrentMagazine();
	    if (!magComponent) return;
	
	    const IEntity magEntity = magComponent.GetOwner();
	    if (!magEntity) return;
		
		ChimeraCharacter character = GetOwningCharacter();
		if (!character) return;
		
		Animation animSys = character.GetAnimation();
		if (!animSys) return;

		TNodeId boneID = animSys.GetBoneIndex("LeftHandProp");
		if (boneID == -1) return;
		
		vector currentLocalTransform[4];
		
		magEntity.GetLocalTransform(currentLocalTransform);
		m_WeaponEntity.RemoveChild(magEntity, true);
		character.AddChild(magEntity, boneID, EAddChildFlags.AUTO_TRANSFORM);

		magEntity.SetLocalTransform(currentLocalTransform);

		m_DetachedMagEntity = magEntity;
	}
	
	void EjectCurrentMagazine()
	{
		if (m_DetachedMagEntity)
		{
			ChimeraCharacter character = GetOwningCharacter();
			if (!character) return;
			
		    Physics magPhysics = m_DetachedMagEntity.GetPhysics();
		    if (!magPhysics) return;
		
		    // Détache du parent
		    character.RemoveChild(m_DetachedMagEntity, true);
		
		    // Setup physique
		    magPhysics.ChangeSimulationState(SimulationState.SIMULATION);
		    magPhysics.SetInteractionLayer(EPhysicsLayerPresets.Debris);
		    magPhysics.EnableGravity(true);
		
			vector right = m_WeaponEntity.GetTransformAxis(0);
			vector up    = m_WeaponEntity.GetTransformAxis(1);
			vector fwd   = m_WeaponEntity.GetTransformAxis(2);
			
			float wUp    = 0.05;  // petit kick sortie
			float wDown  = 0.003;  // chute rapide
			float wFwd   = 0.001;  // TOUJOURS vers l’avant
			
			vector desired = (-up * wDown) + (fwd * wFwd);
			vector impulseDir = desired.Normalized();
		
			float forceMultiplier = 1.5;
			magPhysics.ApplyImpulseAt(magPhysics.GetCenterOfMass(), impulseDir * magPhysics.GetMass() * forceMultiplier);
			
		    if (character)
		    {
		        Physics ownerPhysics = character.GetPhysics();
		        if (ownerPhysics)
		        {
		            vector ownerVel = ownerPhysics.GetVelocity();
		            magPhysics.SetVelocity(magPhysics.GetVelocity() + ownerVel);
		        }
		    }
		
		    vector angVel = -right * 5.0; // vitesse à ajuster
			magPhysics.SetAngularVelocity(angVel);
		}
	}
	
	protected ChimeraCharacter GetOwningCharacter()
	{
		if (!m_WeaponEntity) return null;
		return ChimeraCharacter.Cast(m_WeaponEntity.GetParent());
	}
}