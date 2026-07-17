#include "NG2RyuuCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "NG2CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NG2/NG2GameplayTags.h"
#include "NG2/AbilitySystem/NG2AbilitySystemComponent.h"
#include "NG2/AbilitySystem/NG2AttributeSet.h"
#include "NG2/AbilitySystem/ComboSystem/NG2ComboComponent.h"
#include "NG2/Input/NG2InputComponent.h"


ANG2RyuuCharacter::ANG2RyuuCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNG2CharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetOffset = FVector(0.f,0.f, 60.f);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 8.f;
	SpringArm->CameraLagSpeed = 10.f;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraCompComponent"));
	CameraComp->SetupAttachment(SpringArm);
	CameraComp->FieldOfView = 75.f;

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponComponent"));
	Weapon->SetupAttachment(RootComponent);

	ComboComponent = CreateDefaultSubobject<UNG2ComboComponent>(TEXT("ComboComponent"));

	AbilitySystemComponent = CreateDefaultSubobject<UNG2AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UNG2AttributeSet>(TEXT("AttributeSet"));

	// The character must move independently of the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

UAbilitySystemComponent* ANG2RyuuCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UNG2CharacterMovementComponent* ANG2RyuuCharacter::GetNG2MovementComponent() const
{
	return Cast<UNG2CharacterMovementComponent>(GetCharacterMovement());
}

void ANG2RyuuCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANG2RyuuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UNG2InputComponent* EnhancedInput = Cast<UNG2InputComponent>(PlayerInputComponent);
	EnhancedInput->BindAction(Input_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
	EnhancedInput->BindAction(Input_Look, ETriggerEvent::Triggered, this, &ThisClass::Look);
	EnhancedInput->BindAbilityActions(InputConfig, this, &ThisClass::AbilityCallback);
	AbilitySystemComponent->AddAbilities(ActiveAbilities);
}

void ANG2RyuuCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ANG2RyuuCharacter::AbilityCallback(FGameplayTag InputTag)
{
	AbilitySystemComponent->AbilityInputTagTriggered(InputTag);
}

void ANG2RyuuCharacter::Move(const FInputActionValue& Input)
{
	// We only check for landing, but allow the player to move slightly while in the air through
	// the character movement air control.
	// TODO: Block movement for now, but in between attacks we will allow
	// the player change directions. Need to check how NG2 works when running attack animations
	// and changing directions
	const FGameplayTagContainer BlockedTags = FGameplayTagContainer::CreateFromArray(TArray<FGameplayTag>{
		NG2GameplayTags::State_MovementBlocked, NG2GameplayTags::State_Attacking
	});
	bool bIsFalling = GetCharacterMovement()->IsFalling();
	if (AbilitySystemComponent->HasAnyMatchingGameplayTags(BlockedTags) || bIsFalling) return;

	const FVector2D InputValue = Input.Get<FVector2D>();
	if (InputValue.IsNearlyZero()) return;
	// Get camera-relative directions (flatten to XY plane)
	// The controller rotation is parented to world coordinates.
	// The spring arm component copies the controller rotation.
	const FRotator ControlRot = GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
	// Camera-relative forward and right axes, flattened to XY plane.
	// RotateVector applies sin/cos of the yaw angle to produce a unit vector (length=1) direction.
	// eg: if camera yaw is 90deg (north), ForwardDir=(0,1,0), RightDir=(1,0,0)
	// Example 2:
	// ForwardDir = (cos45, sin45, 0) = (0.707, 0.707, 0)
	// RightDir = (0.707, -0.707, 0) — 90° clockwise from forward
	const FVector ForwardDir = YawRot.RotateVector(FVector::ForwardVector);
	// Right direction can just be obtained by rotating our vector to the right.
	const FVector RightDir = YawRot.RotateVector(FVector::RightVector);

	// Scale each axis by input (-1 to 1) and sum them to get a world-space move direction.
	// eg: stick up = ForwardDir*1 + RightDir*0 = (0,1,0). Normalize ensures diagonal input isn't faster.
	FVector MoveDir = ForwardDir * InputValue.Y + RightDir * InputValue.X;
	MoveDir.Normalize();

	AddMovementInput(MoveDir);

	// Rotate character to face movement direction
	const FRotator TargetRot = MoveDir.Rotation();
	const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 10.f);
	SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));
}

void ANG2RyuuCharacter::Look(const FInputActionValue& Input)
{
	const FVector2D InputValue = Input.Get<FVector2D>();
	AddControllerPitchInput(InputValue.Y * LookPitchMultiplier);
	AddControllerYawInput(InputValue.X * LookYawMultiplier);
}

void ANG2RyuuCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// Remove State.Jumping directly, no effect needed. The Jumping Effect in the blueprint
	// Is also adding movement block, therefore we are better off not removing the effect entirely in a single go.
	// We leave the movement block removal to other code.
	const int32 JumpingTagCount = AbilitySystemComponent->GetGameplayTagCount(
		NG2GameplayTags::State_Jumping);
	if (JumpingTagCount > 0)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(
			NG2GameplayTags::State_Jumping, JumpingTagCount);
	}
	// TODO: [AERIAL LANDING VERTICALITY]
	// When a montage ends mid-air, the character falls to the landing state normally.
	// This does not account for level geometry differences — an attack that moves the
	// character downward over stairs or a slope will land at an unpredictable frame.
	// Future fix: compound animations (loop + finisher triggered by landing detection)
	// or animation speed scaling based on remaining fall distance at montage end.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		NG2GameplayTags::Event_Landed,
		FGameplayEventData{});
}
