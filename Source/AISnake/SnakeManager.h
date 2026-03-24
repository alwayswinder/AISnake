#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Snake.h"
#include "Food.h"
#include "SnakeObstacle.h"
#include "SnakeSegment.h"
#include "SnakeSaveGame.h"
#include "SnakeManager.generated.h"

class ASnakeHUD;

UENUM()
enum class EGameState : uint8
{
	Menu,
	Playing,
	GameOver,
};

UCLASS()
class AISNAKE_API ASnakeManager : public AActor
{
	GENERATED_BODY()

public:
	ASnakeManager();

	virtual void BeginPlay() override;

	// ---- Blueprint-assigned subclasses ----
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<ASnake> SnakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AFood> FoodClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<ASnakeObstacle> ObstacleClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<ASnakeSegment> SegmentClass;

	UPROPERTY(EditDefaultsOnly, Category = "Floor")
	UMaterialInterface* FloorMaterial;

	// ---- Grid settings ----
	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	int32 GridCount = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	float GridCellSize = 100.0f;

	// ---- Game actions ----
	void StartGame();
	void EndGame();

private:
	// ---- Components ----
	UPROPERTY()
	UCameraComponent* Camera;

	UPROPERTY()
	UStaticMeshComponent* FloorMesh;

	// ---- Runtime state ----
	EGameState GameState = EGameState::Menu;
	int32 Score          = 0;
	int32 LastScore      = 0;

	UPROPERTY()
	ASnake* SnakeActor = nullptr;

	UPROPERTY()
	AFood* FoodActor = nullptr;

	UPROPERTY()
	TArray<ASnakeObstacle*> Obstacles;

	UPROPERTY()
	USnakeSaveGame* SaveGame = nullptr;

	// ---- Spawning helpers ----
	void SpawnFood();
	void SpawnObstacle();
	void SpawnFoodAndObstacle();
	void DestroyAllGameActors();

	FVector GridToWorld(FIntPoint GridPos) const;
	FIntPoint GetRandomFreePosition() const;
	EFoodType PickRandomFoodType() const;

	void LoadHighScores();
	void SaveHighScore();
	TArray<int32> GetHighScores() const;

	ASnakeHUD* GetSnakeHUD() const;

	// ---- Snake move callback ----
	UFUNCTION()
	void HandleSnakeMoved();
};
