#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WanderersBattleBoard.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleTurnAdvanced, AWanderersCharacterBase*, ActiveUnit);

class AWanderersGameMode;
class UWanderersCharacterAction;
class UWanderersAction;
class UWanderersTurnManagerComponent;
class AWanderersCharacterBase;

// Represents a single slot on the battle board.
// Owns the local-space offset from the board actor and the unit currently occupying it.
USTRUCT(BlueprintType)
struct FWanderersSlot
{
	GENERATED_BODY()

	// Local offset from the BattleBoard actor origin.
	// Set these in the editor per slot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LocalOffset = FVector::ZeroVector;

	// Unit currently occupying this slot. Null if unoccupied.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<AWanderersCharacterBase> OccupyingUnit = nullptr;
};

// Holds the two slots for one side (player or enemy).
USTRUCT(BlueprintType)
struct FWanderersSide
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWanderersSlot FrontSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWanderersSlot BackSlot;
};

// AWanderersBattleBoard
//
// Placed once in the Combat level. Owns the physical layout and unit assignment
// for a single combat encounter. All slot positions are local offsets from this
// actor's transform — move the actor to reposition the entire battle layout.
//
// Responsibilities:
//   - Owns player and enemy slot data (FWanderersSide)
//   - Assigns units to slots and moves them to the correct world position
//   - Exposes SwapPositions for use by the UI/controller layer
//   - Does not own turn logic — that belongs to AWanderersTurnManager

UCLASS()
class WANDERERSOFTHEMIST_API AWanderersBattleBoard : public AActor
{
	GENERATED_BODY()

public:
	AWanderersBattleBoard();

protected:
	virtual void BeginPlay() override;

public:
	// -------------------------------------------------------------------------
	// Slot Data
	// -------------------------------------------------------------------------
#pragma region Positioning
	// Player side slots. Set LocalOffset values in the editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Board|Player")
	FWanderersSide PlayerSide;

	// Enemy side slots. Set LocalOffset values in the editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Board|Enemy")
	FWanderersSide EnemySide;

	// TODO: Consider changing this to gameplay tags
	UPROPERTY(EditAnywhere, Category = "Battle Board|Test Setup")
	TObjectPtr<AWanderersCharacterBase> PlayerFrontUnit;

	UPROPERTY(EditAnywhere, Category = "Battle Board|Test Setup")
	TObjectPtr<AWanderersCharacterBase> PlayerBackUnit;

	UPROPERTY(EditAnywhere, Category = "Battle Board|Test Setup")
	TObjectPtr<AWanderersCharacterBase> EnemyFrontUnit;

	UPROPERTY(EditAnywhere, Category = "Battle Board|Test Setup")
	TObjectPtr<AWanderersCharacterBase> EnemyBackUnit;

	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void AssignPlayerFront(AWanderersCharacterBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void AssignPlayerBack(AWanderersCharacterBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void AssignEnemyFront(AWanderersCharacterBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void AssignEnemyBack(AWanderersCharacterBase* Unit);

	// Swaps the two player units between front and back slots.
	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void SwapPlayerPositions();

	// Swaps the two enemy units between front and back slots.
	UFUNCTION(BlueprintCallable, Category = "Battle Board")
	void SwapEnemyPositions();

	UFUNCTION(BlueprintPure, Category = "Battle Board")
	AWanderersCharacterBase* GetPlayerFront() const;

	UFUNCTION(BlueprintPure, Category = "Battle Board")
	AWanderersCharacterBase* GetPlayerBack() const;

	UFUNCTION(BlueprintPure, Category = "Battle Board")
	AWanderersCharacterBase* GetEnemyFront() const;

	UFUNCTION(BlueprintPure, Category = "Battle Board")
	AWanderersCharacterBase* GetEnemyBack() const;
#pragma endregion

	// Returns all four units in turn order: PlayerFront, PlayerBack, EnemyFront, EnemyBack.
	UFUNCTION(BlueprintPure, Category = "Battle Board")
	TArray<AWanderersCharacterBase*> GetAllUnits() const;

#pragma region Combat
	UFUNCTION(BlueprintCallable, Category = "BattleBoard")
	void StartCombat();

	void ResolveAction(AWanderersCharacterBase* Caster,
		TArray<AWanderersCharacterBase*> Targets,
		UWanderersCharacterAction* Action);
	
	// Returns all units currently occupying valid target slots for the given ability,
	// filtered by which slots are marked true in ValidTargets.
	UFUNCTION(BlueprintPure, Category = "Combat")
	TArray<AWanderersCharacterBase*> GetValidTargets(
		UWanderersCharacterAction* ActionData
	) const;
#pragma endregion

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWanderersTurnManagerComponent> TurnManagerComponent;

	UPROPERTY(BlueprintAssignable, Category="Turn Management")
	FOnBattleTurnAdvanced OnPlayerTurnStarted;

	UPROPERTY(BlueprintAssignable, Category="Turn Management")
	FOnBattleTurnAdvanced OnEnemyTurnStarted;
	
	UFUNCTION()
	void HandlePlayerTurnStarted(AWanderersCharacterBase* Unit);

	UFUNCTION()
	void HandleEnemyTurnStarted(AWanderersCharacterBase* Unit);
	
	UFUNCTION()
	void HandleRoundStarted();
	
	void CompletePlayerTurn();
	
	// Swaps occupying units between two slots and updates their world positions.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SwapSlots(FWanderersSlot& SlotA, FWanderersSlot& SlotB);
	
	// Checks whether two slots can be swapped.
	// This is only true in the following situations:
	// - Both slot units are alive and do not have any effects preventing them from 
	// being swapped, such as Rooted.
	// - The front unit is dead, and the one in the back must be swapped to the front
	// NOTE the Front and Back slots order.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanSwapUnits(const FWanderersSlot& FrontA, const FWanderersSlot& BackB) const;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanSwapPlayerTeam() const;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanSwapEnemyTeam() const;
private:
	// Moves a unit actor to the world position of a given slot.
	void PlaceUnitAtSlot(AWanderersCharacterBase* Unit, const FWanderersSlot& Slot) const;
	
	void OnCharacterSlotSwap(AWanderersCharacterBase* Unit);
};
