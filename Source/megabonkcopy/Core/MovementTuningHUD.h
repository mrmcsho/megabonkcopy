#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MovementTuningHUD.generated.h"

UCLASS()
class MEGABONKCOPY_API AMovementTuningHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};
