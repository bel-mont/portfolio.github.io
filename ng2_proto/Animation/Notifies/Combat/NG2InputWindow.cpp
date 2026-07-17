#include "NG2InputWindow.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"

void UNG2InputWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!ComboComp || !ASC) return;
	if (!ensureMsgf(ComboComp->InputWindowEffectClass,
	                TEXT("NG2InputWindow::NotifyBegin: Missing InputWindowEffectClass"))) return;

	ComboComp->InputWindowEffectHandle = ASC->ApplyGameplayEffectToSelf(
		ComboComp->InputWindowEffectClass.GetDefaultObject(), 1.f, ASC->MakeEffectContext());
}

void UNG2InputWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!ComboComp || !ASC) return;

	ASC->RemoveActiveGameplayEffect(ComboComp->InputWindowEffectHandle);
	ComboComp->InputWindowEffectHandle = FActiveGameplayEffectHandle();

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}