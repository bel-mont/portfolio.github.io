#pragma once

#include "CoreMinimal.h"
#include "NG2GameplayAbility.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboEntry.h"
#include "NG2AttackInputAbility.generated.h"

class UNG2MeleeExecutionAbility;
class UNG2ComboComponent;

/**
 * GA_AttackInput — lightweight instant ability bound to player input.
 * Each child GA blueprint must set its own AttackButton.
 * 
 * Responsibilities:
 *   - Build an FNG2ComboToken from AttackButton (set per Blueprint child).
 *   - Call GetNextComboEntry on the combo component.
 *   - If resolution is Play and GA_MeleeExecution is not active:
 *       activate GA_MeleeExecution via TryActivateAbilityByClass,
 *       passing the entry via FNG2ComboTargetData inside a gameplay event.
 *   - If resolution is Play and GA_MeleeExecution is already active:
 *       send a gameplay event (Tag: Event.Combo.Chain) with FNG2ComboTargetData payload.
 *   - If resolution is Buffered or Ignored: do nothing — component already handled it.
 *   - Always ends in the same frame.
 *
 * This ability is never long-lived. It owns no tasks and no montage state.
 */
UCLASS()
class NG2_API UNG2AttackInputAbility : public UNG2GameplayAbility
{
	GENERATED_BODY()

public:
	UNG2AttackInputAbility();

	/**
	 * Which button this ability instance represents.
	 * Set this in the Blueprint child (e.g. BP_GA_LightAttack sets Light,
	 * BP_GA_HeavyAttack sets Heavy). Determines the token passed to the component.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	FNG2ComboToken AttackButton = FNG2ComboToken(EAttackButton::Light, false, false);

	/**
	 * Reference to GA_MeleeExecution class. Set in Blueprint child.
	 * Used to check if execution is active and to activate it when fresh.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TSubclassOf<UNG2MeleeExecutionAbility> ExecutionAbilityClass;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	/** True if GA_MeleeExecution is currently active on the owner's ASC. */
	bool IsExecutionAbilityActive() const;

	/**
	 * Build and send an FNG2ComboTargetData payload containing Entry,
	 * tagged with EventTag, to the owning actor.
	 */
	void SendComboEvent(const FNG2ComboEntry& Entry, FGameplayTag EventTag) const;
};