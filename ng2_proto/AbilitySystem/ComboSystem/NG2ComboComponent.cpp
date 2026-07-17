#include "NG2ComboComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NG2/NG2GameplayTags.h"

TAutoConsoleVariable<bool> CVarInteractionDebugDrawing(
	TEXT("game.interaction.DebugDraw"),
	false,
	TEXT("Enable interaction component debug rendering. (0 = off, 1 = enabled)"),
	ECVF_Cheat);

UNG2ComboComponent::UNG2ComboComponent()
{
#if ENABLE_DRAW_DEBUG
	PrimaryComponentTick.bCanEverTick = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
}

void UNG2ComboComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	if (CVarInteractionDebugDrawing.GetValueOnGameThread())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		UAbilitySystemComponent* ASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (!ASC) return;

		const bool bInputWindow = ASC->HasMatchingGameplayTag(NG2GameplayTags::State_Combo_InputWindow);
		const bool bAttackWindow = ASC->HasMatchingGameplayTag(NG2GameplayTags::State_Combo_AttackWindow);
		const bool bMontagePlaying = !CurrentExecutingKey.IsNone();

		DrawDebugComboState(bInputWindow, bAttackWindow, bMontagePlaying && !bInputWindow && !bAttackWindow, DeltaTime);
	}
}

void UNG2ComboComponent::BeginPlay()
{
	Super::BeginPlay();
	BuildLookupMap();
}

void UNG2ComboComponent::BuildLookupMap()
{
	ComboLookupMap.Empty();
	if (!ComboTable) return;

	TArray<FNG2ComboEntry*> Rows;
	ComboTable->GetAllRows<FNG2ComboEntry>(TEXT("BuildLookupMap"), Rows);

	for (FNG2ComboEntry* Entry : Rows)
	{
		if (!Entry) continue;
		ComboLookupMap.Add(Entry->GetComboKey(), *Entry);
	}
}

TArray<FNG2ComboToken> UNG2ComboComponent::BuildCandidateSequence(const FNG2ComboToken& Token) const
{
	TArray<FNG2ComboToken> CandidateSequence;

	// Mid-chain: extend from the currently executing entry's sequence.
	// Fresh (no montage): extend from CurrentSequence (empty when truly idle,
	// may hold history while montage is playing).
	if (!CurrentExecutingKey.IsNone())
	{
		if (const FNG2ComboEntry* CurrentEntry = ComboLookupMap.Find(CurrentExecutingKey))
		{
			CandidateSequence = CurrentEntry->Sequence;
		}
	}
	else
	{
		CandidateSequence = CurrentSequence;
	}

	CandidateSequence.Add(Token);
	return CandidateSequence;
}

const FNG2ComboEntry* UNG2ComboComponent::LookupComboEntry(const TArray<FNG2ComboToken>& Sequence) const
{
	if (Sequence.IsEmpty()) return nullptr;

	auto MakeKey = [](const TArray<FNG2ComboToken>& Seq) -> FName
	{
		FNG2ComboEntry Probe;
		Probe.Sequence = Seq;
		return Probe.GetComboKey();
	};

	// Exact match first.
	const FName ExactKey = MakeKey(Sequence);
	UE_LOG(LogTemp, Warning, TEXT("[Combo] Lookup key (exact): %s"), *ExactKey.ToString());
	if (const FNG2ComboEntry* Found = ComboLookupMap.Find(ExactKey))
	{
		return Found;
	}

	const FNG2ComboToken& LastToken = Sequence.Last();
	const bool bHadHold = LastToken.bHold;
	const bool bHadForward = LastToken.bForward;

	if (!bHadHold && !bHadForward) return nullptr;

	// The incoming token may carry modifiers (bHold, bForward) that no registered
	// entry uses in this position. Rather than hard-failing, strip modifiers in
	// order of specificity and take the first match.
	// Stripping bHold covers: player held Y into a Y Y combo that has no HY Y entry.
	// Stripping bForward covers: player holds forward unconsciously during combos that do not require it.
	// Stripping both covers: held + forward input with only a plain entry registered.
	// All three variants are checked independently — early-returning on the first
	// would make the combined case unreachable.
	struct FFallback
	{
		bool bHold;
		bool bForward;
	};
	const FFallback Fallbacks[] = {
		{false, bHadForward}, // strip hold only
		{bHadHold, false}, // strip forward only
		{false, false}, // strip both
	};

	for (const FFallback& F : Fallbacks)
	{
		TArray<FNG2ComboToken> FallbackSequence = Sequence;
		FallbackSequence.Last().bHold = F.bHold;
		FallbackSequence.Last().bForward = F.bForward;

		const FName FallbackKey = MakeKey(FallbackSequence);
		UE_LOG(LogTemp, Warning, TEXT("[Combo] Lookup key (fallback hold=%d forward=%d): %s"),
		       F.bHold, F.bForward, *FallbackKey.ToString());

		if (const FNG2ComboEntry* Found = ComboLookupMap.Find(FallbackKey))
		{
			return Found;
		}
	}

	return nullptr;
}

ENG2ComboResolution UNG2ComboComponent::ResolveComboState(
	const FNG2ComboEntry* Entry, const FNG2ComboToken& Token,
	bool bInputWindow, bool bAttackWindow, bool bMontagePlaying)
{
	// Attack window takes priority — flush path must play immediately, not buffer again.
	if (bAttackWindow)
	{
		return Entry ? ENG2ComboResolution::Play : ENG2ComboResolution::Ignored;
	}

	// Input window only — buffer for playback when attack window opens.
	if (bInputWindow)
	{
		if (Entry)
		{
			BufferedToken = Token;
			bHasBufferedToken = true;
			return ENG2ComboResolution::Buffered;
		}
		return ENG2ComboResolution::Ignored;
	}

	// Fresh — no montage active, play immediately.
	return Entry ? ENG2ComboResolution::Play : ENG2ComboResolution::Ignored;
}

