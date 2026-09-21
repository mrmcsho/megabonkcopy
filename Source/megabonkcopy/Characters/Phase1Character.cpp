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
    struct FMovementTuningSpec
    {
        const TCHAR* Name;
        float Minimum;
        float Maximum;
        float FineStep;
        float CoarseStep;
        float DefaultValue;
    };

    const FMovementTuningSpec GTuningSpecs[] =
    {
        { TEXT("Walk Speed"),              100.0f, 3000.0f, 25.0f, 100.0f, 650.0f },
        { TEXT("Sprint Speed"),            100.0f, 4000.0f, 25.0f, 100.0f, 1050.0f },
        { TEXT("Acceleration"),            100.0f, 12000.0f, 100.0f, 500.0f, 4200.0f },
        { TEXT("Ground Braking"),            0.0f, 12000.0f, 100.0f, 500.0f, 3600.0f },
        { TEXT("Ground Friction"),            0.0f, 30.0f, 0.25f, 1.0f, 9.0f },
        { TEXT("Air Control"),                0.0f, 1.0f, 0.02f, 0.10f, 0.42f },
        { TEXT("Air Control Boost"),          0.0f, 5.0f, 0.05f, 0.25f, 1.60f },
        { TEXT("Falling Lateral Friction"),   0.0f, 5.0f, 0.05f, 0.25f, 0.15f },
        { TEXT("Jump Velocity"),            100.0f, 2000.0f, 25.0f, 100.0f, 720.0f },
        { TEXT("Gravity Scale"),              0.1f, 8.0f, 0.05f, 0.25f, 2.25f },
        { TEXT("Wall Kick Up"),             100.0f, 2000.0f, 25.0f, 100.0f, 760.0f },
        { TEXT("Wall Kick Out"),            100.0f, 2500.0f, 25.0f, 100.0f, 900.0f },
        { TEXT("Wall Momentum Retention"),    0.0f, 1.0f, 0.025f, 0.10f, 0.75f },
        { TEXT("Wall Grace Time"),            0.0f, 0.5f, 0.01f, 0.05f, 0.12f },
        { TEXT("Same Wall Lockout"),          0.0f, 1.0f, 0.01f, 0.05f, 0.20f },
        { TEXT("Wall Check Extra Distance"),  0.0f, 150.0f, 2.0f, 10.0f, 28.0f },
        { TEXT("Turn Rate (Yaw)"),            0.0f, 3000.0f, 25.0f, 100.0f, 900.0f },
        { TEXT("Walkable Floor Angle"),       0.0f, 89.0f, 1.0f, 5.0f, 46.0f },
        { TEXT("Max Step Height"),            0.0f, 150.0f, 2.0f, 10.0f, 50.0f }
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
}

void APhase1Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateWallContact();

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
    Input->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &APhase1Character::MoveRight);
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
    case 10: return WallJumpUpSpeed;
    case 11: return WallJumpOutSpeed;
    case 12: return WallMomentumRetention;
    case 13: return WallGraceTime;
    case 14: return SameWallLockout;
    case 15: return WallCheckExtraDistance;
    case 16: return MovementRotationRate.Yaw;
    case 17: return WalkableFloorAngle;
    case 18: return MaxStepHeight;
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
    case 10: WallJumpUpSpeed = Value; break;
    case 11: WallJumpOutSpeed = Value; break;
    case 12: WallMomentumRetention = Value; break;
    case 13: WallGraceTime = Value; break;
    case 14: SameWallLockout = Value; break;
    case 15: WallCheckExtraDistance = Value; break;
    case 16: MovementRotationRate.Yaw = Value; break;
    case 17: WalkableFloorAngle = Value; break;
    case 18: MaxStepHeight = Value; break;
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

    Movement->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
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

void APhase1Character::LookYaw(const FInputActionValue& Value)
{
    AddControllerYawInput(Value.Get<float>() * MouseSensitivity);
}

void APhase1Character::LookPitch(const FInputActionValue& Value)
{
    AddControllerPitchInput(Value.Get<float>() * MouseSensitivity);
}

void APhase1Character::StartSprint()
{
    bSprinting = true;
    ApplyMovementTuning();
}

