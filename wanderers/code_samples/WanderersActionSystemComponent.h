#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WanderersActionSystemTypes.h"
#include "WanderersAttributeSet.h"
#include "Components/ActorComponent.h"
#include "WanderersActionSystemComponent.generated.h"

class UWanderersEffectAsset;
class UWanderersAction;
class UWanderersAttributeSet;
class UWanderersTokenBase;

// NOTE: _DYNAMIC_ Is mostly used to enable Blueprint support for the delegate. Non Dynamic provides better performance
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, OldHealth);
// Native C++ Delegates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayTag /* AttributeTag */, float /* NewAttributeValue */, float /*OldAttributeValue*/);
// Blueprint Delegates
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnAttributeDynamicChanged, FGameplayTag, AttributeTag, float, NewAttributeValue, float, OldAttributeValue);

// Tag delegates
typedef TMap<FGameplayTag, FWanderersTagState> FWanderersTagStateMap;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTagChanged, const FWanderersTagStateMap& /* TagStates */);

// -----------------------------------------------------------------------------
// UWanderersActionSystemComponent
//
// Single component on every combat character (player and enemy).
// Owns: actions, turn-based cooldowns, active tag state, and the attribute set.
//
// Add to a character in its constructor:
//   ActionSystemComp = CreateDefaultSubobject<UWanderersActionSystemComponent>(
//       TEXT("ActionSystemComp"));
// -----------------------------------------------------------------------------
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WANDERERSOFTHEMIST_API UWanderersActionSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWanderersActionSystemComponent();

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	
	void SetDefaultAttributeSet(TSubclassOf<UWanderersAttributeSet> AttributeSetClass);

	// Called by TurnManager when this character's turn begins.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Turn")
	void TurnStart();

#pragma region Actions
	// Grants an action to this character. Called during character initialization.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Actions")
	void GrantAction(TSubclassOf<UWanderersAction> ActionClass);
	
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Actions")
	void GrantActions(TArray<TSubclassOf<UWanderersAction>> ActionClasses);

	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Actions")
	void RevokeAction(UWanderersAction* Action);
	
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Actions")
	void RevokeActionsByTags(const TArray<FGameplayTag>& ActionTags);
	
	// Attempts to start the action matching ActionTag. Fails silently if not found or CanStart() == false.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Actions")
	void StartAction(FGameplayTag ActionTag, AActor* Target);

	// Returns all granted actions. Used by UI to populate the action panel.
	UFUNCTION(BlueprintPure, Category = "ActionSystem|Actions")
	TArray<UWanderersAction*> GetActions() { return Actions; }

	UFUNCTION(BlueprintPure, Category = "ActionSystem|Actions")
	TArray<UWanderersAction*> GetActionsByClasses(const TArray<TSubclassOf<UWanderersAction>>& ActionClasses);
	
	UFUNCTION(BlueprintPure, Category = "ActionSystem|Actions")
	UWanderersAction* GetActionByTag(FGameplayTag ActionTag);
	
#pragma endregion

#pragma region Attribute Management
	// Adding const at the end tells the compiler that this function does not modify the object state
	FWanderersAttribute* GetAttribute(FGameplayTag InAttributeTag) const;
	
	UFUNCTION(BlueprintCallable)
	float GetAttributeValue(UPARAM(meta = (Categories="Attribute")) FGameplayTag AttributeTag) const;
	
	FOnAttributeChanged& GetAttributeListener(FGameplayTag AttributeTag);
	
	UFUNCTION(BlueprintCallable, DisplayName="Add Attribute Listener", meta=(Keywords="events,delegate"))
	void AddAttributeDynamicListener(FOnAttributeDynamicChanged Event, FGameplayTag AttributeTag);

	UFUNCTION(BlueprintCallable, DisplayName="Remove Attribute Listener", meta=(Keywords="events,delegate"))
	void RemoveAttributeDynamicListener(FOnAttributeDynamicChanged Event);
	
	// SetAttribute value.
	UFUNCTION(BlueprintCallable)
	void ApplyAttributeChange(FGameplayTag AttributeTag, int Delta, EAttributeModifyType ModifyType);
 
#pragma endregion

#pragma region Token Management
	// The GAS ASC does gameplay tag management for us, but in this case we need to set up our own.

	// Adds a new tag with the provided TagState. If the tag already exists, uses the tag state as a delta.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Tags")
	void AddTag(FGameplayTag Tag, FWanderersTagState TagState);

	// Removes the tag entirely regardless of Count.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Tags")
	void RemoveTag(FGameplayTag Tag, bool bTriggerEvent = false);

	// Decrements Count by 1. Removes the tag at 0. Returns true if the tag was present.
	// Use for consumption tokens (Shield, Miss, etc.) when they trigger.
	// Duration based tokens will be ignored.
	UFUNCTION(BlueprintCallable, Category = "ActionSystem|Tags")
	bool ConsumeTag(FGameplayTag Tag, FWanderersTagState StateChange, bool ConsumeDurationToken = false);

	// Fast presence check. Delegates to ActiveTags.
	UFUNCTION(BlueprintPure, Category = "ActionSystem|Tags")
	bool HasTag(FGameplayTag Tag) const;
	
	UFUNCTION(BlueprintPure, Category = "ActionSystem|Tags")
	bool HasAllTags(FGameplayTagContainer Tags) const;
	
	UFUNCTION(BlueprintPure, Category = "ActionSystem|Tags")
	bool HasAnyTag(FGameplayTagContainer Tags) const;

	// Used by UWanderersAction::CanStart() for RequiredTags / BlockedTags checks
	// and by the targeting widget for position tag queries.
	const TMap<FGameplayTag, FWanderersTagState>& GetActiveTags() const { return ActiveTags; }
	
	TMap<FGameplayTag, FWanderersTagState> GetActiveTagsCopy() { return ActiveTags; }
	
	// C++ native listeners for Tag changes. No BP listeners for now, until needed.
	FOnTagChanged TagChangedListeners;
#pragma endregion 
protected:
#pragma region Attributes
	// Assigned in Blueprint via EditAnywhere + Instanced, same pattern as WanderersActionSystemComponent.
	// Defaults to UWanderersHealthAttributeSet if none is set.
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category = "ActionSystem")
	TObjectPtr<UWanderersAttributeSet> Attributes;
	
	TMap<FGameplayTag, FWanderersAttribute*> CachedAttributes;
	
	// C++ native listeners for Attribute changes
	TMap<FGameplayTag, FOnAttributeChanged> AttributeListeners;

	// Blueprint callable listeners for Attribute changes
	TMap<FGameplayTag, TArray<FOnAttributeDynamicChanged>> AttributeDynamicListeners;
	
#pragma endregion 
#pragma region Actions
	// Actions granted to this character. Populated via GrantAction() during init.
	UPROPERTY()
	TArray<TObjectPtr<UWanderersAction>> Actions;

	// Actions available by default on this character. Assigned in Blueprint CDO.
	// Processed in BeginPlay → GrantAction.
	UPROPERTY(EditAnywhere, Category = "ActionSystem")
	TArray<TSubclassOf<UWanderersAction>> DefaultActions;
#pragma endregion 
#pragma region Tags
	UPROPERTY(EditAnywhere, Category = "ActionSystem|Debug")
	TMap<FGameplayTag, FWanderersTagState> ActiveTags;
#pragma endregion 
	
private:
	// Applies all duration-based tag decrements and removes expired tags.
	// Called from OnTurnStart().
	void TickDurationTags();

	// Applies all action cooldown decrements.
	// Called from OnTurnStart().
	void TickActionCooldowns();
};