#pragma once

#include "CoreMinimal.h"
#include "NG2/Animation/Notifies/NG2AnimNotifyStateBase.h"
#include "NG2AttackWindow.generated.h"

UCLASS()
class NG2_API UNG2AttackWindow : public UNG2AnimNotifyStateBase
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                         float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                       const FAnimNotifyEventReference& EventReference) override;
};
