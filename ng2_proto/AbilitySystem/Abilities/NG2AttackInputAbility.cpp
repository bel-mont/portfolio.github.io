#include "NG2AttackInputAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NG2MeleeExecutionAbility.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboTargetData.h"
#include "NG2/NG2GameplayTags.h"

UNG2AttackInputAbility::UNG2AttackInputAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// This ability ends in the same frame. No blocking tags needed on self.
	// GA_MeleeExecution will hold State.Attacking while it is alive,
	// which prevents this ability from needing to guard against re-entry issues.
}

void UNG2AttackInputAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!TryCommitOrEnd(Handle, ActorInfo, ActivationInfo)) return;

	UNG2ComboComponent* Combo = GetComboComponent();
	if (!Combo)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttackInput] No combo component found."));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// TODO: [FORWARD SAMPLING] Sample bForward here before building the token.
	// ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	// float Dot = FVector::DotProduct(Char->GetActorForwardVector(),
	//     Char->GetCharacterMovement()->GetLastInputVector().GetSafeNormal());
	// bool bForward = Dot >= ForwardArcThreshold;
	// Then pass bForward into the token constructor below.

	bool bFound = false;
	ENG2ComboResolution Resolution = ENG2ComboResolution::Ignored;
	const FNG2ComboEntry Entry = Combo->GetNextComboEntry(AttackButton, bFound, Resolution);

	if (bFound && Resolution == ENG2ComboResolution::Play)
	{
		if (!IsExecutionAbilityActive())
		{
			// Fresh activation. Send event that also triggers GA_MeleeExecution.
			// GA_MeleeExecution must have this tag configured in its Triggers list.
			GetAbilitySystemComponentFromActorInfo()->TryActivateAbilityByClass(ExecutionAbilityClass, false);
		}
		SendComboEvent(Entry, NG2GameplayTags::Event_Combo_Chain);
	}
	// Buffered or Ignored: component already handled state. Nothing to do here.
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool UNG2AttackInputAbility::IsExecutionAbilityActive() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ExecutionAbilityClass) return false;

	return ASC->HasMatchingGameplayTag(NG2GameplayTags::State_Attacking);
}

void UNG2AttackInputAbility::SendComboEvent(const FNG2ComboEntry& Entry, FGameplayTag EventTag) const
{
	// Heap-allocate the target data. FGameplayAbilityTargetDataHandle takes ownership
	// via TSharedPtr internally — do not delete manually.
	FNG2ComboTargetData* Data = new FNG2ComboTargetData();
	Data->ComboEntry = Entry;

	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(Data);

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.TargetData = DataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetAvatarActorFromActorInfo(),
		EventTag,
		Payload);
}