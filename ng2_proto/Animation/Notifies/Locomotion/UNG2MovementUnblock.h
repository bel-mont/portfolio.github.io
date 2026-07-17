#pragma once

#include "CoreMinimal.h"
#include "NG2/Animation/Notifies/NG2AnimNotifyBase.h"
#include "UNG2MovementUnblock.generated.h"

UCLASS()
class NG2_API UUNG2MovementUnblock : public UNG2AnimNotifyBase
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
