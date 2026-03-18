#include "Food.h"
#include "Components/StaticMeshComponent.h"

AFood::AFood()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFood::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsBeingCollected) return;

	CollectionTimer += DeltaTime;
	float Alpha = FMath::Clamp(CollectionTimer / CollectionDuration, 0.f, 1.f);

	// Bounce-up: lerp Z from original to +50 units
	float NewZ = FMath::Lerp(OriginalLocation.Z, OriginalLocation.Z + 50.f, Alpha);
	SetActorLocation(FVector(OriginalLocation.X, OriginalLocation.Y, NewZ));

	// Scale-out: lerp from 1 to 0
	float Scale = FMath::Lerp(1.f, 0.f, Alpha);
	SetActorScale3D(FVector(Scale));

	if (CollectionTimer >= CollectionDuration)
	{
		bIsBeingCollected = false;
		OnCollectionComplete.ExecuteIfBound();
	}
}

void AFood::PlayCollectionAnimation(FOnCollectionComplete Callback)
{
	OriginalLocation      = GetActorLocation();
	bIsBeingCollected     = true;
	CollectionTimer       = 0.f;
	OnCollectionComplete  = Callback;
}

EFoodType AFood::GetRandomFoodType()
{
	float Rand = FMath::FRand();
	if (Rand < 0.70f) return EFoodType::Normal;
	if (Rand < 0.80f) return EFoodType::Invisible;  // 70-80 → 10%
	return EFoodType::Invincible;                    // 80-100 → 20%
}
