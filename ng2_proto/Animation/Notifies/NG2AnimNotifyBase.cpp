#include "NG2AnimNotifyBase.h"
#include "NG2NotifyHelpers.h"
#include "NG2/Player/NG2RyuuCharacter.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"

void UNG2AnimNotifyBase::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	Character = nullptr;
	ComboComp = nullptr;
	ASC = nullptr;

	if (!NG2NotifyHelpers::IsGameWorld(MeshComp)) return;

	Character = Cast<ANG2RyuuCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	ComboComp = Character->FindComponentByClass<UNG2ComboComponent>();
	ASC = Cast<UNG2AbilitySystemComponent>(Character->GetAbilitySystemComponent());
}