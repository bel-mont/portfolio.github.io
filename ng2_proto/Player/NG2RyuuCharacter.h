#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "NG2/AbilitySystem/NG2AttributeSet.h"
#include "NG2RyuuCharacter.generated.h"

class UNG2CharacterMovementComponent;
class UNG2ComboComponent;
class UInputAction;
class UNG2InputConfig;
struct FInputActionValue;
class UAttributeSet;
class UNG2AbilitySystemComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class NG2_API ANG2RyuuCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	explicit ANG2RyuuCharacter(const FObjectInitializer& ObjectInitializer);

	UAttributeSet* GetAttributeSet() const { return AttributeSet; };
	
	UNG2ComboComponent* GetComboComponent() const { return ComboComponent; };
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UNG2CharacterMovementComponent* GetNG2MovementComponent() const;

protected:
#pragma region Components
	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, Category=Combat)
	TObjectPtr<UStaticMeshComponent> Weapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combat)
	TObjectPtr<UNG2ComboComponent> ComboComponent;
#pragma endregion

#pragma region AbilitySystem
	UPROPERTY(VisibleAnywhere, Category=AbilitySystem)
	TObjectPtr<UNG2AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category=AbilitySystem)
	TObjectPtr<UNG2AttributeSet> AttributeSet;

	/**
	 * Add abilities here to trigger them with GAS.
	 */
	UPROPERTY(EditAnywhere, Category=AbilitySystem)
	TArray<TSubclassOf<UGameplayAbility>> ActiveAbilities;

#pragma region GameplayEffects
	UPROPERTY(EditAnywhere, Category=AbilitySystem)
	TSubclassOf<UGameplayEffect> LandedEffectClass;
#pragma endregion

#pragma endregion

#pragma region Input
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> Input_Move;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UInputAction> Input_Look;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TObjectPtr<UNG2InputConfig> InputConfig;
#pragma endregion
#pragma region CameraControl
	/**
	 * Adjusts camera pitch (up and down) sensitivity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float LookPitchMultiplier = .5f;

	/**
	 * Adjusts camera yaw (left and right) sensitivity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float LookYawMultiplier = 1.0f;
#pragma endregion 
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;

	void AbilityCallback(FGameplayTag InputTag);
	
	void AddAbility();

	virtual void Landed(const FHitResult& Hit) override;
	
	void Move(const FInputActionValue& Input);

	void Look(const FInputActionValue& Input);
};
