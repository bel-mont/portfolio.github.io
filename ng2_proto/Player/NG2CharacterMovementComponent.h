#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NG2CharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NG2_API UNG2CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	UNG2CharacterMovementComponent();

public:
	// Multiplier applied to gravity while Velocity.Z < -FallGravityApexThreshold (descending)
	// Determines how fast the character falls after reaching apex
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float FallGravityMultiplier = 2.2f;

	// Multiplier applied while |Velocity.Z| <= FallGravityApexThreshold (apex hang window)
	// Determines the rate at which the character slows down while hanging in the air
	// Rates closer to zero mean the character hangs for longer with no movement,
	// while rates closer to one mean the character hangs for less time with more movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float ApexGravityMultiplier = 0.5f;
	
	// Z velocity magnitude (cm/s) that defines the apex window on both sides of zero
	// Determines the range of Z speed at which the character can hang in the air
	// A lower number means that the character "hangs" for less time
	// A higher number means that the character can "hang" for longer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float FallGravityApexThreshold = 120.f;

	/**
	 * Z velocity magnitude to determine how fast we ascend
	 * This causes the character to reach the apex faster,
	 * meaning that our jump height would be lowered.
	 * Balance it with the Jump Z velocity to increase jump height.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float AscentGravityMultiplier = 2.f;

	/**
	 * Modifies the horizontal speed when jumping
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float ForwardJumpSpeed = 800.f;
	
	/**
	 * Determines jump and fall speeds. Used to determine the "feel" of these movements.
	 * @return 
	 */
	virtual float GetGravityZ() const override;
};
