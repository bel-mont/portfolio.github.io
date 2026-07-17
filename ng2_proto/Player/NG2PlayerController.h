#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NG2PlayerController.generated.h"

UCLASS()
class NG2_API ANG2PlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	/**
	 * Lower limit for camera pitch in degrees
	 * 
	 */
	UPROPERTY(EditDefaultsOnly, Category=Camera)
	float PitchMin = -15.0f;

	/**
	 * Upper limit for camera pitch in degrees
	 */
	UPROPERTY(EditDefaultsOnly, Category=Camera)
	float PitchMax = 40.0f;
	
	virtual void BeginPlay() override;
};
