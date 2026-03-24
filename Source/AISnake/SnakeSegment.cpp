#include "SnakeSegment.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASnakeSegment::ASnakeSegment()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
	// Scale to 90x90x50 for visual separation between cells
	MeshComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.5f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASnakeSegment::SetSegmentMaterial(UMaterialInterface* Mat)
{
	if (Mat)
	{
		MeshComponent->SetMaterial(0, Mat);
	}
}
