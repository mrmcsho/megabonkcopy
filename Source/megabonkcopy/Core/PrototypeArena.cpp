#include "Core/PrototypeArena.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APrototypeArena::APrototypeArena()
{
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    AddBlock(TEXT("Ground"), FVector(0, 0, -50), FVector(8000, 8000, 100));
    AddBlock(TEXT("GentleSlope"), FVector(1300, -1700, 125), FVector(1400, 700, 80), FRotator(-12, 0, 0));
    AddBlock(TEXT("SteepSlope"), FVector(1800, 1400, 280), FVector(1300, 700, 80), FRotator(-32, 0, 0));
    AddBlock(TEXT("Platform"), FVector(-1700, 1300, 225), FVector(900, 900, 450));
    AddBlock(TEXT("PlatformRamp"), FVector(-1150, 1300, 105), FVector(850, 600, 70), FRotator(17, 0, 0));
    AddBlock(TEXT("ObstacleA"), FVector(-400, -1000, 100), FVector(300, 300, 200));
    AddBlock(TEXT("ObstacleB"), FVector(250, -1150, 175), FVector(350, 350, 350));
    AddBlock(TEXT("ObstacleC"), FVector(650, -650, 250), FVector(400, 400, 500));

    UDirectionalLightComponent* Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
    Sun->SetupAttachment(SceneRoot);
    Sun->SetRelativeRotation(FRotator(-55, -35, 0));
    Sun->SetIntensity(6.0f);

    USkyLightComponent* Sky = CreateDefaultSubobject<USkyLightComponent>(TEXT("Sky"));
    Sky->SetupAttachment(SceneRoot);
    Sky->SetIntensity(1.0f);
}

void APrototypeArena::AddBlock(const TCHAR* Name, const FVector& Location, const FVector& Size, const FRotator& Rotation)
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMeshComponent* Block = CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Block->SetupAttachment(SceneRoot);
    Block->SetStaticMesh(Cube.Object);
    Block->SetRelativeLocation(Location);
    Block->SetRelativeRotation(Rotation);
    Block->SetRelativeScale3D(Size / 100.0f);
    Block->SetCollisionProfileName(TEXT("BlockAll"));
}
