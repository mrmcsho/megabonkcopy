#include "Characters/Phase1Character.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"

APhase1Character::APhase1Character()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraDistance;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->bUseSeparateBrakingFriction = true;
    Movement->BrakingFriction = GroundFriction;
    Movement->bMaintainHorizontalGroundVelocity = true;
    Movement->bUseFlatBaseForFloorChecks = true;
}

void APhase1Character::BeginPlay()
{
    Super::BeginPlay();

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = WalkSpeed;
    Movement->MaxAcceleration = MaxAcceleration;
    Movement->BrakingDecelerationWalking = GroundBrakingDeceleration;
    Movement->GroundFriction = GroundFriction;
    Movement->BrakingFriction = GroundFriction;
    Movement->AirControl = AirControl;
    Movement->AirControlBoostMultiplier = AirControlBoostMultiplier;
    Movement->AirControlBoostVelocityThreshold = 300.0f;
    Movement->FallingLateralFriction = FallingLateralFriction;
    Movement->JumpZVelocity = JumpVelocity;
    Movement->GravityScale = GravityScale;
    Movement->RotationRate = MovementRotationRate;
    Movement->SetWalkableFloorAngle(WalkableFloorAngle);
    Movement->MaxStepHeight = MaxStepHeight;

    CameraBoom->TargetArmLength = CameraDistance;
    CameraBoom->bEnableCameraLag = bEnableCameraLag;
    CameraBoom->CameraLagSpeed = CameraLagSpeed;
    CameraBoom->bEnableCameraRotationLag = bEnableCameraRotationLag;
    CameraBoom->CameraRotationLagSpeed = CameraRotationLagSpeed;
    FollowCamera->SetFieldOfView(DefaultFOV);

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->PlayerCameraManager->ViewPitchMin = PitchMin;
        PC->PlayerCameraManager->ViewPitchMax = PitchMax;
    }
}

void APhase1Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const bool bMovingFast = GetVelocity().SizeSquared2D() > FMath::Square(WalkSpeed * 0.75f);
    const float TargetFOV = bSprinting && bMovingFast ? SprintFOV : DefaultFOV;
    FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, FOVBlendSpeed));
}

void APhase1Character::ConfigureInputMapping()
{
    InputContext = NewObject<UInputMappingContext>(this, TEXT("Phase1InputContext"));
    auto MakeAxis = [this](const TCHAR* Name)
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Axis1D;
        return Action;
    };
    MoveForwardAction = MakeAxis(TEXT("MoveForward"));
    MoveRightAction = MakeAxis(TEXT("MoveRight"));
    LookYawAction = MakeAxis(TEXT("LookYaw"));
    LookPitchAction = MakeAxis(TEXT("LookPitch"));
    SprintAction = NewObject<UInputAction>(this, TEXT("Sprint"));
    JumpAction = NewObject<UInputAction>(this, TEXT("Jump"));

    InputContext->MapKey(MoveForwardAction, EKeys::W);
    InputContext->MapKey(MoveForwardAction, EKeys::S).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(MoveRightAction, EKeys::D);
    InputContext->MapKey(MoveRightAction, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(LookYawAction, EKeys::MouseX);
    InputContext->MapKey(LookPitchAction, EKeys::MouseY).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(SprintAction, EKeys::LeftShift);
    InputContext->MapKey(JumpAction, EKeys::SpaceBar);

    if (const APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->AddMappingContext(InputContext, 0);
        }
    }
}

void APhase1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    ConfigureInputMapping();
    UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    Input->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &APhase1Character::MoveForward);
    Input->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APhase1Character::MoveRight);
    Input->BindAction(LookYawAction, ETriggerEvent::Triggered, this, &APhase1Character::LookYaw);
    Input->BindAction(LookPitchAction, ETriggerEvent::Triggered, this, &APhase1Character::LookPitch);
    Input->BindAction(SprintAction, ETriggerEvent::Started, this, &APhase1Character::StartSprint);
    Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhase1Character::StopSprint);
    Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
    Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
}

void APhase1Character::MoveForward(const FInputActionValue& Value)
{
    if (!Controller) return;
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value.Get<float>());
}

void APhase1Character::MoveRight(const FInputActionValue& Value)
{
    if (!Controller) return;
    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value.Get<float>());
}

void APhase1Character::LookYaw(const FInputActionValue& Value) { AddControllerYawInput(Value.Get<float>() * MouseSensitivity); }
void APhase1Character::LookPitch(const FInputActionValue& Value) { AddControllerPitchInput(Value.Get<float>() * MouseSensitivity); }
void APhase1Character::StartSprint() { bSprinting = true; GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }
void APhase1Character::StopSprint() { bSprinting = false; GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }
