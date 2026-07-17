#pragma once

#include "CoreMinimal.h"
#include "NG2GameplayAbility.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboEntry.h"
#include "NG2MeleeExecutionAbility.generated.h"

class UNG2ComboComponent;
class UAbilityTask_PlayMontageAndWait;

/**
 * GA_MeleeExecution — long-lived ability that owns montage playback.
 *
 * Activation:
 *   Never bound to player input. Activated by GA_AttackInput via gameplay event
 *   tagged Event.Combo.Activate. GA_MeleeExecution must have that tag configured
 *   in its Triggers list in the ability's GameplayAbility settings.
 *
 * Lifetime:
 *   Stays alive for the full duration of a combo chain.
 *   Ends when:
 *     - Montage completes naturally with no buffered continuation (OnMontageCompleted).
 *     - External cancel (stagger, knockback, death) fires OnMontageInterrupted.
 *     - Sequence reset timer expires (TODO).
 *
 * Combo chaining:
 *   Two paths bring the next entry in:
 *     1. FlushBuffer path: component's OnComboEntryReady delegate fires when the
 *        attack window opens and a buffered token resolves. Bound in OnAvatarSet.
 *     2. Direct event path: GA_AttackInput sends Event.Combo.Chain with an
 *        FNG2ComboTargetData payload when the execution ability is already active
 *        and the attack window is open. Handled via WaitGameplayEvent task.
 *
 *   Both paths call PlayComboMontage, which stops the current task and starts a new one.
 *   The animations should blend between.
 *
 * Tags:
 *   ActivationOwnedTags: State.Attacking — auto-granted while this ability is active,
 *   auto-removed when it ends. Other systems (including GA_AttackInput) read this tag.
 */
UCLASS()
class NG2_API UNG2MeleeExecutionAbility : public UNG2GameplayAbility
{
	GENERATED_BODY()

public:
	UNG2MeleeExecutionAbility();

	virtual void OnAvatarSet(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

public:
	/** Stop current task and play the montage for Entry. */
	void PlayComboMontage(const FNG2ComboEntry& Entry);

private:
	

	// ── State ─────────────────────────────────────────────────────────────────

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	// Cached activation info — needed to call EndAbility from callbacks.
	FGameplayAbilitySpecHandle CurrentSpecHandle;
	
	const FGameplayAbilityActorInfo* CurrentActorInfo = nullptr;
	
	FGameplayAbilityActivationInfo CurrentActivationInfo;
	
	FName PendingLandSection = NAME_None;
	
	/**
	 * Extract FNG2ComboEntry from an FGameplayEventData payload.
	 * Returns false if the payload does not contain valid FNG2ComboTargetData.
	 */
	bool ExtractEntryFromEvent(const FGameplayEventData* EventData, FNG2ComboEntry& OutEntry) const;

	/** Delegate handler: component's OnComboEntryReady (flush-buffer path). */
	void HandleComboEntryReady(const FNG2ComboEntry& Entry);

	/** Called when the WaitGameplayEvent task fires (chain-event path). */
	UFUNCTION()
	void HandleChainEvent(FGameplayEventData EventData);

	/**
	 * Called when the WaitGameplayEvent task fires (land-event path).
	 * Usually called when the character lands after a melee attack, to trigger the "landing" part of the animation.
	 * @param EventData 
	 */
	UFUNCTION()
	void HandleLandedEvent(FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();
};