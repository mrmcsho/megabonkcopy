#include "Characters/Phase1Character.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    struct FMovementTuningSpec { const TCHAR* Name; float Minimum, Maximum, FineStep, CoarseStep, DefaultValue; };
    const FMovementTuningSpec GTuningSpecs[] =
    {
        {TEXT("[SPEED] Walk Speed"),100,3000,25,100,650}, {TEXT("[SPEED] Sprint Speed"),100,4500,25,100,2350},
        {TEXT("[GROUND] Acceleration"),100,15000,100,500,7100}, {TEXT("[GROUND] Ground Braking"),0,18000,100,500,12000},
        {TEXT("[GROUND] Ground Friction"),0,30,.25,1,9}, {TEXT("[AIR] Air Control"),0,1,.02,.1,1},
        {TEXT("[AIR] Air Control Boost"),0,5,.05,.25,1.6}, {TEXT("[AIR] Falling Friction"),0,5,.05,.25,.15},
        {TEXT("[JUMP / BHOP] Jump Velocity"),100,2000,25,100,720}, {TEXT("[AIR] Gravity Scale"),.1,8,.05,.25,2.25},
        {TEXT("[MOMENTUM] Overall Retention"),.8,1,.001,.01,.999}, {TEXT("[MOMENTUM] Ground Retention"),.8,1,.001,.01,.998},
        {TEXT("[MOMENTUM] Air Retention"),.8,1,.0005,.005,.9995}, {TEXT("[MOMENTUM] Landing Retention"),0,1,.01,.05,.96},
        {TEXT("[MOMENTUM] Soft Speed Cap"),500,8000,50,250,3200}, {TEXT("[MOMENTUM] Overspeed Drag"),0,5,.05,.25,.65},
        {TEXT("[MOMENTUM] Max Reasonable Speed"),1000,12000,100,500,5200}, {TEXT("[JUMP / BHOP] Jump Retention"),0,1,.01,.05,1},
        {TEXT("[JUMP / BHOP] Jump Buffer"),0,.5,.01,.05,.14}, {TEXT("[JUMP / BHOP] Coyote Time"),0,.5,.01,.05,.12},
        {TEXT("[JUMP / BHOP] Bunny Retention"),0,1,.01,.05,.98}, {TEXT("[GROUND] Turn Influence"),0,20,.25,1,7.5},
        {TEXT("[AIR] Turn Influence"),0,20,.25,1,2.5}, {TEXT("[SPEED] Diagonal Multiplier"),.5,1.5,.01,.05,1},
        {TEXT("[SLOPE] Acceleration"),0,4000,50,250,900}, {TEXT("[SLOPE] Downhill Multiplier"),.5,2,.01,.05,1.08},
        {TEXT("[SLOPE] Uphill Resistance"),0,2,.025,.1,.35}, {TEXT("[WALL KICK] Up"),100,2000,25,100,1000},
        {TEXT("[WALL KICK] Out"),100,2500,25,100,400}, {TEXT("[WALL KICK] Momentum Retention"),0,1,.025,.1,1},
        {TEXT("[WALL KICK] Grace Time"),0,.5,.01,.05,.12}, {TEXT("[WALL KICK] Same Wall Lockout"),0,1,.01,.05,.2},
        {TEXT("[WALL KICK] Check Distance"),0,150,2,10,28}, {TEXT("[GROUND] Turn Rate (Yaw)"),0,3000,25,100,900},
        {TEXT("[GROUND] Walkable Angle"),0,89,1,5,46}, {TEXT("[GROUND] Max Step Height"),0,150,2,10,52}
    };
}

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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    auto ConfigureDebugMesh = [this](UStaticMeshComponent* Component)
    {
        Component->SetupAttachment(RootComponent);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
    };

    DebugTorso = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugTorso"));
    ConfigureDebugMesh(DebugTorso);
    DebugTorso->SetStaticMesh(CubeMesh.Object);
    DebugTorso->SetRelativeLocation(FVector(0.0f, 0.0f, 5.0f));
    DebugTorso->SetRelativeScale3D(FVector(0.42f, 0.30f, 0.72f));

    DebugHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugHead"));
    ConfigureDebugMesh(DebugHead);
    DebugHead->SetStaticMesh(SphereMesh.Object);
    DebugHead->SetRelativeLocation(FVector(0.0f, 0.0f, 62.0f));
    DebugHead->SetRelativeScale3D(FVector(0.32f));

    DebugLeftLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugLeftLeg"));
    ConfigureDebugMesh(DebugLeftLeg);
    DebugLeftLeg->SetStaticMesh(CubeMesh.Object);
    DebugLeftLeg->SetRelativeLocation(FVector(2.0f, -15.0f, -57.0f));
    DebugLeftLeg->SetRelativeScale3D(FVector(0.22f, 0.18f, 0.42f));

    DebugRightLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugRightLeg"));
    ConfigureDebugMesh(DebugRightLeg);
    DebugRightLeg->SetStaticMesh(CubeMesh.Object);
    DebugRightLeg->SetRelativeLocation(FVector(2.0f, 15.0f, -57.0f));
    DebugRightLeg->SetRelativeScale3D(FVector(0.22f, 0.18f, 0.42f));

    DebugForwardMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugForwardMarker"));
    ConfigureDebugMesh(DebugForwardMarker);
    DebugForwardMarker->SetStaticMesh(CubeMesh.Object);
    DebugForwardMarker->SetRelativeLocation(FVector(38.0f, 0.0f, 18.0f));
    DebugForwardMarker->SetRelativeScale3D(FVector(0.34f, 0.055f, 0.055f));

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->bUseSeparateBrakingFriction = true;
    Movement->bMaintainHorizontalGroundVelocity = true;
    Movement->bUseFlatBaseForFloorChecks = true;
}

