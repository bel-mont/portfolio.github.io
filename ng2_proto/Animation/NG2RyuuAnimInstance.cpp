#include "NG2RyuuAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/Player/NG2RyuuCharacter.h"

void UNG2RyuuAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
    
	CachedOwner = Cast<ANG2RyuuCharacter>(TryGetPawnOwner());
	if (!CachedOwner.IsValid()) return;
    
	CachedASC = Cast<UNG2AbilitySystemComponent>(CachedOwner->GetAbilitySystemComponent());
}

void UNG2RyuuAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!CachedOwner.IsValid() || !CachedASC.IsValid()) return;

	UCharacterMovementComponent* CMC = CachedOwner->GetCharacterMovement();

	Speed = CachedOwner->GetVelocity().SizeSquared();

	const bool bHasXMovement = !FMath::IsNearlyZero(CachedOwner->GetVelocity().X);
	const bool bHasYMovement = !FMath::IsNearlyZero(CachedOwner->GetVelocity().Y);
	bIsMovingHorizontally = bHasXMovement || bHasYMovement;
	bIsInAir = CMC->IsFalling() || CMC->MovementMode == MOVE_Flying || CachedASC->HasMatchingGameplayTag(NG2GameplayTags::State_Jumping);
	bIsAttackAerial = bIsInAir && CachedASC->HasMatchingGameplayTag(NG2GameplayTags::State_AirAttacking);
	bIsAttacking = CachedASC->HasMatchingGameplayTag(NG2GameplayTags::State_Attacking);
	// Allows landing animation cancelling by inputting another jump.
	bIsJumping = CachedASC->HasMatchingGameplayTag(NG2GameplayTags::State_Jumping);
}

bool UNG2RyuuAnimInstance::ApplyGameplayEffectToOwner(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!EffectClass || !CachedOwner.IsValid()) return false;
	
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		CachedOwner.Get());
	if (!AbilityComponent) return false;

	AbilityComponent->ApplyGameplayEffectToSelf(
		EffectClass.GetDefaultObject(),
		1.0f,
		AbilityComponent->MakeEffectContext());
	return true;
}
