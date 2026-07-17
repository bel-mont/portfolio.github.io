#include "NG2AbilitySystemComponent.h"
#include "Abilities/NG2GameplayAbility.h"

void UNG2AbilitySystemComponent::AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (const TSubclassOf<UGameplayAbility> Ability : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		if (const UNG2GameplayAbility* NG2Ability = Cast<UNG2GameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(NG2Ability->DefaultInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void UNG2AbilitySystemComponent::AbilityInputTagTriggered(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			TryActivateAbility(AbilitySpec.Handle);
		}
	}
}