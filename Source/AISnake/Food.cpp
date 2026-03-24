#include "Food.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AFood::AFood()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
	}
	MeshComponent->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFood::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCollecting) return;

	CollectTimer += DeltaTime;
	const float Duration = 0.5f;
	float Alpha = FMath::Min(CollectTimer / Duration, 1.0f);

	// Bounce up and scale down
	float Z = FMath::Lerp(0.0f, 100.0f, Alpha);
	float Scale = FMath::Lerp(0.6f, 0.0f, Alpha);
	SetActorLocation(InitialLocation + FVector(0.0f, 0.0f, Z));
	SetActorScale3D(FVector(Scale, Scale, Scale));

	if (CollectTimer >= Duration)
	{
		bIsCollecting = false;
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void AFood::InitFood(FIntPoint InGridPos, FVector WorldPos, EFoodType InType)
{
	GridPosition = InGridPos;
	FoodType = InType;
	SetActorLocation(WorldPos);
	SetActorHiddenInGame(false);
	SetActorScale3D(FVector(0.6f, 0.6f, 0.6f));
	bIsCollecting = false;
	CollectTimer = 0.0f;

	UMaterialInterface* Mat = nullptr;
	switch (FoodType)
	{
	case EFoodType::Normal:     Mat = NormalMaterial;     break;
	case EFoodType::Invisible:  Mat = InvisibleMaterial;  break;
	case EFoodType::Invincible: Mat = InvincibleMaterial; break;
	}
	if (Mat)
	{
		MeshComponent->SetMaterial(0, Mat);
	}
}

void AFood::PlayCollectAnimation()
{
	bIsCollecting = true;
	CollectTimer = 0.0f;
	InitialLocation = GetActorLocation();
}
