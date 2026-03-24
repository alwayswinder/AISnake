#include "SnakeObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASnakeObstacle::ASnakeObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
	MeshComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.7f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASnakeObstacle::InitObstacle(FIntPoint InGridPos, FVector WorldPos)
{
	GridPosition = InGridPos;
	SetActorLocation(WorldPos);
	if (ObstacleMaterial)
	{
		MeshComponent->SetMaterial(0, ObstacleMaterial);
	}
}
