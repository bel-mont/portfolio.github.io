#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NG2GameplayAbility.generated.h"

class UNG2ComboComponent;

UCLASS()
class NG2_API UNG2GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category=Input)
	FGameplayTag DefaultInputTag;
	
protected:
	UNG2ComboComponent* GetComboComponent() const;
	
	bool TryCommitOrEnd(const FGameplayAbilitySpecHandle Handle,
						const FGameplayAbilityActorInfo* ActorInfo,
						const FGameplayAbilityActivationInfo& ActivationInfo);
	
	void ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> EffectClass);
};
