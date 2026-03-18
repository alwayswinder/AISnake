#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeSegment.generated.h"

UCLASS()
class AISNAKE_API ASnakeSegment : public AActor
{
	GENERATED_BODY()

public:
	ASnakeSegment();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComp;

	FVector2D GridPosition;
};