void APhase1Character::StopSprint()
{
    bSprinting = false;
    ApplyMovementTuning();
}

void APhase1Character::HandleJumpStarted()
{
    if (!TryWallKick())
    {
        Jump();
    }
}

void APhase1Character::HandleJumpCompleted()
{
    StopJumping();
}

void APhase1Character::UpdateWallContact()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || !Movement->IsFalling())
    {
        return;
    }

    FHitResult WallHit;
    if (FindNearbyWall(WallHit))
    {
        FVector HorizontalNormal(WallHit.ImpactNormal.X, WallHit.ImpactNormal.Y, 0.0f);
        if (HorizontalNormal.Normalize())
        {
            LastWallNormal = HorizontalNormal;
            LastWallContactTime = GetWorld()->GetTimeSeconds();
        }
    }
}

bool APhase1Character::FindNearbyWall(FHitResult& OutHit) const
{
    const UWorld* World = GetWorld();
    const UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (!World || !Capsule)
    {
        return false;
    }

    const FVector Start = GetActorLocation();
    const float TraceDistance = Capsule->GetScaledCapsuleRadius() + WallCheckExtraDistance;

    static const FVector Directions[] =
    {
        FVector(1.0f, 0.0f, 0.0f),
        FVector(-1.0f, 0.0f, 0.0f),
        FVector(0.0f, 1.0f, 0.0f),
        FVector(0.0f, -1.0f, 0.0f),
        FVector(1.0f, 1.0f, 0.0f).GetSafeNormal(),
        FVector(1.0f, -1.0f, 0.0f).GetSafeNormal(),
        FVector(-1.0f, 1.0f, 0.0f).GetSafeNormal(),
        FVector(-1.0f, -1.0f, 0.0f).GetSafeNormal()
    };

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WallKickTrace), false, this);
    bool bFoundWall = false;
    float BestDistance = TNumericLimits<float>::Max();

    for (const FVector& Direction : Directions)
    {
        FHitResult Hit;
        const FVector End = Start + Direction * TraceDistance;
        if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
        {
            continue;
        }

        if (GetCharacterMovement()->IsWalkable(Hit))
        {
            continue;
        }

        if (FMath::Abs(Hit.ImpactNormal.Z) > 0.35f)
        {
            continue;
        }

        if (Hit.Distance < BestDistance)
        {
            BestDistance = Hit.Distance;
            OutHit = Hit;
            bFoundWall = true;
        }
    }

    return bFoundWall;
}

bool APhase1Character::TryWallKick()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    UWorld* World = GetWorld();
    if (!Movement || !World || !Movement->IsFalling())
    {
        return false;
    }

    FHitResult WallHit;
    if (FindNearbyWall(WallHit))
    {
        FVector HorizontalNormal(WallHit.ImpactNormal.X, WallHit.ImpactNormal.Y, 0.0f);
        if (HorizontalNormal.Normalize())
        {
            LastWallNormal = HorizontalNormal;
            LastWallContactTime = World->GetTimeSeconds();
        }
    }

    const float Now = World->GetTimeSeconds();
    if ((Now - LastWallContactTime) > WallGraceTime || LastWallNormal.IsNearlyZero())
    {
        return false;
    }

    const bool bSameWall =
        !LastWallJumpNormal.IsNearlyZero() &&
        FVector::DotProduct(LastWallNormal, LastWallJumpNormal) > 0.85f;

    if (bSameWall && (Now - LastWallJumpTime) < SameWallLockout)
    {
        return false;
    }

    const FVector CurrentVelocity = GetVelocity();
    const FVector HorizontalVelocity(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

    const FVector AlongWallVelocity =
        HorizontalVelocity - LastWallNormal * FVector::DotProduct(HorizontalVelocity, LastWallNormal);

    const FVector NewHorizontalVelocity =
        AlongWallVelocity * WallMomentumRetention +
        LastWallNormal * WallJumpOutSpeed;

    LaunchCharacter(
        FVector(NewHorizontalVelocity.X, NewHorizontalVelocity.Y, WallJumpUpSpeed),
        true,
        true);

    LastWallJumpNormal = LastWallNormal;
    LastWallJumpTime = Now;
    LastWallContactTime = -1000.0f;

    return true;
}