void APhase1Character::BeginPlay()
{
    Super::BeginPlay();
    ApplyMovementTuning();
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
    bWasFalling = GetCharacterMovement()->IsFalling();
}

void APhase1Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || !Controller) return;

    const float Now = GetWorld()->GetTimeSeconds();
    const bool bGrounded = Movement->IsMovingOnGround();
    if (bWasFalling && bGrounded) HandleLanding();
    if (bGrounded) { LastGroundedTime = Now; bWallKickState = false; }
    else if (Movement->IsFalling()) LastAirHorizontalVelocity = FVector(Movement->Velocity.X, Movement->Velocity.Y, 0);
    bWasFalling = Movement->IsFalling();

    UpdateWallContact();
    const FRotator Yaw(0, Controller->GetControlRotation().Yaw, 0);
    FVector Wish = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X) * ForwardInput + FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y) * RightInput;
    const bool bDiagonal = FMath::Abs(ForwardInput) > KINDA_SMALL_NUMBER && FMath::Abs(RightInput) > KINDA_SMALL_NUMBER;
    const float RawMagnitude = FMath::Min(Wish.Size(), 1.0f);
    Wish = Wish.GetSafeNormal();
    const float InputScale = RawMagnitude * (bDiagonal ? DiagonalMovementMultiplier : 1.0f);
    if (!Wish.IsNearlyZero()) AddMovementInput(Wish, InputScale);

    if (bGrounded) { UpdateGroundMovement(DeltaSeconds, Wish, InputScale); UpdateSlopeMovement(DeltaSeconds, Wish); }
    else UpdateAirMovement(DeltaSeconds, Wish, InputScale);
    UpdateMomentum(DeltaSeconds, bGrounded);

    const float HorizontalSpeed = Movement->Velocity.Size2D();
    const float InputTargetSpeed = GetNormalTargetSpeed() * (bDiagonal ? DiagonalMovementMultiplier : 1.0f);
    Movement->MaxWalkSpeed = FMath::Max(InputTargetSpeed, HorizontalSpeed + 1.0f);
    const bool bMovingFast = HorizontalSpeed > WalkSpeed * .75f;
    FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, bSprinting && bMovingFast ? SprintFOV : DefaultFOV, DeltaSeconds, FOVBlendSpeed));
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

    auto MakeButton = [this](const TCHAR* Name)
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Boolean;
        return Action;
    };

    MoveForwardAction = MakeAxis(TEXT("MoveForward"));
    MoveRightAction = MakeAxis(TEXT("MoveRight"));
    LookYawAction = MakeAxis(TEXT("LookYaw"));
    LookPitchAction = MakeAxis(TEXT("LookPitch"));
    SprintAction = MakeButton(TEXT("Sprint"));
    JumpAction = MakeButton(TEXT("Jump"));

    TuningToggleAction = MakeButton(TEXT("TuningToggle"));
    TuningPreviousAction = MakeButton(TEXT("TuningPrevious"));
    TuningNextAction = MakeButton(TEXT("TuningNext"));
    TuningDecreaseFineAction = MakeButton(TEXT("TuningDecreaseFine"));
    TuningIncreaseFineAction = MakeButton(TEXT("TuningIncreaseFine"));
    TuningDecreaseCoarseAction = MakeButton(TEXT("TuningDecreaseCoarse"));
    TuningIncreaseCoarseAction = MakeButton(TEXT("TuningIncreaseCoarse"));
    TuningResetSelectedAction = MakeButton(TEXT("TuningResetSelected"));
    TuningResetAllAction = MakeButton(TEXT("TuningResetAll"));

    InputContext->MapKey(MoveForwardAction, EKeys::W);
    InputContext->MapKey(MoveForwardAction, EKeys::S).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(MoveRightAction, EKeys::D);
    InputContext->MapKey(MoveRightAction, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(LookYawAction, EKeys::MouseX);
    InputContext->MapKey(LookPitchAction, EKeys::MouseY).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
    InputContext->MapKey(SprintAction, EKeys::LeftShift);
    InputContext->MapKey(JumpAction, EKeys::SpaceBar);

    InputContext->MapKey(TuningToggleAction, EKeys::F1);
    InputContext->MapKey(TuningPreviousAction, EKeys::Up);
    InputContext->MapKey(TuningNextAction, EKeys::Down);
    InputContext->MapKey(TuningDecreaseFineAction, EKeys::Left);
    InputContext->MapKey(TuningIncreaseFineAction, EKeys::Right);
    InputContext->MapKey(TuningDecreaseCoarseAction, EKeys::PageDown);
    InputContext->MapKey(TuningIncreaseCoarseAction, EKeys::PageUp);
    InputContext->MapKey(TuningResetSelectedAction, EKeys::R);
    InputContext->MapKey(TuningResetAllAction, EKeys::F2);

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
    Input->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &APhase1Character::MoveForward);
    Input->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APhase1Character::MoveRight);
    Input->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &APhase1Character::MoveRight);
    Input->BindAction(LookYawAction, ETriggerEvent::Triggered, this, &APhase1Character::LookYaw);
    Input->BindAction(LookPitchAction, ETriggerEvent::Triggered, this, &APhase1Character::LookPitch);
    Input->BindAction(SprintAction, ETriggerEvent::Started, this, &APhase1Character::StartSprint);
    Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhase1Character::StopSprint);
    Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APhase1Character::HandleJumpStarted);
    Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &APhase1Character::HandleJumpCompleted);

    Input->BindAction(TuningToggleAction, ETriggerEvent::Started, this, &APhase1Character::ToggleMovementTuning);
    Input->BindAction(TuningPreviousAction, ETriggerEvent::Started, this, &APhase1Character::SelectPreviousMovementTuning);
    Input->BindAction(TuningNextAction, ETriggerEvent::Started, this, &APhase1Character::SelectNextMovementTuning);
    Input->BindAction(TuningDecreaseFineAction, ETriggerEvent::Started, this, &APhase1Character::DecreaseMovementTuningFine);
    Input->BindAction(TuningIncreaseFineAction, ETriggerEvent::Started, this, &APhase1Character::IncreaseMovementTuningFine);
    Input->BindAction(TuningDecreaseCoarseAction, ETriggerEvent::Started, this, &APhase1Character::DecreaseMovementTuningCoarse);
    Input->BindAction(TuningIncreaseCoarseAction, ETriggerEvent::Started, this, &APhase1Character::IncreaseMovementTuningCoarse);
    Input->BindAction(TuningResetSelectedAction, ETriggerEvent::Started, this, &APhase1Character::ResetSelectedMovementTuning);
    Input->BindAction(TuningResetAllAction, ETriggerEvent::Started, this, &APhase1Character::ResetAllMovementTuning);
}

