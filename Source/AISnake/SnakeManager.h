#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Food.h"
#include "SnakeManager.generated.h"

class ASnake;
class ASnakeObstacle;
class ASnakeSegment;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Menu      UMETA(DisplayName="Menu"),
	Playing   UMETA(DisplayName="Playing"),
	GameOver  UMETA(DisplayName="GameOver")
};

UCLASS()
class AISNAKE_API ASnakeManager : public AActor
{
	GENERATED_BODY()

public:
	ASnakeManager();

	virtual void BeginPlay() override;

	// ─────────────── Grid Config ───────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridWidth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	float CellSize = 100.f;

	// ─────────────── Spawnable Classes ───────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<ASnake> SnakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<AFood> FoodClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<ASnakeObstacle> ObstacleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<ASnakeSegment> SegmentClass;

	// ─────────────── Game State ───────────────
	UPROPERTY(BlueprintReadOnly, Category="Game")
	int32 CurrentScore = 0;

	UPROPERTY(BlueprintReadOnly, Category="Game")
	EGameState GameState = EGameState::Menu;

	// ─────────────── Public API ───────────────
	UFUNCTION(BlueprintCallable, Category="Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category="Game")
	void TriggerGameOver();

	/** Called by Snake after every move; snake passes its full body cell list. */
	void UpdateSnakeCells(const TArray<FVector2D>& BodyPositions);

	/** Returns true and sets OutType if there is food at GridPos. */
	bool IsFoodAt(FVector2D GridPos, EFoodType& OutType) const;

	bool IsObstacleAt(FVector2D GridPos) const;
	bool IsSnakeBodyAt(FVector2D GridPos) const;

	/** Destroys obstacle at pos and removes it from tracking. */
	void DestroyObstacleAt(FVector2D GridPos);

	/** Runs food-eaten sequence: score, animation, respawn, obstacle. */
	void HandleFoodEaten(FVector2D GridPos);

	/** World position for the centre of a grid cell. */
	UFUNCTION(BlueprintCallable, Category="Grid")
	FVector GridToWorld(FVector2D GridPos) const;

	/** Pick a random cell that has nothing on it. Returns FVector2D(-1,-1) if full. */
	FVector2D FindRandomValidPosition() const;

	/** Loads top-10 scores from disk. */
	TArray<int32> LoadHighScores() const;

private:
	UPROPERTY()
	ASnake* ManagedSnake = nullptr;

	UPROPERTY()
	AFood* ActiveFood = nullptr;

	UPROPERTY()
	TArray<ASnakeObstacle*> ActiveObstacles;

	// Fast-lookup sets (grid coords stored as FVector2D)
	TSet<FVector2D> SnakeCells;
	TMap<FVector2D, AFood*> FoodCells;
	TMap<FVector2D, ASnakeObstacle*> ObstacleCells;

	void SpawnSnake();
	void SpawnFood();
	void SpawnObstacle();
	void ClearAll();
	void SetupCamera();
	void DrawBoundary() const;
	void SaveHighScore(int32 Score) const;

	UPROPERTY()
	ACameraActor* GameCamera = nullptr;
};
