#include "WanderersActionSystemComponent.h"

#include "WanderersAction.h"
#include "WanderersActionSystemTypes.h"
#include "WanderersAttributeSet.h"
#include "WanderersOfTheMist/WanderersOfTheMist.h"

UWanderersActionSystemComponent::UWanderersActionSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UWanderersActionSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	Attributes->InitializeAttributes();
}

void UWanderersActionSystemComponent::SetDefaultAttributeSet(TSubclassOf<UWanderersAttributeSet> AttributeSetClass)
{
	check(!HasBeenInitialized());
	// Because we cannot do this:
	// Attributes = CreateDefaultSubobject<AttributeSetClass>(TEXT("Attributes"));
	// We use this other method
	FObjectInitializer& ObjectInitializer = FObjectInitializer::Get();
	Attributes = Cast<UWanderersAttributeSet>(
		ObjectInitializer.CreateDefaultSubobject(this, TEXT("Attributes"), AttributeSetClass, AttributeSetClass));
}

void UWanderersActionSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (Attributes == nullptr)
	{
		Attributes = NewObject<UWanderersAttributeSet>(this, UWanderersAttributeSet::StaticClass());
		UE_LOG(LogGame, Warning, TEXT("No default AttributeSet defined. Set using SetDefaultAttributeSet() "
			       "during construction, or assign in Blueprint ActionComponent for %s."), *GetNameSafe(GetOwner()));
	}

	for (TFieldIterator<FStructProperty> PropIt(Attributes->GetClass()); PropIt; ++PropIt)
	{
		FWanderersAttribute* FoundAttribute = PropIt->ContainerPtrToValuePtr<FWanderersAttribute>(Attributes);
		// Make sure these attribute and tags match the available GameplayTags
		FName AttributeTagName = FName("Attribute." + PropIt->GetName());
		FGameplayTag GameplayTag = FGameplayTag::RequestGameplayTag(AttributeTagName);
		CachedAttributes.Add(GameplayTag, FoundAttribute);
	}

	// Grant default actions defined in Blueprint CDO.
	for (const TSubclassOf<UWanderersAction>& ActionClass : DefaultActions)
	{
		if (ensure(ActionClass))
		{
			GrantAction(ActionClass);
		}
	}
}

// -----------------------------------------------------------------------------
// Turn Lifecycle
// -----------------------------------------------------------------------------
void UWanderersActionSystemComponent::TurnStart()
{
	TickDurationTags();
	TickActionCooldowns();
}

void UWanderersActionSystemComponent::TickDurationTags()
{
	TArray<FGameplayTag> TagsToRemove;
	for (TTuple<FGameplayTag, FWanderersTagState>& Pair : ActiveTags)
	{
		if (!Pair.Value.bIsDurationBased) continue;
		Pair.Value.Count--;
		if (Pair.Value.Count <= 0)
		{
			TagsToRemove.Add(Pair.Key);
		}
	}

	for (FGameplayTag& Tag : TagsToRemove)
	{
		RemoveTag(Tag);
	}
}

void UWanderersActionSystemComponent::TickActionCooldowns()
{
	for (UWanderersAction* Action : Actions)
	{
		if (Action)
		{
			Action->TickCooldown();
		}
	}
}


// -----------------------------------------------------------------------------
// Action Management
// -----------------------------------------------------------------------------

void UWanderersActionSystemComponent::GrantAction(TSubclassOf<UWanderersAction> ActionClass)
{
	// TODO: Check for duplicate action tags before granting if needed.
	UWanderersAction* NewAction = NewObject<UWanderersAction>(this, ActionClass);
	Actions.Add(NewAction);
}

void UWanderersActionSystemComponent::GrantActions(TArray<TSubclassOf<UWanderersAction>> ActionClasses)
{
	for (TSubclassOf<UWanderersAction> ActionClass : ActionClasses)
	{
		GrantAction(ActionClass);
	}
}

void UWanderersActionSystemComponent::RevokeAction(UWanderersAction* Action)
{
	Actions.RemoveSingle(Action);
}

