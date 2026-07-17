#include "NG2AnimNotifyStateBase.h"
#include "NG2NotifyHelpers.h"
#include "NG2/Player/NG2RyuuCharacter.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"

void UNG2AnimNotifyStateBase::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	Character = nullptr;
	ComboComp = nullptr;
	ASC = nullptr;

	if (!NG2NotifyHelpers::IsGameWorld(MeshComp)) return;

	Character = Cast<ANG2RyuuCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	ComboComp = Character->FindComponentByClass<UNG2ComboComponent>();
	ASC = Cast<UNG2AbilitySystemComponent>(Character->GetAbilitySystemComponent());
}

void UNG2AnimNotifyStateBase::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	Character = nullptr;
	ComboComp = nullptr;
	ASC = nullptr;
}
