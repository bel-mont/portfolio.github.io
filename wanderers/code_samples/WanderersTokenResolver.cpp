#include "WanderersTokenResolver.h"

#include "WanderersOfTheMist/WanderersGameplayTags.h"
#include "WanderersOfTheMist/AbilitySystem/WanderersActionCustomEffect.h"
#include "WanderersOfTheMist/AbilitySystem/WanderersActionSystemComponent.h"
#include "WanderersOfTheMist/AbilitySystem/WanderersEffectAsset.h"
#include "WanderersOfTheMist/Characters/WanderersCharacterAction.h"
#include "WanderersOfTheMist/Core/WanderersGameMode.h"

class UWanderersCharacterAction;

FWanderersHitContext UWanderersTokenResolver::ResolveHit(UWanderersActionSystemComponent* CasterASC,
                                                         UWanderersActionSystemComponent* TargetASC,
                                                         UWanderersCharacterAction* Action,
                                                         AWanderersGameMode* GameMode)
{
	FWanderersHitContext HitContext;
	HitContext.bHitLanded = true;
	HitContext.bDamageApplied = true;
	HitContext.bPositionChangeLocked = false;
	HitContext.FinalDamage = 0;
	HitContext.InitialTargetTokenStates = TargetASC->GetActiveTagsCopy();

	if (!HitSuccededGate(CasterASC, Action, GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("Action missed. Should remove Miss token from caster."));
		HitContext.bHitLanded = false;
		return HitContext;
	}

	// If we hit the target

	// Calculate the damage modifiers
	DamageModifierGate(CasterASC, TargetASC, GameMode, Action, HitContext);

	// Apply damage a damage nullifier. It CAN nullify the entire damage.
	DamageNullifiedGate(TargetASC, Action, GameMode, HitContext);

	// Apply the damage before any effects are fired.
	if (HitContext.FinalDamage != 0)
	{
		TargetASC->ApplyAttributeChange(WanderersGameplayTags::Attribute_Health, HitContext.FinalDamage,
		                                EAttributeModifyType::Base);
	}

	// And finally, we apply target tag changes.
	ApplyEffects(TargetASC, Action->GetTargetEffect(), GameMode);

	// Run the action's special effects.
	UWanderersActionCustomEffect* CustomEffect = Action->GetPostTokenResolutionCustomEffect();
	if (CustomEffect)
	{
		// The effects here can do anything like add more damage or remove/assign tokens.
		CustomEffect->Execute(CasterASC->GetOwner(), CasterASC, TargetASC->GetOwner(), TargetASC, HitContext);
	}

	// Returned just for the caller to check things like miss cases.
	return HitContext;
}

void UWanderersTokenResolver::ApplyEffects(UWanderersActionSystemComponent* ASC,
                                           UWanderersEffectAsset* Effect,
                                           AWanderersGameMode* GM)
{
	if (!Effect) return;
	// Non damage related tags are added and removed according to what the action
	// effects on the target are.
	// No need to check, just iterate and add by count or turn duration.
	// However, an important decision is to decide what happens when we have a token of 1 type,
	// and we add the same type but with a different duration type.
	// For now, we will simply override them completely.

	int32 MaxTokenCountPerType = GM->GetMaxTokenCountPerType();
	for (TTuple<FGameplayTag, FWanderersTagState>& Pair : Effect->RemoveTags)
	{
		if (Pair.Value.Count > 0)
		{
			// Removals are straightforward and require no special checks
			ASC->ConsumeTag(Pair.Key, Pair.Value);
			// OutTokenStates.Add(Pair.Key, {Pair.Value.Count * -1, Pair.Value.bIsDurationBased});
		}
	}
	for (TTuple<FGameplayTag, FWanderersTagState>& Pair : Effect->GrantTags)
	{
		// Count tokens can add up to 3 tokens max (configurable from the game mode)
		// Duration tokens CANNOT be stacked. You cannot add a duration token top of another one
		// to prolong it. Your only choice for a stronger "duration" based effect
		// is to use a skill whose token assignment IS turn based and has more turn applications.
		int32 Count = FMath::Min(Pair.Value.Count, MaxTokenCountPerType);
		ASC->AddTag(Pair.Key, {Count, Pair.Value.bIsDurationBased});
		// OutTokenStates.Add(Pair.Key, {Count, Pair.Value.bIsDurationBased});
	}

	// For now, just updating attributes as is is enough.
	for (TTuple<FGameplayTag, FAttributeModifier> Pair : Effect->AttributeModifiers)
	{
		// The only exception is that attribute changes that cause damage
		// are not allowed here. Anything else is ok, including health changes to HEAL.
		if (Pair.Key != WanderersGameplayTags::Attribute_Health)
		{
			ASC->ApplyAttributeChange(Pair.Key, Pair.Value.Amount, Pair.Value.ModifyType);
		}
		else
		{
			// Health recovery can be applied by having a positive health attribute amount
			if (Pair.Value.Amount > 0)
			{
				ASC->ApplyAttributeChange(Pair.Key, Pair.Value.Amount, Pair.Value.ModifyType);
			}
		}
	}
}

