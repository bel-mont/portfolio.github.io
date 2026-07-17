#pragma once

#include "CoreMinimal.h"
#include "NG2/Animation/Notifies/NG2AnimNotifyBase.h"
#include "NG2MovementBlock.generated.h"

UCLASS()
class NG2_API UNG2MovementBlock : public UNG2AnimNotifyBase
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
