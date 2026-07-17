#include "NG2PlayerController.h"

void ANG2PlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = PitchMin;
		PlayerCameraManager->ViewPitchMax = PitchMax;
	}
}
