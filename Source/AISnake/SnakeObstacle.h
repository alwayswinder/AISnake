#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeObstacle.generated.h"

UCLASS()
class AISNAKE_API ASnakeObstacle : public AActor
{
	GENERATED_BODY()

public:
	ASnakeObstacle();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* ObstacleMaterial;

	FIntPoint GridPosition;

	void InitObstacle(FIntPoint InGridPos, FVector WorldPos);
};
