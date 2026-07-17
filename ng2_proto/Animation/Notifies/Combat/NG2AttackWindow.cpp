#include "NG2AttackWindow.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"

void UNG2AttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!ComboComp || !ASC) return;
	if (!ensureMsgf(ComboComp->AttackWindowEffectClass,
	                TEXT("NG2AttackWindow::NotifyBegin: Missing AttackWindowEffectClass"))) return;

	ComboComp->AttackWindowEffectHandle = ASC->ApplyGameplayEffectToSelf(
		ComboComp->AttackWindowEffectClass.GetDefaultObject(), 1.f, ASC->MakeEffectContext());

	// Attack window opened — flush any token buffered during the input window.
	ComboComp->FlushBuffer();
}

void UNG2AttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!ComboComp || !ASC) return;

	ASC->RemoveActiveGameplayEffect(ComboComp->AttackWindowEffectHandle);
	ComboComp->AttackWindowEffectHandle = FActiveGameplayEffectHandle();

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}