// TODO: [FORWARD SAMPLING] bForward is hardcoded false.
// Sample at moment of press inside GA_AttackInput before calling GetNextComboEntry.
// GetLastInputVector() from CharacterMovementComponent, dot against GetActorForwardVector().
// Threshold: ForwardArcThreshold (default 0.707). Pass result into the token constructor.

// TODO: [HOLD DETECTION] bHold is hardcoded false.
// Option A: Separate IA_AttackHeavyHold input action, separate GA_AttackInput variant with bHold=true.
// Option B: Enhanced Input hold trigger on IA_AttackHeavy, same ability reads trigger state.
// Only applies to Heavy for now.

// TODO: [PREFIX DETECTION] Currently fires on exact match only.
// Must also check if candidate key is a valid prefix of any longer entry.
// If prefix found: do not fire, wait for next input.
// If no exact match and no prefix: clear sequence, re-evaluate token from scratch.
// Flat scan of ComboLookupMap keys is acceptable — 30 entries max, no trie needed.

// TODO: [SEQUENCE RESET TIMER]
// Started by GA_MeleeExecution in OnMontageCompleted, not by the component.
// Duration: Entry.SequenceResetOverride > 0 ? Entry.SequenceResetOverride : GlobalSequenceResetTimeout.
// On expiry: call ClearSequence(). Cancelled when a new token arrives within the window.

// TODO: [HITBOX COLLISION]
// UNG2HitboxNotifyState: enable/disable collision trace on weapon component.
// On hit: read HitDamageValues[index] from CurrentExecutingKey entry, apply GE_MeleeDamage to target ASC.
// Hitstop: separate notify, applies GE_HitStop to attacker and target ASC.

// TODO: [DELIMB]
// On hit confirmation, roll delimb chance using DelimbBoneGroup and DelimbChanceModifier from entry.
// UDelimbComponent on target owns base chance and bone group resolution.
FNG2ComboEntry UNG2ComboComponent::GetNextComboEntry(
	FNG2ComboToken Token, bool& bFound, ENG2ComboResolution& OutResolution)
{
	bFound = false;
	OutResolution = ENG2ComboResolution::Ignored;

	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

	const bool bInputWindow = ASC && ASC->HasMatchingGameplayTag(NG2GameplayTags::State_Combo_InputWindow);
	const bool bAttackWindow = ASC && ASC->HasMatchingGameplayTag(NG2GameplayTags::State_Combo_AttackWindow);
	const bool bMontagePlaying = !CurrentExecutingKey.IsNone();

	// Spam zone: montage active, no window open. Reject without touching buffer.
	if (bMontagePlaying && !bInputWindow && !bAttackWindow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Combo] Spam zone — ignored."));
		return FNG2ComboEntry{};
	}

	const TArray<FNG2ComboToken> CandidateSequence = BuildCandidateSequence(Token);
	const FNG2ComboEntry* Entry = LookupComboEntry(CandidateSequence);
	bFound = (Entry != nullptr);

	OutResolution = ResolveComboState(Entry, Token, bInputWindow, bAttackWindow, bMontagePlaying);

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Combo] No entry found after exact and fallback lookup."));
		return FNG2ComboEntry{};
	}

	return *Entry;
}

void UNG2ComboComponent::FlushBuffer()
{
	if (!bHasBufferedToken) return;

	const FNG2ComboToken Token = BufferedToken;
	bHasBufferedToken = false;
	BufferedToken = FNG2ComboToken{};

	bool bFound = false;
	ENG2ComboResolution Resolution = ENG2ComboResolution::Ignored;
	const FNG2ComboEntry Entry = GetNextComboEntry(Token, bFound, Resolution);

	UE_LOG(LogTemp, Warning, TEXT("[Combo] FlushBuffer: %s. bFound: %d, Resolution: %d"),
	       *Entry.GetComboKey().ToString(), bFound, static_cast<int32>(Resolution));
	if (bFound && Resolution == ENG2ComboResolution::Play)
	{
		OnComboEntryReady.Broadcast(Entry);
	}
}

void UNG2ComboComponent::ClearSequence()
{
	CurrentSequence.Reset();
	CurrentExecutingKey = NAME_None;
	bHasBufferedToken = false;
	BufferedToken = FNG2ComboToken{};
}

void UNG2ComboComponent::DrawDebugComboState(
	bool bInputWindow, bool bAttackWindow, bool bSpamZone, float Duration) const
{
	if (CVarInteractionDebugDrawing.GetValueOnGameThread())
	{
		AActor* Owner = GetOwner();
		if (!Owner) return;
		UWorld* World = Owner->GetWorld();
		if (!World) return;

		const FVector Center = Owner->GetActorLocation();

		if (bSpamZone)
		{
			// Red box burst at character location — spam input feedback
			DrawDebugBox(World, Center, FVector(40, 20, 90), FColor::Red, false, -1.f, 0, 2.f);
		}
		else if (bAttackWindow)
		{
			DrawDebugBox(World, Center, FVector(40, 20, 90), FColor::Green, false, -1.f, 0, 2.f);
		}
		else if (bInputWindow)
		{
			DrawDebugBox(World, Center, FVector(40, 20, 90), FColor::Yellow, false, -1.f, 0, 2.f);
		}
		else
		{
			// Attacking but no window — dim sphere to show ability is active
			DrawDebugBox(World, Center, FVector(40, 20, 90), FColor::White, false, -1.f, 0, 1.f);
		}
	}
}
