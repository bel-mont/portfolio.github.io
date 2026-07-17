#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "NG2ComboEntry.h"
#include "Components/ActorComponent.h"
#include "NG2ComboComponent.generated.h"

class UGameplayEffect;

/** Fired when the component has resolved an entry the ability should play next. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnComboEntryReady, const FNG2ComboEntry& /*Entry*/);

UENUM(BlueprintType)
enum class ENG2ComboResolution : uint8
{
	None,
	Play,
	Ignored,
	Buffered,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NG2_API UNG2ComboComponent : public UActorComponent
{
	GENERATED_BODY()
	
	UNG2ComboComponent();

protected:
	void BuildLookupMap();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	/**
	 * Tracks the running sequence of tokens in the current combo chain.
	 * Cleared on interrupt, cancel, or sequence reset timer expiry.
	 */
	TArray<FNG2ComboToken> CurrentSequence;

	void DrawDebugComboState(bool bInputWindow, bool bAttackWindow, bool bSpamZone, float Duration) const;
	
	/**
	 * Builds the token sequence to look up, rooted at the correct point in the chain.
	 * Mid-chain roots from the currently executing entry; fresh roots from CurrentSequence,
	 * which may still hold history while a reset timer ticks.
	 */
	TArray<FNG2ComboToken> BuildCandidateSequence(const FNG2ComboToken& Token) const;

	/**
	 * Looks up a sequence in the combo map. Attempts an exact match first, then retries
	 * with modifiers stripped from the last token (hold, forward, both) so that moves
	 * with no prefixed variant gracefully resolve to their plain counterpart.
	 *
	 * Does not mutate component state.
	 */
	const FNG2ComboEntry* LookupComboEntry(const TArray<FNG2ComboToken>& Sequence) const;

	/**
	 * Determines what the caller should do with a resolved entry given the current
	 * window state. Buffers the token as a side effect when resolution is Buffered —
	 * caller does not need to handle that case separately.
	 */
	ENG2ComboResolution ResolveComboState(const FNG2ComboEntry* Entry, const FNG2ComboToken& Token,
										  bool bInputWindow, bool bAttackWindow, bool bMontagePlaying);
public:
	// ── Gameplay Effect classes ───────────────────────────────────────────────
	// Assigned in the Blueprint child of this component (or on the owning actor).
	// Applied/removed by notify states: State.Combo.AttackWindow, State.Combo.InputWindow.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TSubclassOf<UGameplayEffect> InputWindowEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Combo)
	TSubclassOf<UGameplayEffect> AttackWindowEffectClass;

	FActiveGameplayEffectHandle InputWindowEffectHandle;
	FActiveGameplayEffectHandle AttackWindowEffectHandle;

	virtual void BeginPlay() override;

	// ── Data ─────────────────────────────────────────────────────────────────

	/** DataTable of FNG2ComboEntry rows. Assign in editor. */
	UPROPERTY(EditDefaultsOnly, Category=Combo)
	TObjectPtr<UDataTable> ComboTable;

	/** Runtime lookup map built from ComboTable at BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Combo)
	TMap<FName, FNG2ComboEntry> ComboLookupMap;

	/**
	 * Key of the entry whose montage is currently playing.
	 * NAME_None when no montage is active. Written by GA_MeleeExecution.
	 */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category=Combo)
	FName CurrentExecutingKey = NAME_None;

	// ── Buffer ────────────────────────────────────────────────────────────────

	UPROPERTY(Transient)
	FNG2ComboToken BufferedToken;

	UPROPERTY(Transient)
	bool bHasBufferedToken = false;

	// ── Delegate ──────────────────────────────────────────────────────────────

	/**
	 * Broadcast by FlushBuffer when the attack window opens and a valid buffered
	 * token resolves to Play. GA_MeleeExecution binds to this in OnAvatarSet.
	 */
	FOnComboEntryReady OnComboEntryReady;

	// ── Core API ──────────────────────────────────────────────────────────────

	/**
	 * Resolve a token into a combo entry.
	 *
	 * Window/state matrix:
	 *   InputWindow tag active, montage playing  → buffer the token (if valid match).
	 *                                              Resolution = Buffered.
	 *   AttackWindow tag active                  → play immediately (if valid match).
	 *                                              Resolution = Play or Ignored.
	 *   No window, no current key (fresh)        → play immediately (if valid match).
	 *                                              Resolution = Play or Ignored.
	 *   No window, has current key (spam zone)   → reject outright.
	 *                                              Resolution = Ignored.
	 *
	 * @param Token         Input token to resolve.
	 * @param bFound        True iff the candidate sequence matched a map entry.
	 * @param OutResolution What the caller should do next.
	 * @return              Matching entry when bFound, otherwise default-constructed.
	 */
	UFUNCTION(BlueprintCallable)
	FNG2ComboEntry GetNextComboEntry(FNG2ComboToken Token, bool& bFound, ENG2ComboResolution& OutResolution);

	/**
	 * Drain the buffered token through the resolver.
	 * Called by UNG2AttackWindowNotifyState::NotifyBegin.
	 * Broadcasts OnComboEntryReady if the token resolves to Play.
	 */
	UFUNCTION(BlueprintCallable)
	void FlushBuffer();

	/**
	 * Reset all sequence state. Called by GA_MeleeExecution on interrupt,
	 * cancel, or sequence reset timer expiry.
	 */
	UFUNCTION(BlueprintCallable)
	void ClearSequence();

	/** Write the key of the currently executing entry. Pass NAME_None to clear. */
	UFUNCTION(BlueprintCallable)
	void SetCurrentExecutingKey(FName Key) { CurrentExecutingKey = Key; }
};