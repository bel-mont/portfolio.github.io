#include "NG2AerialAttack.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/Player/NG2RyuuCharacter.h"

void UNG2AerialAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!Character) return;

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	if (!ensureMsgf(CMC, TEXT("NG2AerialAttack::NotifyBegin: No CharacterMovementComponent"))) return;
	
	// ASC = Cast<UNG2AbilitySystemComponent>(Character->GetAbilitySystemComponent());
	if (!ASC) return;
	ASC->AddLooseGameplayTag(NG2GameplayTags::State_AirAttacking); // Is removed later by an Ability
	CMC->SetMovementMode(MOVE_Flying);
}

void UNG2AerialAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!Character) return;

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	if (!ensureMsgf(CMC, TEXT("NG2AerialAttack::NotifyEnd: No CharacterMovementComponent"))) return;
	CMC->Velocity.Z = OnEndVerticalVelocity;
	CMC->SetMovementMode(MOVE_Falling);

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}