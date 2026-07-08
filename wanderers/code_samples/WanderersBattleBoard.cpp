#include "WanderersBattleBoard.h"

#include "WanderersTokenResolver.h"
#include "WanderersTurnManagerComponent.h"
#include "WanderersOfTheMist/WanderersOfTheMist.h"
#include "WanderersOfTheMist/AbilitySystem/WanderersActionCustomEffect.h"
#include "WanderersOfTheMist/AbilitySystem/WanderersActionSystemComponent.h"
#include "WanderersOfTheMist/Characters/WanderersCharacterAction.h"
#include "WanderersOfTheMist/Characters/WanderersCharacterBase.h"
#include "WanderersOfTheMist/Core/WanderersGameMode.h"

AWanderersBattleBoard::AWanderersBattleBoard()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default local offsets — adjust in the editor to match your scene layout.
	// Layout reference:
	//   [ PLAYER FRONT ] [ PLAYER BACK ] | [ ENEMY BACK ] [ ENEMY FRONT ]
	PlayerSide.FrontSlot.LocalOffset = FVector(-200.f, 0.f, 0.f);
	PlayerSide.BackSlot.LocalOffset = FVector(-500.f, 0.f, 0.f);
	EnemySide.FrontSlot.LocalOffset = FVector(200.f, 0.f, 0.f);
	EnemySide.BackSlot.LocalOffset = FVector(500.f, 0.f, 0.f);

	TurnManagerComponent = CreateDefaultSubobject<UWanderersTurnManagerComponent>(TEXT("TurnManagerComponent"));
}

void AWanderersBattleBoard::BeginPlay()
{
	Super::BeginPlay();
	// Be careful of not having this in the constructor
	// If you do that, this ends up pointing to the Class Default Object, instead
	// of the instance.
	TurnManagerComponent->BattleBoard = this;

	// TODO: Need to change how units are assigned later once we have
	// proper battle board initialization.
	if (PlayerFrontUnit) {
		AssignPlayerFront(PlayerFrontUnit);
		PlayerFrontUnit->OnSwapPositions.AddUObject(this, &ThisClass::OnCharacterSlotSwap);
	}
	if (PlayerBackUnit) {
		AssignPlayerBack(PlayerBackUnit);
		PlayerBackUnit->OnSwapPositions.AddUObject(this, &ThisClass::OnCharacterSlotSwap);
	}
	if (EnemyFrontUnit) {
		AssignEnemyFront(EnemyFrontUnit);
		EnemyFrontUnit->OnSwapPositions.AddUObject(this, &ThisClass::OnCharacterSlotSwap);
	}
	if (EnemyBackUnit) {
		AssignEnemyBack(EnemyBackUnit);
		EnemyBackUnit->OnSwapPositions.AddUObject(this, &ThisClass::OnCharacterSlotSwap);
	}

	TurnManagerComponent->OnPlayerTurnStarted.AddDynamic(this, &AWanderersBattleBoard::HandlePlayerTurnStarted);
	TurnManagerComponent->OnEnemyTurnStarted.AddDynamic(this, &AWanderersBattleBoard::HandleEnemyTurnStarted);
	TurnManagerComponent->OnRoundStarted.AddDynamic(this, &AWanderersBattleBoard::HandleRoundStarted);
}

// -----------------------------------------------------------------------------
// Assignment
// -----------------------------------------------------------------------------
void AWanderersBattleBoard::AssignPlayerFront(AWanderersCharacterBase* Unit)
{
	if (!Unit) return;
	Unit->OwningBattleBoard = this;
	PlayerSide.FrontSlot.OccupyingUnit = Unit;
	PlaceUnitAtSlot(Unit, PlayerSide.FrontSlot);
}

void AWanderersBattleBoard::AssignPlayerBack(AWanderersCharacterBase* Unit)
{
	if (!Unit) return;
	Unit->OwningBattleBoard = this;
	PlayerSide.BackSlot.OccupyingUnit = Unit;
	PlaceUnitAtSlot(Unit, PlayerSide.BackSlot);
}

void AWanderersBattleBoard::AssignEnemyFront(AWanderersCharacterBase* Unit)
{
	if (!Unit) return;
	Unit->OwningBattleBoard = this;
	EnemySide.FrontSlot.OccupyingUnit = Unit;
	PlaceUnitAtSlot(Unit, EnemySide.FrontSlot);
}

void AWanderersBattleBoard::AssignEnemyBack(AWanderersCharacterBase* Unit)
{
	if (!Unit) return;
	Unit->OwningBattleBoard = this;
	EnemySide.BackSlot.OccupyingUnit = Unit;
	PlaceUnitAtSlot(Unit, EnemySide.BackSlot);
}

// -----------------------------------------------------------------------------
// Swap
// -----------------------------------------------------------------------------

void AWanderersBattleBoard::SwapPlayerPositions()
{
	if (!PlayerSide.FrontSlot.OccupyingUnit || !PlayerSide.BackSlot.OccupyingUnit) return;
	SwapSlots(PlayerSide.FrontSlot, PlayerSide.BackSlot);
}

