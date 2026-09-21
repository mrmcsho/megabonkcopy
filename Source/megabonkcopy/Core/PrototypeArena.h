#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrototypeArena.generated.h"

class USceneComponent;

UCLASS()
class MEGABONKCOPY_API APrototypeArena : public AActor
{
    GENERATED_BODY()

public:
    APrototypeArena();

private:
    void AddBlock(const TCHAR* Name, const FVector& Location, const FVector& Size, const FRotator& Rotation = FRotator::ZeroRotator);

    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
};
