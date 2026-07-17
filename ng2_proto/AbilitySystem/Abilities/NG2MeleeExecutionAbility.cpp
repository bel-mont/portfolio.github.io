#include "NG2MeleeExecutionAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboTargetData.h"
#include "NG2/NG2GameplayTags.h"

UNG2MeleeExecutionAbility::UNG2MeleeExecutionAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// State.Attacking is auto-granted while this ability is active and
	// auto-removed when it ends. GA_AttackInput and other systems query this tag.
	ActivationOwnedTags.AddTag(NG2GameplayTags::State_Attacking);

	// Block re-activation while already running. GA_AttackInput uses the chain-event
	// path instead of re-activating when this is active.
	ActivationBlockedTags.AddTag(NG2GameplayTags::State_Attacking);
}

void UNG2MeleeExecutionAbility::OnAvatarSet(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Bind to the flush-buffer delegate once, at grant time.
	// This binding persists for the lifetime of the ASC — FlushBuffer fires only
	// when this ability is active, so no spurious calls occur when it is inactive.
	if (UNG2ComboComponent* Combo = GetComboComponent())
	{
		Combo->OnComboEntryReady.AddUObject(this, &UNG2MeleeExecutionAbility::HandleComboEntryReady);
	}
}

void UNG2MeleeExecutionAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!TryCommitOrEnd(Handle, ActorInfo, ActivationInfo)) return;

	CurrentSpecHandle = Handle;
	CurrentActorInfo = ActorInfo;
	CurrentActivationInfo = ActivationInfo;
	// Start listening for chain events from GA_AttackInput (attack-window path).
	// This task lives for the full ability lifetime — it is not per-montage.
	UAbilityTask_WaitGameplayEvent* ChainEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		NG2GameplayTags::Event_Combo_Chain,
		nullptr,     // optional target actor filter — not needed
		false,       // only trigger once? No — listen for the full chain
		true);       // only match exact tag
	ChainEventTask->EventReceived.AddDynamic(this, &UNG2MeleeExecutionAbility::HandleChainEvent);
	ChainEventTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* LandTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		NG2GameplayTags::Event_Landed,
		nullptr,
		true,
		true);
	
	LandTask->EventReceived.AddDynamic(this, &UNG2MeleeExecutionAbility::HandleLandedEvent);
	LandTask->ReadyForActivation();
}

void UNG2MeleeExecutionAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clean up montage task if it is still running (e.g. external cancel path).
	if (CurrentMontageTask)
	{
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}

	// Remove tags used during combo
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (int32 TagCount = ASC->GetGameplayTagCount(NG2GameplayTags::State_AirAttacking); TagCount > 0)
		{
			ASC->RemoveLooseGameplayTag(NG2GameplayTags::State_AirAttacking, TagCount);
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UNG2MeleeExecutionAbility::PlayComboMontage(const FNG2ComboEntry& Entry)
{
	if (!Entry.Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Execution] PlayComboMontage: Entry has null Montage."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Execution] PlayComboMontage: %s"), *Entry.Montage->GetName());
	// Stop the previous montage task cleanly.
	// EndTask does not fire the OnCompleted/OnInterrupted delegates,
	// so no risk of double-ending the ability.
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveAll(this);
		CurrentMontageTask->OnInterrupted.RemoveAll(this);
		CurrentMontageTask->OnCancelled.RemoveAll(this);
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}

	SetCurrentMontage(Entry.Montage);

	// Tell the component which entry is now executing so GetNextComboEntry
	// can build candidate sequences from it.
	if (UNG2ComboComponent* Combo = GetComboComponent())
	{
		Combo->SetCurrentExecutingKey(Entry.GetComboKey());
	}

	// By default, this stops the montage and unbinds the listeners
	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, GetCurrentMontage(), 1.0f);

	CurrentMontageTask->OnCompleted.AddDynamic(this, &UNG2MeleeExecutionAbility::OnMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UNG2MeleeExecutionAbility::OnMontageInterrupted);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UNG2MeleeExecutionAbility::OnMontageInterrupted);

	CurrentMontageTask->ReadyForActivation();
	// If this move has a loop section, wire it up immediately after activation.
	if (!Entry.LoopSection.IsNone())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->CurrentMontageSetNextSectionName(Entry.LoopSection, Entry.LoopSection);
		}
		PendingLandSection = Entry.LandSection; // cache for landing transition
	}
	else
	{
		PendingLandSection = NAME_None;
	}
}

void UNG2MeleeExecutionAbility::HandleComboEntryReady(const FNG2ComboEntry& Entry)
{
	UE_LOG(LogTemp, Warning, TEXT("[Execution] HandleComboEntryReady called. IsActive: %d"), IsActive() ? 1 : 0);
	// Fired by FlushBuffer when the attack window opens and a buffered token resolves.
	// Guard: only act if this ability is currently active.
	if (!IsActive()) return;

	UE_LOG(LogTemp, Warning, TEXT("[Execution] HandleComboEntryReady: chaining to %s"),
		Entry.Montage ? *Entry.Montage->GetName() : TEXT("NULL"));

	PlayComboMontage(Entry);
}

void UNG2MeleeExecutionAbility::HandleChainEvent(FGameplayEventData EventData)
{
	// Fired by GA_AttackInput via SendGameplayEventToActor(Event.Combo.Chain)
	// when the attack window is open and the ability is already active.
	// Also on fresh starts (new combo)
	FNG2ComboEntry Entry;
	if (!ExtractEntryFromEvent(&EventData, Entry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Execution] HandleChainEvent: invalid payload, ignoring."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Execution] HandleChainEvent: chaining to %s"),
		Entry.Montage ? *Entry.Montage->GetName() : TEXT("NULL"));

	PlayComboMontage(Entry);
}

void UNG2MeleeExecutionAbility::HandleLandedEvent(FGameplayEventData EventData)
{
	if (PendingLandSection.IsNone()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	ASC->CurrentMontageJumpToSection(PendingLandSection);
	PendingLandSection = NAME_None;
	// OnMontageCompleted fires naturally when Land section finishes.
}

void UNG2MeleeExecutionAbility::OnMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("[Execution] OnMontageCompleted."));
	CurrentMontageTask = nullptr;
	SetCurrentMontage(nullptr);

	if (UNG2ComboComponent* Combo = GetComboComponent())
	{
		// TODO: [SEQUENCE RESET TIMER] Start timer here instead of ClearSequence().
		Combo->ClearSequence();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UNG2MeleeExecutionAbility::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Warning, TEXT("[Execution] OnMontageInterrupted."));
	CurrentMontageTask = nullptr;
	SetCurrentMontage(nullptr);

	if (UNG2ComboComponent* Combo = GetComboComponent())
	{
		Combo->ClearSequence();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

bool UNG2MeleeExecutionAbility::ExtractEntryFromEvent(
	const FGameplayEventData* EventData, FNG2ComboEntry& OutEntry) const
{
	if (!EventData) return false;
	if (!EventData->TargetData.IsValid(0)) return false;

	const FNG2ComboTargetData* Data =
		static_cast<const FNG2ComboTargetData*>(EventData->TargetData.Get(0));

	if (!Data) return false;

	OutEntry = Data->ComboEntry;
	return true;
}