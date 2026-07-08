#pragma once

#include "CoreMinimal.h"
#include "WanderersCombatTypes.h"
#include "WanderersTokenResolver.generated.h"

class UWanderersAction;
class UWanderersEffectAsset;
class UWanderersCharacterAction;
class AWanderersGameMode;
class UWanderersActionSystemComponent;

UCLASS()
class WANDERERSOFTHEMIST_API UWanderersTokenResolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FWanderersHitContext ResolveHit(UWanderersActionSystemComponent* CasterASC,
	                                       UWanderersActionSystemComponent* TargetASC,
	                                       UWanderersCharacterAction* Action,
	                                       AWanderersGameMode* GameMode);

	static void ApplyEffects(UWanderersActionSystemComponent* ASC,
	                         UWanderersEffectAsset* Effect,
	                         AWanderersGameMode* GM);

private:
	static bool HitSuccededGate(UWanderersActionSystemComponent* CasterASC,
	                            UWanderersCharacterAction* Action,
	                            AWanderersGameMode* GameMode);

	static void DamageNullifiedGate(UWanderersActionSystemComponent* TargetAsc,
	                                UWanderersCharacterAction* Action,
	                                AWanderersGameMode* GameMode,
	                                FWanderersHitContext& HitContext);

	static void DamageModifierGate(UWanderersActionSystemComponent* CasterAsc,
	                               UWanderersActionSystemComponent* TargetAsc,
	                               AWanderersGameMode* GameMode,
	                               UWanderersCharacterAction* Action,
	                               FWanderersHitContext& HitContext);
};