int32 APhase1Character::GetMovementTuningCount() const
{
    return UE_ARRAY_COUNT(GTuningSpecs);
}

FString APhase1Character::GetMovementTuningName(int32 Index) const
{
    if (!FMath::IsWithinInclusive(Index, 0, GetMovementTuningCount() - 1))
    {
        return TEXT("Invalid");
    }

    return GTuningSpecs[Index].Name;
}

float APhase1Character::GetMovementTuningValue(int32 Index) const
{
    switch (Index)
    {
    case 0: return WalkSpeed;
    case 1: return SprintSpeed;
    case 2: return MaxAcceleration;
    case 3: return GroundBrakingDeceleration;
    case 4: return GroundFriction;
    case 5: return AirControl;
    case 6: return AirControlBoostMultiplier;
    case 7: return FallingLateralFriction;
    case 8: return JumpVelocity;
    case 9: return GravityScale;
    case 10: return MomentumRetention; case 11: return GroundMomentumRetention;
    case 12: return AirMomentumRetention; case 13: return LandingMomentumRetention;
    case 14: return SoftSpeedCap; case 15: return OverspeedDrag;
    case 16: return MaxReasonableSpeed; case 17: return JumpMomentumRetention;
    case 18: return JumpBufferTime; case 19: return GroundCoyoteTime;
    case 20: return BunnyHopRetention; case 21: return GroundTurnInfluence;
    case 22: return AirTurnInfluence; case 23: return DiagonalMovementMultiplier;
    case 24: return SlopeAcceleration; case 25: return DownhillMomentumMultiplier;
    case 26: return UphillResistance; case 27: return WallJumpUpSpeed;
    case 28: return WallJumpOutSpeed; case 29: return WallMomentumRetention;
    case 30: return WallGraceTime; case 31: return SameWallLockout;
    case 32: return WallCheckExtraDistance; case 33: return MovementRotationRate.Yaw;
    case 34: return WalkableFloorAngle; case 35: return MaxStepHeight;
    default: return 0.0f;
    }
}

