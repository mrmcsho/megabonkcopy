#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PrototypeGameMode.generated.h"

UCLASS()
class MEGABONKCOPY_API APrototypeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APrototypeGameMode();
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:
    virtual void BeginPlay() override;
};
