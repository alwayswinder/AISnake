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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComp;

	FVector2D GridPosition;
};
