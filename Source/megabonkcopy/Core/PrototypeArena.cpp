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
    AddBlock(TEXT("Slope15"), FVector(1300, -1900, 190), FVector(1600, 700, 80), FRotator(-15, 0, 0));
    AddBlock(TEXT("Slope30"), FVector(2100, 1200, 400), FVector(1500, 700, 80), FRotator(-30, 0, 0));
    AddBlock(TEXT("Slope45"), FVector(3300, -300, 560), FVector(1500, 650, 80), FRotator(-45, 0, 0));
    AddBlock(TEXT("LongDownhillRamp"), FVector(-2600, -1900, 520), FVector(3200, 900, 90), FRotator(18, 0, 0));
    AddBlock(TEXT("Platform"), FVector(-1700, 1300, 225), FVector(900, 900, 450));
    AddBlock(TEXT("PlatformRamp"), FVector(-1150, 1300, 105), FVector(850, 600, 70), FRotator(17, 0, 0));
    AddBlock(TEXT("ObstacleA"), FVector(-400, -1000, 100), FVector(300, 300, 200));
    AddBlock(TEXT("ObstacleB"), FVector(250, -1150, 175), FVector(350, 350, 350));
    AddBlock(TEXT("ObstacleC"), FVector(650, -650, 250), FVector(400, 400, 500));
    AddBlock(TEXT("WallKickLeft"), FVector(-600, 2600, 650), FVector(120, 1700, 1300));
    AddBlock(TEXT("WallKickRight"), FVector(600, 2600, 650), FVector(120, 1700, 1300));
    AddBlock(TEXT("HighPlatform"), FVector(-2800, 1700, 450), FVector(900, 900, 900));
    AddBlock(TEXT("LowPlatform"), FVector(-1500, 2600, 140), FVector(700, 700, 280));
    AddBlock(TEXT("Step1"), FVector(1100, 2750, 25), FVector(400, 700, 50));
    AddBlock(TEXT("Step2"), FVector(1500, 2750, 50), FVector(400, 700, 100));
    AddBlock(TEXT("Step3"), FVector(1900, 2750, 78), FVector(400, 700, 156));

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
