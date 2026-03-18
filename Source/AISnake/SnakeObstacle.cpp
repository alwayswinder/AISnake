#include "SnakeObstacle.h"
#include "Components/StaticMeshComponent.h"

ASnakeObstacle::ASnakeObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
