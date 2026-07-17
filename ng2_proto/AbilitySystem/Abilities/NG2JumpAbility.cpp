#include "NG2JumpAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "NG2/NG2GameplayTags.h"

void UNG2JumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!TryCommitOrEnd(Handle, ActorInfo, ActivationInfo)) return;
	
	ExecuteAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ASC->AddLooseGameplayTag(NG2GameplayTags::State_Jumping); // Removed on landing
	// This ability only triggers the action of jumping, but the animation and locomotion itself is done separately.
	// Check the JumpLaunch notify for the next parts.
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UNG2JumpAbility::ExecuteAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->OwnerActor);
	check(Character);
	// The actual character jump is delegated to a notify
	// The reasoning is that the moment of the physical jump
	// is different from the moment of the ability activation.
	// The animation can start playing before the jump is executed.
	// On ability trigger, we only start the animation, which can in turn
	// tell the character to WHEN to jump.
	// Character->Jump();
}
