#include "NG2MovementBlock.h"

#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"

void UNG2MovementBlock::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                               const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!ASC) return;
	ASC->AddLooseGameplayTag(NG2GameplayTags::State_MovementBlocked);
}