void APhase1Character::SetMovementTuningValue(int32 Index, float Value)
{
    if (!FMath::IsWithinInclusive(Index, 0, GetMovementTuningCount() - 1))
    {
        return;
    }

    Value = FMath::Clamp(Value, GTuningSpecs[Index].Minimum, GTuningSpecs[Index].Maximum);

    switch (Index)
    {
    case 0: WalkSpeed = Value; break;
    case 1: SprintSpeed = Value; break;
    case 2: MaxAcceleration = Value; break;
    case 3: GroundBrakingDeceleration = Value; break;
    case 4: GroundFriction = Value; break;
    case 5: AirControl = Value; break;
    case 6: AirControlBoostMultiplier = Value; break;
    case 7: FallingLateralFriction = Value; break;
    case 8: JumpVelocity = Value; break;
    case 9: GravityScale = Value; break;
    case 10: MomentumRetention = Value; break; case 11: GroundMomentumRetention = Value; break;
    case 12: AirMomentumRetention = Value; break; case 13: LandingMomentumRetention = Value; break;
    case 14: SoftSpeedCap = Value; break; case 15: OverspeedDrag = Value; break;
    case 16: MaxReasonableSpeed = Value; break; case 17: JumpMomentumRetention = Value; break;
    case 18: JumpBufferTime = Value; break; case 19: GroundCoyoteTime = Value; break;
    case 20: BunnyHopRetention = Value; break; case 21: GroundTurnInfluence = Value; break;
    case 22: AirTurnInfluence = Value; break; case 23: DiagonalMovementMultiplier = Value; break;
    case 24: SlopeAcceleration = Value; break; case 25: DownhillMomentumMultiplier = Value; break;
    case 26: UphillResistance = Value; break; case 27: WallJumpUpSpeed = Value; break;
    case 28: WallJumpOutSpeed = Value; break; case 29: WallMomentumRetention = Value; break;
    case 30: WallGraceTime = Value; break; case 31: SameWallLockout = Value; break;
    case 32: WallCheckExtraDistance = Value; break; case 33: MovementRotationRate.Yaw = Value; break;
    case 34: WalkableFloorAngle = Value; break; case 35: MaxStepHeight = Value; break;
    default: break;
    }
}

void APhase1Character::ApplyMovementTuning()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement)
    {
        return;
    }

    // MaxWalkSpeed is the normal acceleration target, not an absolute cap. Keeping it
    // above existing velocity prevents UCharacterMovement from deleting earned speed.
    Movement->MaxWalkSpeed = FMath::Max(bSprinting ? SprintSpeed : WalkSpeed, Movement->Velocity.Size2D() + 1.0f);
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
}

