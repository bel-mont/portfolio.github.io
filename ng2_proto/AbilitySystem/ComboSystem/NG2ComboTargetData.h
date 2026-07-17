#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboEntry.h"
#include "NG2ComboTargetData.generated.h"

/**
 * Carries a resolved FNG2ComboEntry through a gameplay event from GA_AttackInput
 * to GA_MeleeExecution. Avoids writing to a shared component field and reading it
 * back, which would create an implicit ordering dependency.
 *
 * Usage (sender — GA_AttackInput):
 *   FNG2ComboTargetData* Data = new FNG2ComboTargetData();
 *   Data->ComboEntry = ResolvedEntry;
 *   FGameplayAbilityTargetDataHandle Handle;
 *   Handle.Add(Data);
 *   FGameplayEventData Payload;
 *   Payload.TargetData = Handle;
 *   UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor, Tag, Payload);
 *
 * Usage (receiver — GA_MeleeExecution, in ActivateAbility or event handler):
 *   if (TriggerEventData && TriggerEventData->TargetData.IsValid(0))
 *   {
 *       const FNG2ComboTargetData* Data =
 *           static_cast<const FNG2ComboTargetData*>(TriggerEventData->TargetData.Get(0));
 *       if (Data) { ... Data->ComboEntry ... }
 *   }
 */
USTRUCT()
struct NG2_API FNG2ComboTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	UPROPERTY()
	FNG2ComboEntry ComboEntry;

	// UScriptStruct identity — required for FGameplayAbilityTargetData subclasses.
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNG2ComboTargetData::StaticStruct();
	}
};