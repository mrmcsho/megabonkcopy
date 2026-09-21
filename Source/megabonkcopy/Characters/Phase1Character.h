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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    // Temporary visible body used only for Phase 1 movement-feel testing.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual")
    TObjectPtr<UStaticMeshComponent> DebugTorso;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual")
    TObjectPtr<UStaticMeshComponent> DebugHead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual")
    TObjectPtr<UStaticMeshComponent> DebugLeftLeg;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual")
    TObjectPtr<UStaticMeshComponent> DebugRightLeg;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug Visual")
    TObjectPtr<UStaticMeshComponent> DebugForwardMarker;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Speed", meta=(ClampMin="0"))
    float WalkSpeed = 650.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Speed", meta=(ClampMin="0"))
    float SprintSpeed = 1050.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Response")
    float MaxAcceleration = 4200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Response")
    float GroundBrakingDeceleration = 3600.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Response")
    float GroundFriction = 9.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air")
    float AirControl = 0.42f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air")
    float AirControlBoostMultiplier = 1.6f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air")
    float FallingLateralFriction = 0.15f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Jump")
    float JumpVelocity = 720.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Jump")
    float GravityScale = 2.25f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0"))
    float WallJumpUpSpeed = 760.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0"))
    float WallJumpOutSpeed = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0", ClampMax="1"))
    float WallMomentumRetention = 0.75f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0"))
    float WallGraceTime = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0"))
    float SameWallLockout = 0.20f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Wall Kick", meta=(ClampMin="0"))
    float WallCheckExtraDistance = 28.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Ground")
    FRotator MovementRotationRate = FRotator(0.0f, 900.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Ground", meta=(ClampMin="0", ClampMax="89"))
    float WalkableFloorAngle = 46.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Ground")
    float MaxStepHeight = 50.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float PitchMin = -65.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float PitchMax = 55.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float CameraDistance = 480.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    bool bEnableCameraLag = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float CameraLagSpeed = 18.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    bool bEnableCameraRotationLag = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera")
    float CameraRotationLagSpeed = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|FOV")
    float DefaultFOV = 90.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|FOV")
    float SprintFOV = 97.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|FOV")
    float FOVBlendSpeed = 9.0f;

private:
    void MoveForward(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void LookYaw(const FInputActionValue& Value);
    void LookPitch(const FInputActionValue& Value);
    void StartSprint();
    void StopSprint();
    void HandleJumpStarted();
    void HandleJumpCompleted();
    void UpdateWallContact();
    bool FindNearbyWall(FHitResult& OutHit) const;
    bool TryWallKick();
    void ConfigureInputMapping();

    UPROPERTY(Transient) TObjectPtr<UInputMappingContext> InputContext;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MoveForwardAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MoveRightAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> LookYawAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> LookPitchAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> SprintAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> JumpAction;

    bool bSprinting = false;
    FVector LastWallNormal = FVector::ZeroVector;
    FVector LastWallJumpNormal = FVector::ZeroVector;
    float LastWallContactTime = -1000.0f;
    float LastWallJumpTime = -1000.0f;
};
