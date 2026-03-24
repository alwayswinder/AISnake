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

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* DefaultMaterial;

	void SetSegmentMaterial(UMaterialInterface* Mat);
};