bool UWanderersTokenResolver::HitSuccededGate(UWanderersActionSystemComponent* CasterASC,
                                              UWanderersCharacterAction* Action,
                                              AWanderersGameMode* GameMode)
{
	// TODO: Update once we have more miss tokens.
	if (Action->GetIsMissable())
	{
		// For now we check all available tokens, but we may cahnge our approach in the future
		// We only consume a single token when checking this, so when a token is found
		// that has miss chance, we MUST return true or false.
		for (TTuple<FGameplayTag, FWanderersTagState> Pair : CasterASC->GetActiveTags())
		{
			const FTokenDefinition* Token = GameMode->GetTokenDefinition(Pair.Key);
			// We skip tokens with no Miss chance setting, or a weird setup like a negative number. 
			if (!Token || Token->MissChance <= 0.0f) continue;
			if (FMath::IsNearlyEqual(Token->MissChance, 1.0))
			{
				CasterASC->ConsumeTag(Pair.Key, {1, false});
				return false;
			}
			const float MissChance = FMath::FRand();
			if (MissChance < Token->MissChance)
			{
				CasterASC->ConsumeTag(Pair.Key, {1, false});
				return false;
			}
			break;
		}
	}
	return true;
}

void UWanderersTokenResolver::DamageNullifiedGate(UWanderersActionSystemComponent* TargetAsc,
                                                  UWanderersCharacterAction* Action,
                                                  AWanderersGameMode* GameMode,
                                                  FWanderersHitContext& HitContext)
{
	if (!Action->GetDamageCanBeNegated()) return;

	// For now we check all available tokens, but we may cahnge our approach in the future
	// We only consume a single token when checking this, so when a token is found
	// that has miss chance, we MUST return true or false.
	for (TTuple<FGameplayTag, FWanderersTagState> Pair : TargetAsc->GetActiveTags())
	{
		const FTokenDefinition* Token = GameMode->GetTokenDefinition(Pair.Key);
		if (!Token || Token->DamageNullifyPercentage <= 0.0f) continue;
		HitContext.FinalDamage = FMath::FloorToInt(HitContext.FinalDamage * (1.0f - Token->DamageNullifyPercentage));
		break;
	}
}

void UWanderersTokenResolver::DamageModifierGate(UWanderersActionSystemComponent* CasterAsc,
                                                 UWanderersActionSystemComponent* TargetAsc,
                                                 AWanderersGameMode* GameMode,
                                                 UWanderersCharacterAction* Action,
                                                 FWanderersHitContext& HitContext)
{
	// We cannot change anything if there is no effect.
	UWanderersEffectAsset* TargetEffect = Action->GetTargetEffect();
	if (!TargetEffect) return;

	// If damage was not negated, continue
	FAttributeModifier* Modifier = TargetEffect->AttributeModifiers.Find(
		WanderersGameplayTags::Attribute_Health);
	if (Modifier)
	{
		// TODO: check if different modifiers should cause different processing.
		HitContext.FinalDamage = Modifier->Amount;
	}

	const TMap<FGameplayTag, FWanderersTagState>& TagPair = CasterAsc->GetActiveTags();
	TMap<FGameplayTag, FWanderersTagState> CasterTagsToConsume;
	for (TTuple<FGameplayTag, FWanderersTagState> CasterPair : TagPair)
	{
		const FTokenDefinition* Token = GameMode->GetTokenDefinition(CasterPair.Key);
		if (Token && Token->OutgoingDamageModifier != 0)
		{
			// Both the modifier and the base damage are negative, so we add them together
			HitContext.FinalDamage += Token->OutgoingDamageModifier;
			CasterTagsToConsume.Add(CasterPair.Key, {1, false});
		}
	}
	for (TTuple<FGameplayTag, FWanderersTagState>& ToConsume : CasterTagsToConsume)
	{
		CasterAsc->ConsumeTag(ToConsume.Key, ToConsume.Value);
	}

	const TMap<FGameplayTag, FWanderersTagState>& TargetTagPair = TargetAsc->GetActiveTags();
	TMap<FGameplayTag, FWanderersTagState> TargetTagsToConsume;
	for (TTuple<FGameplayTag, FWanderersTagState> TargetPair : TargetTagPair)
	{
		const FTokenDefinition* Token = GameMode->GetTokenDefinition(TargetPair.Key);
		if (Token && Token->IncomingDamageModifier != 0)
		{
			int32 NewValue = HitContext.FinalDamage + Token->IncomingDamageModifier;
			HitContext.FinalDamage = FMath::Min(NewValue, 0);
			TargetTagsToConsume.Add(TargetPair.Key, {1, false});
		}
	}
	for (TTuple<FGameplayTag, FWanderersTagState>& ToConsume : TargetTagsToConsume)
	{
		TargetAsc->ConsumeTag(ToConsume.Key, ToConsume.Value);
	}
}