void AWanderersBattleBoard::SwapEnemyPositions()
{
	if (!EnemySide.FrontSlot.OccupyingUnit || !EnemySide.BackSlot.OccupyingUnit) return;
	SwapSlots(EnemySide.FrontSlot, EnemySide.BackSlot);
}

// -----------------------------------------------------------------------------
// Queries
// -----------------------------------------------------------------------------

AWanderersCharacterBase* AWanderersBattleBoard::GetPlayerFront() const
{
	UE_LOG(LogGame, Warning, TEXT("GetPlayerFront returning: %s"), 
		PlayerSide.FrontSlot.OccupyingUnit ? *PlayerSide.FrontSlot.OccupyingUnit->GetName() : TEXT("NULL"));
	return PlayerSide.FrontSlot.OccupyingUnit;
}

AWanderersCharacterBase* AWanderersBattleBoard::GetPlayerBack() const
{
	return PlayerSide.BackSlot.OccupyingUnit;
}

AWanderersCharacterBase* AWanderersBattleBoard::GetEnemyFront() const
{
	return EnemySide.FrontSlot.OccupyingUnit;
}

AWanderersCharacterBase* AWanderersBattleBoard::GetEnemyBack() const
{
	return EnemySide.BackSlot.OccupyingUnit;
}

TArray<AWanderersCharacterBase*> AWanderersBattleBoard::GetAllUnits() const
{
	TArray<AWanderersCharacterBase*> Units;

	if (PlayerSide.FrontSlot.OccupyingUnit) Units.Add(PlayerSide.FrontSlot.OccupyingUnit);
	if (PlayerSide.BackSlot.OccupyingUnit) Units.Add(PlayerSide.BackSlot.OccupyingUnit);
	if (EnemySide.FrontSlot.OccupyingUnit) Units.Add(EnemySide.FrontSlot.OccupyingUnit);
	if (EnemySide.BackSlot.OccupyingUnit) Units.Add(EnemySide.BackSlot.OccupyingUnit);

	return Units;
}

void AWanderersBattleBoard::StartCombat()
{
	TurnManagerComponent->StartCombat(GetAllUnits());
}

void AWanderersBattleBoard::ResolveAction(AWanderersCharacterBase* Caster, TArray<AWanderersCharacterBase*> Targets,
	UWanderersCharacterAction* Action)
{
	if (!Caster || !Targets.Num() || !Action) return;
	UWanderersActionSystemComponent* ASC = Caster->GetActionSystemComponent();
	if (!ASC) return;
	AWanderersGameMode* GM = Cast<AWanderersGameMode>(GetWorld()->GetAuthGameMode());
	if	(!GM) return;

	// TODO: Consider removing target validation / sorting out if the UI layer can filter that ahead of time
	TArray<AWanderersCharacterBase*> ValidTargets = GetValidTargets(Action);
	TArray<AWanderersCharacterBase*> ToHit = Action->IsHitsAllValidTargets() 
		? ValidTargets 
		: Targets;
	FWanderersHitContext FinalHitContext = FWanderersHitContext();
	int32 AccumulatedDamage = 0;
	UE_LOG(LogGame, Warning, TEXT("Action %s Targeting %d targets, comparing with %d valid targets"), *Action->GetName(), Targets.Num(), ValidTargets.Num());
	for (AWanderersCharacterBase* Target : ToHit)
	{
		if (ValidTargets.Contains(Target))
		{
			UE_LOG(LogTemp, Warning, TEXT("Ability hits target"));
			FWanderersHitContext Hit = UWanderersTokenResolver::ResolveHit(Caster->GetActionSystemComponent(), 
									Target->GetActionSystemComponent(),
											Action, GM);
			AccumulatedDamage += Hit.FinalDamage;
			FinalHitContext.bHitLanded = Hit.bHitLanded;
			FinalHitContext.bDamageApplied = Hit.bDamageApplied;
			
			// The consume logic is for actions with multiple hits, but
			// when 1 attack hits all targets we will also loop through each
			// causing this to trigger more than once. It does not matter,
			// But it is something to be aware of in case we are inspecting
			// and notice that our leftover actions are a bit off.
			Action->ConsumeHit();
			if (!Hit.bHitLanded) break;
			Action->StartAction(Target);
			Target->UpdateCharacterHUD();
		}
	}
	FWanderersHitContext CasterHitContext;
	UWanderersTokenResolver::ApplyEffects(Caster->GetActionSystemComponent(), Action->GetCasterEffect(), GM);
	if (Action->GetHitsRemaining() <= 0)
	{
		Action->ApplyCooldown();
		// Post Action custom effect execute
		UWanderersActionCustomEffect* CustomEffect = Action->GetPostActionExecutionCustomEffect();
		if (CustomEffect)
		{
			FinalHitContext.FinalDamage = AccumulatedDamage;
			CustomEffect->Execute(Caster, Caster->GetActionSystemComponent(), nullptr, nullptr,
			                      FinalHitContext);
		}
	}
}

