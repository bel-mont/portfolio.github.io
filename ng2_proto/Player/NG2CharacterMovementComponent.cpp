#include "NG2CharacterMovementComponent.h"

UNG2CharacterMovementComponent::UNG2CharacterMovementComponent()
{
	// Higher jump apex
	JumpZVelocity = 1300.0f;
	MaxWalkSpeed = 600.f;
	BrakingDecelerationWalking = 1200.0f;
}

float UNG2CharacterMovementComponent::GetGravityZ() const
{
	const float BaseGravity = Super::GetGravityZ();

	if (!IsFalling()) return BaseGravity;
	
	const float VelZ = Velocity.Z;
	
	// Descending - fast fall
	if (VelZ < -FallGravityApexThreshold)
	{
		return BaseGravity * FallGravityMultiplier; 
	}
	
	// Apex window - reduced gravity for hang
	if (FMath::Abs(VelZ) <= FallGravityApexThreshold)
	{
		return BaseGravity * ApexGravityMultiplier;
	}
	
	// Ascending (VelZ > FallGravityApexThreshold)
	return BaseGravity * AscentGravityMultiplier;
}