void UWanderersActionSystemComponent::RevokeActionsByTags(const TArray<FGameplayTag>& ActionTags)
{
	for (const FGameplayTag ActionTag : ActionTags)
	{
		UWanderersAction* Action = GetActionByTag(ActionTag);
		if (Action) RevokeAction(Action);
	}
}

void UWanderersActionSystemComponent::StartAction(FGameplayTag ActionTag, AActor* Target)
{
	for (UWanderersAction* Action : Actions)
	{
		if (Action && Action->GetActionTag() == ActionTag)
		{
			if (Action->CanStart())
			{
				Action->StartAction(Target);
			}
			else
			{
				// TODO: Review if we need this log at all
				UE_LOG(LogGame, Warning, TEXT("Action %s cannot be started"), *Action->GetName());
			}
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("WanderersActionSystemComponent: Action %s not found on %s."), *ActionTag.ToString(),
	       *GetNameSafe(GetOwner()));
}

FWanderersAttribute* UWanderersActionSystemComponent::GetAttribute(FGameplayTag InAttributeTag) const
{
	FWanderersAttribute* const* FoundAttribute = CachedAttributes.Find(InAttributeTag);
	return *FoundAttribute;
}

float UWanderersActionSystemComponent::GetAttributeValue(FGameplayTag AttributeTag) const
{
	return GetAttribute(AttributeTag)->GetValue();
}

FOnAttributeChanged& UWanderersActionSystemComponent::GetAttributeListener(FGameplayTag AttributeTag)
{
	return AttributeListeners.FindOrAdd(AttributeTag);
}

void UWanderersActionSystemComponent::AddAttributeDynamicListener(FOnAttributeDynamicChanged Event,
	FGameplayTag AttributeTag)
{
	TArray<FOnAttributeDynamicChanged>& Events = AttributeDynamicListeners.FindOrAdd(AttributeTag);
	Events.Add(Event);
}

void UWanderersActionSystemComponent::RemoveAttributeDynamicListener(FOnAttributeDynamicChanged Event)
{
	for (TTuple<FGameplayTag, TArray<FOnAttributeDynamicChanged>>& Listener : AttributeDynamicListeners)
	{
		if (Listener.Value.Remove(Event) > 0)
		{
			UE_LOG(LogGame, Log, TEXT("Cleaned up expired attribute delegate for %s"), *GetNameSafe(GetOwner()));
			break;
		}
	}
}

// -----------------------------------------------------------------------------
// Tag / Token Management
// -----------------------------------------------------------------------------

void UWanderersActionSystemComponent::AddTag(FGameplayTag Tag, FWanderersTagState TagState)
{
	// If the tag already exists, update the count. For now, do not put an upper limit
	// If the tag does not exist, add it and set its count
	// TODO: Review limiting tag count.
	if (ActiveTags.Contains(Tag))
	{
		FWanderersTagState* Existing = ActiveTags.Find(Tag);
		Existing->Count += TagState.Count;
		TagState.Count = Existing->Count; // Updated for the Broadcast.
	}
	else
	{
		ActiveTags.Add(Tag, TagState);
	}
	
	TagChangedListeners.Broadcast(ActiveTags);
	// TagDynamicListeners.ExecuteIfBound(Tag, TagState);
}

void UWanderersActionSystemComponent::RemoveTag(FGameplayTag Tag, bool bTriggerEvent)
{
	ActiveTags.Remove(Tag);
	if (bTriggerEvent)
	{
		UE_LOG(LogGame, Log, TEXT("Individual Tag Removed event triggered: %s"), *Tag.ToString());
		TagChangedListeners.Broadcast(ActiveTags);
	}
}

bool UWanderersActionSystemComponent::ConsumeTag(FGameplayTag Tag, FWanderersTagState StateChange, bool ConsumeDurationToken)
{
	FWanderersTagState* CurrentState = ActiveTags.Find(Tag);
	if (!CurrentState) return false;
	
	// For duration based tokens, we don't want to consume them here.
	// The duration is ticked down at a later point.
	if (!ConsumeDurationToken && CurrentState->bIsDurationBased)
	{
		return false;
	}
	CurrentState->Count = FMath::Max(0, CurrentState->Count - StateChange.Count);
	
	if (CurrentState->Count <= 0)
	{
		RemoveTag(Tag);
	}
	TagChangedListeners.Broadcast(ActiveTags);
	// TagDynamicListeners.ExecuteIfBound(Tag, TagState);
	return true;
}

bool UWanderersActionSystemComponent::HasTag(FGameplayTag Tag) const
{
	return ActiveTags.Contains(Tag);
}

bool UWanderersActionSystemComponent::HasAllTags(FGameplayTagContainer Tags) const
{
	for (const FGameplayTag& Tag : Tags)
	{
		if (!HasTag(Tag)) return false;
	}
	return true;
}

bool UWanderersActionSystemComponent::HasAnyTag(FGameplayTagContainer Tags) const
{
	for (const FGameplayTag& Tag : Tags)
	{
		if (HasTag(Tag)) return true;
	}
	return false;
}

void UWanderersActionSystemComponent::ApplyAttributeChange(FGameplayTag AttributeTag, int Delta,
                                                           EAttributeModifyType ModifyType)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WanderersActionSystemComponent: Invalid attribute tag %s"), *AttributeTag.ToString());
		return;
	}
	FWanderersAttribute* FoundAttribute = GetAttribute(AttributeTag);
	check(FoundAttribute);
	float OldValue = FoundAttribute->GetValue();
	switch (ModifyType)
	{
	case EAttributeModifyType::Base:
		FoundAttribute->Base += Delta;
		break;
	case EAttributeModifyType::Modifier:
		FoundAttribute->Modifier += Delta;
		break;
	case EAttributeModifyType::OverrideBase:
		FoundAttribute->Base = Delta;
		break;
	default:
		check(false);
	}
	
	Attributes->PostAttributeChange();
	// Native C++ Listeners
	if (FOnAttributeChanged* Event = AttributeListeners.Find(AttributeTag))
	{
		Event->Broadcast(AttributeTag, FoundAttribute->GetValue(), OldValue);
	}
	// Blueprints listeners
	if (TArray<FOnAttributeDynamicChanged>* Events = AttributeDynamicListeners.Find(AttributeTag))
	{
		for (int i = Events->Num() - 1; i >= 0; --i)
		{
			FOnAttributeDynamicChanged& Event = (*Events)[i];
			bool bIsBound = Event.ExecuteIfBound(AttributeTag, FoundAttribute->GetValue(), OldValue);
			if (!bIsBound)
			{
				Events->RemoveAt(i);
				UE_LOG(LogGame, Log, TEXT("Cleaned up expired attribute delegate for %s"), *GetNameSafe(GetOwner()));
			}
		}
	}
	
	UE_LOGFMT(LogGame, Log, "Attribute: {0}, New: {1}, Old: {2}", AttributeTag.ToString(), FoundAttribute->GetValue(), OldValue);	
}

TArray<UWanderersAction*> UWanderersActionSystemComponent::GetActionsByClasses(
	const TArray<TSubclassOf<UWanderersAction>>& ActionClasses)
{
	TArray<UWanderersAction*> Result;
	// O(n^2) but we have less than 15~ total actions at any given time.
	// Consider changing it later for elegance or performance reasons.
	for (UWanderersAction* Action : Actions)
	{
		if (Action && ActionClasses.Contains(Action->GetClass()))
		{
			Result.Add(Action);
		}
	}
	return Result;
}

UWanderersAction* UWanderersActionSystemComponent::GetActionByTag(FGameplayTag ActionTag)
{
	for (UWanderersAction* Action : Actions)
	{
		if (Action && Action->GetActionTag() == ActionTag)
		{
			return Action;
		}
	}
	return nullptr;
}
