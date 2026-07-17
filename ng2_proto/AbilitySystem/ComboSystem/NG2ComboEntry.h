#pragma once

#include "CoreMinimal.h"
#include "NG2ComboEntry.generated.h"

// TODO: Check ideal jump key check setup (Gameplay Tag based?)
UENUM(BlueprintType)
enum class EAttackButton : uint8
{
	Light,  // X
	Heavy,  // Y
	// Spin, // Used for level 2 + moves, ignore for now.
	Jump,   // Encodes aerial entry into a sequence. 
	JumpForward, // e.g. JX = neutral aerial light, J>X = forward aerial light
};

UENUM(BlueprintType)
enum class EDelimbBoneGroup : uint8
{
	None,
	Head,
	Legs,
	Arms,
};

USTRUCT(BlueprintType)
struct FNG2ComboToken
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EAttackButton Button = EAttackButton::Light;

	/* Sampled at moment of press only. ~±45° arc from character forward. */
	UPROPERTY(EditAnywhere)
	bool bForward = false;

	/* Sampled after press. True if button held past HoldThreshold. */
	UPROPERTY(EditAnywhere)
	bool bHold = false;
};

USTRUCT(BlueprintType)
struct FNG2ComboEntry : public FTableRowBase
{
	GENERATED_BODY()


    // --- Identity ---

    /*
     * Display name. e.g. "Piercing Dragon's Fang".
     * Intermediate moves without a canonical name use descriptive placeholders
     * e.g. "Piercing Dragon's Fang Pre-1", "Piercing Dragon's Fang Pre-2".
     */
    UPROPERTY(EditAnywhere, Category = "Identity")
    FName MoveName;

    /*
     * If true, this entry is not shown in the in-game combo list UI.
     * Use for intermediate moves that are real attacks but not named techniques.
     */
    UPROPERTY(EditAnywhere, Category = "Identity")
    bool bHideFromComboList = false;

    // --- Input Sequence ---
    /*
     * Full token sequence required to reach this move.
     * Every entry must have a valid sequence. No empty sequences.
     */
    UPROPERTY(EditAnywhere, Category = "Input")
    TArray<FNG2ComboToken> Sequence;

    /*
     * Time before this move's animation ends during which the next input is accepted (seconds).
     * -1.0 = use UComboComponent::GlobalInputWindow.
     * Must be less than RecoveryDuration if RecoveryDuration > 0. Editor will warn if violated.
     */
	// DISABLED: Adjust playback speed in the montage asset directly.
    // UPROPERTY(EditAnywhere, Category = "Input")
    // float InputWindowOverride = -1.0f;

    /*
     * Time after this move ends with no follow-up before the sequence clears (seconds).
     * -1.0 = use UComboComponent::GlobalSequenceResetTimeout.
     */
    UPROPERTY(EditAnywhere, Category = "Input")
    float SequenceResetOverride = -1.0f;

    /*
     * Hold duration to qualify the final button press as bHold=true (seconds).
     * -1.0 = use UComboComponent::GlobalHoldThreshold.
     * Set per-entry only when a specific move needs a tighter or looser hold window.
     * TODO: Review how the input mapping will handle hold. This property may not be needed if
     * all is solved on the input mapping level.
     */
    UPROPERTY(EditAnywhere, Category = "Input")
    float HoldThresholdOverride = -1.0f;

    // --- Animation ---
    /*
     * Required. All entries must have a montage.
     * Montage owns all per-frame timing via notifies:
     *   hitbox windows, i-frame windows, hitstop, SFX, input window open/close.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> Montage;

	/**
	 * When the move is looping, this is the section name to loop on.
	 * NAME_None = not a looping move
	 * Used together with LandSection.
	 */
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName LoopSection = NAME_None;

	/**
	 * When the move lands, this is the section name to play.
	 * Used together with LoopSection.
	 */
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName LandSection = NAME_None;

    // DISABLED: Adjust playback speed in the montage asset directly.
    // Changing it here is invisible in the montage editor and will cause confusion.
    // Re-enable only if runtime speed scaling becomes a requirement.
    // UPROPERTY(EditAnywhere, Category = "Animation")
    // float PlayRate = 1.0f;

    // --- Damage ---
    /*
     * Flat damage per hit window in this move. Index 0 = first hitbox enable, index 1 = second, etc.
     * Single-hit moves have one element. Multi-hit moves list each hit separately.
     * Example: a 2-hit move where the first hit does 40 and the second does 120 -> {40.0f, 120.0f}
     * Passed to GE_MeleeDamage via SetByCaller. AN_HitboxEnable carries the hit index as a parameter.
     * Assumes all actors start with 1000 health. Tune values accordingly.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TArray<float> HitDamageValues;

    // --- Hit Response ---
    /* Local space. Applied to target on confirmed hit. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Response")
    TArray<FVector> LaunchVector = {FVector::ZeroVector};

    /*
     * Seconds. Applied on hit confirmation. Default ~4 frames at 60fps.
     * Passed to AN_HitStop notify as a parameter.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Response")
    TArray<float> HitstopDuration = {0.066f};

    /* Seconds. Applied when target successfully blocks. Shorter than hitstop. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Response")
    TArray<float> BlockstopDuration = {0.033f};

    // --- Delimb ---
    /* Matches FDelimbEntry.BoneGroupName on UDelimbComponent. NAME_None = no delimb roll. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delimb")
    EDelimbBoneGroup DelimbBoneGroup = EDelimbBoneGroup::None;

    /* Multiplier on the base delimb chance defined in UDelimbComponent. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delimb")
    float DelimbChanceModifier = 1.0f;

    // --- Recovery ---
    /*
     * Additional lockout AFTER the montage ends before the next GA can activate (seconds).
     * Usually 0 - montage length handles recovery implicitly via animation timing.
     * Use only for moves that end abruptly and need an explicit post-animation lockout window.
     * If set > 0, InputWindowOverride must be less than this value. Editor will warn if violated.
     */
    UPROPERTY(EditAnywhere, Category = "Recovery")
    float RecoveryDuration = 0.0f;

	/**
	 * Creates a unique key for this combo entry based on its sequence of attack buttons.
	 * Uses XBox controller notation.
	 * An input is made up from input combinations, and each combination in the key
	 * is separated by a space.
	 * X = Light, Y = Heavy, J = Jump, > = Forward motion
	 * Additions to an input are normally performed at the same time as the button press,
	 * and are attached to each combination. For example, if a button requires holding,
	 * the combination will be an H suffix before the button.
	 * In practice, not all combinations are necessarily used.
	 * @return Unique combo key
	 * @example 
	 * "X HY Y" = Light attack, hold heavy attack, heavy attack
	 * "X X >X X" = Light attack, light attack, forward light attack, Light attack
	 * "Y >Y X" = Heavy attack, hold heavy attack, light attack
	 * "X >HY X" = Light attack, hold heavy attack, light attack
	 */
	FName GetComboKey() const;
};
