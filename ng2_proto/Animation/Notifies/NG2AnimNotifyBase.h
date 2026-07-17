#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NG2AnimNotifyBase.generated.h"

class ANG2RyuuCharacter;
class UNG2ComboComponent;
class UNG2AbilitySystemComponent;

UCLASS(Abstract)
class NG2_API UNG2AnimNotifyBase : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	TObjectPtr<ANG2RyuuCharacter> Character;

	UPROPERTY(Transient)
	TObjectPtr<UNG2ComboComponent> ComboComp;

	UPROPERTY(Transient)
	TObjectPtr<UNG2AbilitySystemComponent> ASC;

	// Populates Character, ComboComp, and ASC. Child classes must call
	// Super::Notify before accessing any of them.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};