// -------------------------------------------------------------------------
// Combat Resolution
// -------------------------------------------------------------------------

TArray<AWanderersCharacterBase*> AWanderersBattleBoard::GetValidTargets(
	UWanderersCharacterAction* ActionData) const
{
	TArray<AWanderersCharacterBase*> AllUnits = GetAllUnits();
	const FGameplayTagContainer& TeamTags = ActionData->GetTeamTags();
	const FGameplayTagContainer& PositionTags = ActionData->GetPositionTags();

	TArray<AWanderersCharacterBase*> Targets;
	for (AWanderersCharacterBase* Unit : AllUnits)
	{
		bool bIsInValidPosition = Unit->GetActionSystemComponent()->HasAnyTag(PositionTags);
		bool bIsInValidTeam = Unit->GetActionSystemComponent()->HasAnyTag(TeamTags);
		if (bIsInValidPosition && bIsInValidTeam)
		{
			Targets.Add(Unit);
		}
	}

	return Targets;
}


void AWanderersBattleBoard::HandlePlayerTurnStarted(AWanderersCharacterBase* Unit)
{
	UE_LOG(LogTemp, Warning, TEXT("Battle Board: Player turn started"));
	if (OnPlayerTurnStarted.IsBound()) OnPlayerTurnStarted.Broadcast(Unit);
}

void AWanderersBattleBoard::HandleEnemyTurnStarted(AWanderersCharacterBase* Unit)
{
	UE_LOG(LogTemp, Warning, TEXT("Battle Board: Enemy turn started"));
	if (OnEnemyTurnStarted.IsBound()) OnEnemyTurnStarted.Broadcast(Unit);
}

void AWanderersBattleBoard::HandleRoundStarted()
{
	TArray<AWanderersCharacterBase*> Units = GetAllUnits();
	for (AWanderersCharacterBase* Unit : Units)
	{
		Unit->TickTurnBasedTokens();
	}
}

void AWanderersBattleBoard::CompletePlayerTurn()
{
	TurnManagerComponent->NotifyPlayerTurnCompleted();
}

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void AWanderersBattleBoard::PlaceUnitAtSlot(AWanderersCharacterBase* Unit, const FWanderersSlot& Slot) const
{
	const FVector WorldPosition = GetActorTransform().TransformPosition(Slot.LocalOffset);
	Unit->SetActorLocation(WorldPosition);
}

void AWanderersBattleBoard::OnCharacterSlotSwap(AWanderersCharacterBase* Unit)
{
	if (Unit->IsPlayerTeam())
	{
		SwapPlayerPositions();
	}
	else
	{
		SwapEnemyPositions();
	}
}

void AWanderersBattleBoard::SwapSlots(FWanderersSlot& SlotA, FWanderersSlot& SlotB)
{
	// TODO: Reimplement to work with animations too
	TObjectPtr<AWanderersCharacterBase> Temp = SlotA.OccupyingUnit;
	SlotA.OccupyingUnit = SlotB.OccupyingUnit;
	SlotB.OccupyingUnit = Temp;

	PlaceUnitAtSlot(SlotA.OccupyingUnit, SlotA);
	PlaceUnitAtSlot(SlotB.OccupyingUnit, SlotB);

	if (SlotA.OccupyingUnit)
	{
		SlotA.OccupyingUnit->SwapPositionTag();
	}
	if (SlotB.OccupyingUnit)
	{
		SlotB.OccupyingUnit->SwapPositionTag();
	}
}

bool AWanderersBattleBoard::CanSwapUnits(const FWanderersSlot& FrontA, const FWanderersSlot& BackB) const
{
	AWanderersCharacterBase* FrontCharacter = FrontA.OccupyingUnit;
	AWanderersCharacterBase* BackCharacter = BackB.OccupyingUnit;
	if (!FrontCharacter || !BackCharacter) return false;
	
	// TODO: This will cause issues once character death is added.
	// Either character is dead check
	if (FrontCharacter->bIsDead() || BackCharacter->bIsDead())
	{
		// If only the front character is dead, the units can be swapped.
		// Cannot swap otherwise.
		return FrontCharacter->bIsDead() && !BackCharacter->bIsDead();
	}
	
	// Rooted check.
	const TFunction<bool(const FTokenDefinition&)> Predicate = ([](const FTokenDefinition& Def)
	{
		return Def.bBlocksPositionChange;
	});
	const bool FrontRooted = FrontCharacter->HasTagWithPredicate(Predicate);
	const bool BackRooted = BackCharacter->HasTagWithPredicate(Predicate);
	if (FrontRooted || BackRooted) return false;
	
	return true;
}

bool AWanderersBattleBoard::CanSwapPlayerTeam() const
{
	return CanSwapUnits(PlayerSide.FrontSlot, PlayerSide.BackSlot);
}

bool AWanderersBattleBoard::CanSwapEnemyTeam() const
{
	return CanSwapUnits(EnemySide.FrontSlot, EnemySide.BackSlot);
}
