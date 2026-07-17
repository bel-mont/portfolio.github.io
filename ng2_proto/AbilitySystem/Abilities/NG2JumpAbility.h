#pragma once

#include "CoreMinimal.h"
#include "NG2GameplayAbility.h"
#include "NG2JumpAbility.generated.h"

UCLASS()
class NG2_API UNG2JumpAbility : public UNG2GameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
private:
	virtual void ExecuteAbility(const FGameplayAbilitySpecHandle Handle,
	                            const FGameplayAbilityActorInfo* ActorInfo,
	                            const FGameplayAbilityActivationInfo ActivationInfo,
	                            const FGameplayEventData* TriggerEventData);
};
