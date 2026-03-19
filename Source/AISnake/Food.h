#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Food.generated.h"

UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Normal		UMETA(DisplayName="Normal"),
	Invisible	UMETA(DisplayName="Invisible"),
	Invincible	UMETA(DisplayName="Invincible")
};

DECLARE_DELEGATE(FOnCollectionComplete);

UCLASS()
class AISNAKE_API AFood : public AActor
{
	GENERATED_BODY()

public:
	AFood();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Food")
	EFoodType FoodType = EFoodType::Normal;

	/** Per-type materials – assign in BP_Food */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* NormalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* InvisibleMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* InvincibleMaterial = nullptr;

	/** Apply the material that matches the current FoodType. Call after FoodType is set. */
	void ApplyMaterial();

	FVector2D GridPosition;

	/** Called by SnakeManager when the snake eats this food. */
	void PlayCollectionAnimation(FOnCollectionComplete Callback);

	/** Returns random type respecting 70/10/20 weights. */
	static EFoodType GetRandomFoodType();

private:
	bool   bIsBeingCollected  = false;
	float  CollectionTimer    = 0.f;
	float  CollectionDuration = 0.5f;
	FVector OriginalLocation;
	FOnCollectionComplete OnCollectionComplete;
};
