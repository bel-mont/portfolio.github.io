// NG2NotifyHelpers.cpp
#include "NG2NotifyHelpers.h"
#include "Components/SkeletalMeshComponent.h"

bool NG2NotifyHelpers::IsGameWorld(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp || !MeshComp->GetOwner()) return false;
	UWorld* World = MeshComp->GetWorld();
	return World && World->IsGameWorld();
}