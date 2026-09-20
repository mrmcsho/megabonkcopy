#include "Core/PrototypeGameMode.h"

#include "Characters/Phase1Character.h"
#include "Core/PrototypeArena.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

APrototypeGameMode::APrototypeGameMode()
{
    DefaultPawnClass = APhase1Character::StaticClass();
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (!UGameplayStatics::GetActorOfClass(this, APrototypeArena::StaticClass()))
    {
        GetWorld()->SpawnActor<APrototypeArena>(APrototypeArena::StaticClass(), FTransform::Identity);
    }
}

AActor* APrototypeGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    if (AActor* ExistingStart = Super::ChoosePlayerStart_Implementation(Player))
    {
        ExistingStart->SetActorLocation(FVector(0, 0, 150));
        return ExistingStart;
    }
    return GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(0, 0, 150), FRotator::ZeroRotator);
}
