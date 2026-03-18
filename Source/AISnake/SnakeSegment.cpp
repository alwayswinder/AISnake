#include "SnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ASnakeSegment::ASnakeSegment()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
