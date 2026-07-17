#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NG2RyuuAnimInstance.generated.h"

class UNG2AbilitySystemComponent;
class ANG2RyuuCharacter;
class UGameplayEffect;

UCLASS()
class NG2_API UNG2RyuuAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	TWeakObjectPtr<ANG2RyuuCharacter> CachedOwner;
	TWeakObjectPtr<UNG2AbilitySystemComponent> CachedASC;
public:
	UPROPERTY(BlueprintReadOnly, Category=Anima)
	float Speed;
	
	UPROPERTY(BlueprintReadOnly, Category=Anima)
	bool bIsInAir;
	
	UPROPERTY(BlueprintReadOnly, Category=Anima)
	bool bIsMovingHorizontally;
	
	// True when in air due to an attack, not a jump.
	// Prevents jump startup animations from playing during aerial combos.
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttackAerial = false;

	// True while State.Attacking tag is active on the ASC.
	UPROPERTY(BlueprintReadOnly)  
	bool bIsAttacking = false;

	// True while State.Jumping tag is active on the ASC.
	UPROPERTY(BlueprintReadOnly)  
	bool bIsJumping = false;
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, Category=AbilitySystem)
	bool ApplyGameplayEffectToOwner(TSubclassOf<UGameplayEffect> EffectClass);
};