void APhase1Character::ToggleMovementTuning()
{
    bMovementTuningVisible = !bMovementTuningVisible;
}

void APhase1Character::SelectPreviousMovementTuning()
{
    if (!bMovementTuningVisible)
    {
        return;
    }

    MovementTuningSelection =
        (MovementTuningSelection - 1 + GetMovementTuningCount()) % GetMovementTuningCount();
}

void APhase1Character::SelectNextMovementTuning()
{
    if (!bMovementTuningVisible)
    {
        return;
    }

    MovementTuningSelection = (MovementTuningSelection + 1) % GetMovementTuningCount();
}

void APhase1Character::DecreaseMovementTuningFine()
{
    AdjustMovementTuning(-1.0f, false);
}

void APhase1Character::IncreaseMovementTuningFine()
{
    AdjustMovementTuning(1.0f, false);
}

void APhase1Character::DecreaseMovementTuningCoarse()
{
    AdjustMovementTuning(-1.0f, true);
}

void APhase1Character::IncreaseMovementTuningCoarse()
{
    AdjustMovementTuning(1.0f, true);
}

void APhase1Character::AdjustMovementTuning(float Direction, bool bCoarse)
{
    if (!bMovementTuningVisible)
    {
        return;
    }

    const FMovementTuningSpec& Spec = GTuningSpecs[MovementTuningSelection];
    const float Step = bCoarse ? Spec.CoarseStep : Spec.FineStep;
    SetMovementTuningValue(
        MovementTuningSelection,
        GetMovementTuningValue(MovementTuningSelection) + Direction * Step);

    ApplyMovementTuning();
}

void APhase1Character::ResetSelectedMovementTuning()
{
    if (!bMovementTuningVisible)
    {
        return;
    }

    SetMovementTuningValue(MovementTuningSelection, GTuningSpecs[MovementTuningSelection].DefaultValue);
    ApplyMovementTuning();
}

void APhase1Character::ResetAllMovementTuning()
{
    if (!bMovementTuningVisible)
    {
        return;
    }

    for (int32 Index = 0; Index < GetMovementTuningCount(); ++Index)
    {
        SetMovementTuningValue(Index, GTuningSpecs[Index].DefaultValue);
    }

    ApplyMovementTuning();
}

void APhase1Character::MoveForward(const FInputActionValue& Value) { ForwardInput = Value.Get<float>(); }
void APhase1Character::MoveRight(const FInputActionValue& Value) { RightInput = Value.Get<float>(); }
void APhase1Character::LookYaw(const FInputActionValue& Value) { AddControllerYawInput(Value.Get<float>() * MouseSensitivity); }
void APhase1Character::LookPitch(const FInputActionValue& Value) { AddControllerPitchInput(Value.Get<float>() * MouseSensitivity); }
void APhase1Character::StartSprint() { bSprinting = true; ApplyMovementTuning(); }
void APhase1Character::StopSprint() { bSprinting = false; ApplyMovementTuning(); }

void APhase1Character::HandleJumpStarted()
{
    JumpPressedTime = GetWorld()->GetTimeSeconds();
    if (TryWallKick()) return;
    const UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement->IsMovingOnGround() || JumpPressedTime - LastGroundedTime <= GroundCoyoteTime)
        PerformJump(false);
}

void APhase1Character::HandleJumpCompleted() { StopJumping(); }

bool APhase1Character::PerformJump(bool bBunnyHop)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement) return false;
    FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0);
    Horizontal *= bBunnyHop ? BunnyHopRetention : JumpMomentumRetention;
    Movement->Velocity = FVector(Horizontal.X, Horizontal.Y, JumpVelocity);
    Movement->SetMovementMode(MOVE_Falling);
    JumpPressedTime = -1000.0f;
    RecordMovementEvent(bBunnyHop ? TEXT("Bunny Hop") : TEXT("Jump"));
    return true;
}

