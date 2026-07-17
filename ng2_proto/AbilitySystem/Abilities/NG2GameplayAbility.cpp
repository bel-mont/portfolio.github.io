#include "NG2GameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NG2/Player/NG2RyuuCharacter.h"

bool UNG2GameplayAbility::TryCommitOrEnd(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo& ActivationInfo)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return false;
	}
	return true;
}

UNG2ComboComponent* UNG2GameplayAbility::GetComboComponent() const
{
	const ANG2RyuuCharacter* Character = Cast<ANG2RyuuCharacter>(GetActorInfo().OwnerActor);
	if (!ensure(Character)) return nullptr;
	return Character->GetComboComponent();
}

void UNG2GameplayAbility::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> EffectClass)
{
	UAbilitySystemComponent* TargetComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetComp)
	{
		return;
	}
	check(TargetComp);

	TargetComp->ApplyGameplayEffectToSelf(
		EffectClass.GetDefaultObject(),
		1.0f,
		TargetComp->MakeEffectContext());
}
