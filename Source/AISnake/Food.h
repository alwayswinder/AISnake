#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Food.generated.h"

UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Normal     UMETA(DisplayName = "Normal"),
	Invisible  UMETA(DisplayName = "Invisible"),
	Invincible UMETA(DisplayName = "Invincible"),
};

UCLASS()
class AISNAKE_API AFood : public AActor
{
	GENERATED_BODY()

public:
	AFood();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* NormalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvisibleMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvincibleMaterial;

	EFoodType FoodType;
	FIntPoint GridPosition;

	void InitFood(FIntPoint InGridPos, FVector WorldPos, EFoodType InType);
	void PlayCollectAnimation();

private:
	bool bIsCollecting = false;
	float CollectTimer = 0.0f;
	FVector InitialLocation;
};