void APhase1Character::HandleLanding()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement) return;
    const float Retention = LandingMomentumRetention;
    FVector LandingHorizontal = LastAirHorizontalVelocity * Retention;
    if (LandingHorizontal.Size2D() > Movement->Velocity.Size2D())
    {
        Movement->Velocity.X = LandingHorizontal.X;
        Movement->Velocity.Y = LandingHorizontal.Y;
    }
    if (GetWorld()->GetTimeSeconds() - JumpPressedTime <= JumpBufferTime) PerformJump(true);
}

void APhase1Character::UpdateGroundMovement(float DeltaSeconds, const FVector& WishDirection, float /*InputScale*/)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || WishDirection.IsNearlyZero()) return;
    FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0);
    const float Speed = Horizontal.Size();
    if (Speed < 1.0f) return;
    const FVector Steered = FMath::Lerp(Horizontal.GetSafeNormal(), WishDirection, FMath::Clamp(GroundTurnInfluence * DeltaSeconds, 0.0f, 1.0f)).GetSafeNormal();
    Movement->Velocity.X = Steered.X * Speed;
    Movement->Velocity.Y = Steered.Y * Speed;
}

void APhase1Character::UpdateAirMovement(float DeltaSeconds, const FVector& WishDirection, float /*InputScale*/)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || WishDirection.IsNearlyZero()) return;
    FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0);
    const float Speed = Horizontal.Size();
    if (Speed < 1.0f) return;
    const FVector Steered = FMath::Lerp(Horizontal.GetSafeNormal(), WishDirection, FMath::Clamp(AirTurnInfluence * DeltaSeconds, 0.0f, 1.0f)).GetSafeNormal();
    Movement->Velocity.X = Steered.X * Speed;
    Movement->Velocity.Y = Steered.Y * Speed;
}

void APhase1Character::UpdateMomentum(float DeltaSeconds, bool bGrounded)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0);
    float Speed = Horizontal.Size();
    const float Target = GetNormalTargetSpeed();
    if (Speed <= Target) return;

    const float FrameRetention = MomentumRetention * (bGrounded ? GroundMomentumRetention : AirMomentumRetention);
    Speed *= FMath::Pow(FMath::Clamp(FrameRetention, 0.0f, 1.0f), DeltaSeconds * 60.0f);
    if (Speed > SoftSpeedCap)
    {
        const float ExcessRatio = (Speed - SoftSpeedCap) / FMath::Max(SoftSpeedCap, 1.0f);
        Speed *= FMath::Exp(-OverspeedDrag * ExcessRatio * DeltaSeconds);
    }
    Speed = FMath::Min(Speed, MaxReasonableSpeed);
    const FVector Adjusted = Horizontal.GetSafeNormal() * Speed;
    Movement->Velocity.X = Adjusted.X; Movement->Velocity.Y = Adjusted.Y;
}

void APhase1Character::UpdateSlopeMovement(float DeltaSeconds, const FVector& WishDirection)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || !Movement->CurrentFloor.IsWalkableFloor()) return;
    const FVector Normal = Movement->CurrentFloor.HitResult.ImpactNormal;
    if (Normal.Z > .995f) return;
    FVector Downhill = FVector::VectorPlaneProject(FVector(0, 0, -1), Normal).GetSafeNormal();
    FVector HorizontalDownhill(Downhill.X, Downhill.Y, 0); HorizontalDownhill.Normalize();
    const float Alignment = FVector::DotProduct(Movement->Velocity.GetSafeNormal2D(), HorizontalDownhill);
    if (Alignment > .05f)
    {
        Movement->Velocity += HorizontalDownhill * SlopeAcceleration * Alignment * DeltaSeconds;
        Movement->Velocity.X *= FMath::Pow(DownhillMomentumMultiplier, DeltaSeconds);
        Movement->Velocity.Y *= FMath::Pow(DownhillMomentumMultiplier, DeltaSeconds);
        RecordMovementEvent(TEXT("Slope Boost"));
    }
    else if (Alignment < -.05f)
    {
        const float Scale = FMath::Max(0.0f, 1.0f - UphillResistance * -Alignment * DeltaSeconds);
        Movement->Velocity.X *= Scale; Movement->Velocity.Y *= Scale;
    }
}

void APhase1Character::UpdateWallContact()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || !Movement->IsFalling()) return;
    FHitResult WallHit;
    if (FindNearbyWall(WallHit))
    {
        FVector Normal(WallHit.ImpactNormal.X, WallHit.ImpactNormal.Y, 0);
        if (Normal.Normalize()) { LastWallNormal = Normal; LastWallContactTime = GetWorld()->GetTimeSeconds(); }
    }
}

