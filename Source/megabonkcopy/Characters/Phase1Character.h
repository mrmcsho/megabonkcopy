#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Phase1Character.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class MEGABONKCOPY_API APhase1Character : public ACharacter
{
    GENERATED_BODY()

public:
    APhase1Character();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    bool IsMovementTuningVisible() const { return bMovementTuningVisible; }
    int32 GetMovementTuningSelection() const { return MovementTuningSelection; }
    int32 GetMovementTuningCount() const;
    FString GetMovementTuningName(int32 Index) const;
    float GetMovementTuningValue(int32 Index) const;
    FString GetMovementStateText() const;
    FString GetRecentMovementEvent() const;
    bool HasWallContact() const;
    float GetNormalTargetSpeed() const { return bSprinting ? SprintSpeed : WalkSpeed; }
    float GetOverspeed() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") TObjectPtr<UCameraComponent> FollowCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual") TObjectPtr<UStaticMeshComponent> DebugTorso;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual") TObjectPtr<UStaticMeshComponent> DebugHead;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual") TObjectPtr<UStaticMeshComponent> DebugLeftLeg;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual") TObjectPtr<UStaticMeshComponent> DebugRightLeg;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual") TObjectPtr<UStaticMeshComponent> DebugForwardMarker;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Speed") float WalkSpeed = 650.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Speed") float SprintSpeed = 2350.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") float MaxAcceleration = 7100.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") float GroundBrakingDeceleration = 12000.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") float GroundFriction = 9.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Air") float AirControl = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Air") float AirControlBoostMultiplier = 1.6f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Air") float FallingLateralFriction = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float JumpVelocity = 720.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float GravityScale = 2.25f;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float MomentumRetention = 0.999f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float GroundMomentumRetention = 0.998f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float AirMomentumRetention = 0.9995f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float LandingMomentumRetention = 0.96f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float SoftSpeedCap = 3200.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float OverspeedDrag = 0.65f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Momentum") float MaxReasonableSpeed = 5200.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float JumpMomentumRetention = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float JumpBufferTime = 0.14f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float GroundCoyoteTime = 0.12f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Jump") float BunnyHopRetention = 0.98f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Steering") float GroundTurnInfluence = 7.5f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Steering") float AirTurnInfluence = 2.5f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Steering") float DiagonalMovementMultiplier = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Slope") float SlopeAcceleration = 900.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Slope") float DownhillMomentumMultiplier = 1.08f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Slope") float UphillResistance = 0.35f;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float WallJumpUpSpeed = 1000.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float WallJumpOutSpeed = 400.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float WallMomentumRetention = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float WallGraceTime = 0.12f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float SameWallLockout = 0.20f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Wall Kick") float WallCheckExtraDistance = 28.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") FRotator MovementRotationRate = FRotator(0, 900, 0);
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") float WalkableFloorAngle = 46.0f;
    UPROPERTY(EditDefaultsOnly, Category="Movement|Ground") float MaxStepHeight = 52.0f;

    UPROPERTY(EditDefaultsOnly, Category="Camera") float MouseSensitivity = 1.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float PitchMin = -65.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float PitchMax = 55.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float CameraDistance = 480.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") bool bEnableCameraLag = true;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float CameraLagSpeed = 18.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera") bool bEnableCameraRotationLag = false;
    UPROPERTY(EditDefaultsOnly, Category="Camera") float CameraRotationLagSpeed = 20.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera|FOV") float DefaultFOV = 90.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera|FOV") float SprintFOV = 97.0f;
    UPROPERTY(EditDefaultsOnly, Category="Camera|FOV") float FOVBlendSpeed = 9.0f;

private:
    void MoveForward(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void LookYaw(const FInputActionValue& Value);
    void LookPitch(const FInputActionValue& Value);
    void StartSprint(); void StopSprint();
    void HandleJumpStarted(); void HandleJumpCompleted();
    void UpdateGroundMovement(float DeltaSeconds, const FVector& WishDirection, float InputScale);
    void UpdateAirMovement(float DeltaSeconds, const FVector& WishDirection, float InputScale);
    void UpdateMomentum(float DeltaSeconds, bool bGrounded);
    void UpdateSlopeMovement(float DeltaSeconds, const FVector& WishDirection);
    void UpdateWallContact();
    void HandleLanding();
    bool PerformJump(bool bBunnyHop);
    bool FindNearbyWall(FHitResult& OutHit) const;
    bool TryWallKick();
    void RecordMovementEvent(const TCHAR* EventName);
    void ConfigureInputMapping();
    void ToggleMovementTuning(); void SelectPreviousMovementTuning(); void SelectNextMovementTuning();
    void DecreaseMovementTuningFine(); void IncreaseMovementTuningFine();
    void DecreaseMovementTuningCoarse(); void IncreaseMovementTuningCoarse();
    void ResetSelectedMovementTuning(); void ResetAllMovementTuning();
    void AdjustMovementTuning(float Direction, bool bCoarse);
    void SetMovementTuningValue(int32 Index, float Value);
    void ApplyMovementTuning();

    UPROPERTY(Transient) TObjectPtr<UInputMappingContext> InputContext;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MoveForwardAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MoveRightAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> LookYawAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> LookPitchAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> SprintAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> JumpAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningToggleAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningPreviousAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningNextAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningDecreaseFineAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningIncreaseFineAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningDecreaseCoarseAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningIncreaseCoarseAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningResetSelectedAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> TuningResetAllAction;

    bool bSprinting = false;
    bool bMovementTuningVisible = false;
    bool bWasFalling = false;
    bool bWallKickState = false;
    int32 MovementTuningSelection = 0;
    float ForwardInput = 0.0f;
    float RightInput = 0.0f;
    float LastGroundedTime = -1000.0f;
    float JumpPressedTime = -1000.0f;
    float LastWallContactTime = -1000.0f;
    float LastWallJumpTime = -1000.0f;
    float RecentEventTime = -1000.0f;
    FVector LastAirHorizontalVelocity = FVector::ZeroVector;
    FVector LastWallNormal = FVector::ZeroVector;
    FVector LastWallJumpNormal = FVector::ZeroVector;
    FString RecentMovementEvent;
};
