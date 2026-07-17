#include "NG2JumpLaunch.h"

#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/Player/NG2CharacterMovementComponent.h"
#include "NG2/Player/NG2RyuuCharacter.h"

void UNG2JumpLaunch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!Character || !ASC) return;

	Character->Jump();
	// Check for jump type.
	UNG2CharacterMovementComponent* CMC = Cast<UNG2CharacterMovementComponent>(Character->GetCharacterMovement());
	// When doing a forward jump, adjust velocity
	if (const float Speed = Character->GetVelocity().SizeSquared2D(); Speed > 0.f)
	{
		const FVector LaunchDir = Character->GetActorForwardVector();
		const float ForwardJumpSpeed = CMC->ForwardJumpSpeed;
		FVector NewVelocity = CMC->Velocity;
		NewVelocity.X = LaunchDir.X * ForwardJumpSpeed;
		NewVelocity.Y = LaunchDir.Y * ForwardJumpSpeed;
		CMC->Velocity = NewVelocity;
	}
	
	// Removed on a case by case basis by animations and other systems
	ASC->AddLooseGameplayTag(NG2GameplayTags::State_MovementBlocked);
}