bool APhase1Character::FindNearbyWall(FHitResult& OutHit) const
{
    const UWorld* World = GetWorld(); const UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (!World || !Capsule) return false;
    const FVector Start = GetActorLocation();
    const float Distance = Capsule->GetScaledCapsuleRadius() + WallCheckExtraDistance;
    static const FVector Directions[] = { FVector(1,0,0), FVector(-1,0,0), FVector(0,1,0), FVector(0,-1,0), FVector(1,1,0).GetSafeNormal(), FVector(1,-1,0).GetSafeNormal(), FVector(-1,1,0).GetSafeNormal(), FVector(-1,-1,0).GetSafeNormal() };
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WallKickTrace), false, this);
    float Best = TNumericLimits<float>::Max(); bool bFound = false;
    for (const FVector& Direction : Directions)
    {
        FHitResult Hit;
        if (!World->LineTraceSingleByChannel(Hit, Start, Start + Direction * Distance, ECC_Visibility, Params)) continue;
        if (GetCharacterMovement()->IsWalkable(Hit) || FMath::Abs(Hit.ImpactNormal.Z) > .35f) continue;
        if (Hit.Distance < Best) { Best = Hit.Distance; OutHit = Hit; bFound = true; }
    }
    return bFound;
}

bool APhase1Character::TryWallKick()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement(); UWorld* World = GetWorld();
    if (!Movement || !World || !Movement->IsFalling()) return false;
    FHitResult Hit;
    if (FindNearbyWall(Hit))
    {
        FVector Normal(Hit.ImpactNormal.X, Hit.ImpactNormal.Y, 0);
        if (Normal.Normalize()) { LastWallNormal = Normal; LastWallContactTime = World->GetTimeSeconds(); }
    }
    const float Now = World->GetTimeSeconds();
    if (Now - LastWallContactTime > WallGraceTime || LastWallNormal.IsNearlyZero()) return false;
    const bool bSameWall = !LastWallJumpNormal.IsNearlyZero() && FVector::DotProduct(LastWallNormal, LastWallJumpNormal) > .85f;
    if (bSameWall && Now - LastWallJumpTime < SameWallLockout) return false;
    FVector Horizontal(Movement->Velocity.X, Movement->Velocity.Y, 0);
    const float IntoWall = FVector::DotProduct(Horizontal, -LastWallNormal);
    if (IntoWall > 0) Horizontal += LastWallNormal * IntoWall;
    const FVector Tangential = Horizontal - LastWallNormal * FVector::DotProduct(Horizontal, LastWallNormal);
    const FVector NewHorizontal = Tangential * WallMomentumRetention + LastWallNormal * WallJumpOutSpeed;
    LaunchCharacter(FVector(NewHorizontal.X, NewHorizontal.Y, WallJumpUpSpeed), true, true);
    LastWallJumpNormal = LastWallNormal; LastWallJumpTime = Now; LastWallContactTime = -1000;
    bWallKickState = true; JumpPressedTime = -1000; RecordMovementEvent(TEXT("Wall Kick"));
    return true;
}

void APhase1Character::RecordMovementEvent(const TCHAR* EventName)
{
    RecentMovementEvent = EventName; RecentEventTime = GetWorld()->GetTimeSeconds();
}

FString APhase1Character::GetMovementStateText() const
{
    if (bWallKickState && GetWorld()->GetTimeSeconds() - LastWallJumpTime < .35f) return TEXT("Wall Kick");
    return GetCharacterMovement()->IsMovingOnGround() ? TEXT("Grounded") : TEXT("Falling");
}

FString APhase1Character::GetRecentMovementEvent() const
{
    return GetWorld()->GetTimeSeconds() - RecentEventTime <= .75f ? RecentMovementEvent : FString();
}

bool APhase1Character::HasWallContact() const
{
    return GetWorld()->GetTimeSeconds() - LastWallContactTime <= WallGraceTime;
}

float APhase1Character::GetOverspeed() const
{
    return FMath::Max(0.0f, GetVelocity().Size2D() - GetNormalTargetSpeed());
}
