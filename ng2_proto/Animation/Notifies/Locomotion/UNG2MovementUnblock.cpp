#include "UNG2MovementUnblock.h"

#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"

void UUNG2MovementUnblock::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!ASC) return;
	if (int32 TagCount = ASC->GetGameplayTagCount(NG2GameplayTags::State_MovementBlocked); TagCount > 0)
	{
		ASC->RemoveLooseGameplayTag(NG2GameplayTags::State_MovementBlocked);
	}
}
