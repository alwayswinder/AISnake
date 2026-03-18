#include "SnakeManager.h"
#include "Snake.h"
#include "SnakeObstacle.h"
#include "SnakeSegment.h"
#include "SnakeSaveGame.h"
#include "SnakeHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

ASnakeManager::ASnakeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASnakeManager::BeginPlay()
{
	Super::BeginPlay();
	SetupCamera();
	DrawBoundary();

	// Show main menu via HUD
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASnakeHUD* HUD = Cast<ASnakeHUD>(PC->GetHUD()))
		{
			HUD->ShowMenu(LoadHighScores(), 0);
		}
	}
}

// ─────────────── Game Control ───────────────

void ASnakeManager::StartGame()
{
	ClearAll();
	CurrentScore = 0;
	GameState    = EGameState::Playing;

	SpawnSnake();
	SpawnFood();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASnakeHUD* HUD = Cast<ASnakeHUD>(PC->GetHUD()))
		{
			HUD->ShowGameUI();
			HUD->UpdateScore(0);
		}
		if (ManagedSnake)
		{
			PC->Possess(ManagedSnake);
		}
	}
}

void ASnakeManager::TriggerGameOver()
{
	if (GameState != EGameState::Playing) return;
	GameState = EGameState::GameOver;

	SaveHighScore(CurrentScore);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASnakeHUD* HUD = Cast<ASnakeHUD>(PC->GetHUD()))
		{
			HUD->ShowMenu(LoadHighScores(), CurrentScore);
		}
	}
}

// ─────────────── Cell Tracking ───────────────

void ASnakeManager::UpdateSnakeCells(const TArray<FVector2D>& BodyPositions)
{
	SnakeCells.Empty();
	for (const FVector2D& Pos : BodyPositions)
	{
		SnakeCells.Add(Pos);
	}
}

bool ASnakeManager::IsFoodAt(FVector2D GridPos, EFoodType& OutType) const
{
	if (const AFood* const* Found = FoodCells.Find(GridPos))
	{
		OutType = (*Found)->FoodType;
		return true;
	}
	return false;
}

bool ASnakeManager::IsObstacleAt(FVector2D GridPos) const
{
	return ObstacleCells.Contains(GridPos);
}

bool ASnakeManager::IsSnakeBodyAt(FVector2D GridPos) const
{
	return SnakeCells.Contains(GridPos);
}

void ASnakeManager::DestroyObstacleAt(FVector2D GridPos)
{
	if (ASnakeObstacle** Found = ObstacleCells.Find(GridPos))
	{
		(*Found)->Destroy();
		ActiveObstacles.Remove(*Found);
		ObstacleCells.Remove(GridPos);
	}
}

// ─────────────── Food Eaten ───────────────

void ASnakeManager::HandleFoodEaten(FVector2D GridPos)
{
	if (GameState != EGameState::Playing) return;

	CurrentScore += 10;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ASnakeHUD* HUD = Cast<ASnakeHUD>(PC->GetHUD()))
		{
			HUD->UpdateScore(CurrentScore);
		}
	}

	// Remove from cell map before animation so snake can walk through
	FoodCells.Remove(GridPos);

	if (ActiveFood)
	{
		AFood* EatenFood = ActiveFood;
		ActiveFood = nullptr;

		// Bind lambda for post-animation callback
		FOnCollectionComplete Callback;
		Callback.BindLambda([this, EatenFood]()
		{
			if (IsValid(EatenFood)) EatenFood->Destroy();
			SpawnFood();
			SpawnObstacle();
		});

		EatenFood->PlayCollectionAnimation(Callback);
	}
}

// ─────────────── Helpers ───────────────

FVector ASnakeManager::GridToWorld(FVector2D GridPos) const
{
	return FVector(GridPos.X * CellSize, GridPos.Y * CellSize, 0.f);
}

FVector2D ASnakeManager::FindRandomValidPosition() const
{
	// Collect all free cells
	TArray<FVector2D> FreeCells;
	FreeCells.Reserve(GridWidth * GridHeight);

	for (int32 X = 0; X < GridWidth; ++X)
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			FVector2D Cell(X, Y);
			if (!SnakeCells.Contains(Cell)
				&& !FoodCells.Contains(Cell)
				&& !ObstacleCells.Contains(Cell))
			{
				FreeCells.Add(Cell);
			}
		}
	}

	if (FreeCells.IsEmpty()) return FVector2D(-1.f, -1.f);
	return FreeCells[FMath::RandRange(0, FreeCells.Num() - 1)];
}

// ─────────────── Spawn / Clear ───────────────

void ASnakeManager::SpawnSnake()
{
	if (!SnakeClass) return;

	FVector2D StartPos(GridWidth / 2, GridHeight / 2);
	FVector   WorldPos = GridToWorld(StartPos);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ManagedSnake = GetWorld()->SpawnActor<ASnake>(SnakeClass, WorldPos, FRotator::ZeroRotator, Params);
	if (ManagedSnake)
	{
		ManagedSnake->Initialize(this, StartPos);
	}
}

void ASnakeManager::SpawnFood()
{
	if (!FoodClass) return;

	FVector2D SpawnPos = FindRandomValidPosition();
	if (SpawnPos == FVector2D(-1.f, -1.f)) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFood* NewFood = GetWorld()->SpawnActor<AFood>(FoodClass, GridToWorld(SpawnPos), FRotator::ZeroRotator, Params);
	if (NewFood)
	{
		NewFood->FoodType    = AFood::GetRandomFoodType();
		NewFood->GridPosition = SpawnPos;
		ActiveFood           = NewFood;
		FoodCells.Add(SpawnPos, NewFood);
	}
}

void ASnakeManager::SpawnObstacle()
{
	if (!ObstacleClass) return;

	FVector2D SpawnPos = FindRandomValidPosition();
	if (SpawnPos == FVector2D(-1.f, -1.f)) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASnakeObstacle* NewObs = GetWorld()->SpawnActor<ASnakeObstacle>(ObstacleClass, GridToWorld(SpawnPos), FRotator::ZeroRotator, Params);
	if (NewObs)
	{
		NewObs->GridPosition = SpawnPos;
		ActiveObstacles.Add(NewObs);
		ObstacleCells.Add(SpawnPos, NewObs);
	}
}

void ASnakeManager::ClearAll()
{
	if (ManagedSnake) { ManagedSnake->Destroy(); ManagedSnake = nullptr; }
	if (ActiveFood)   { ActiveFood->Destroy();   ActiveFood   = nullptr; }

	for (ASnakeObstacle* Obs : ActiveObstacles)
	{
		if (IsValid(Obs)) Obs->Destroy();
	}
	ActiveObstacles.Empty();
	SnakeCells.Empty();
	FoodCells.Empty();
	ObstacleCells.Empty();
}

// ─────────────── Camera & Visuals ───────────────

void ASnakeManager::SetupCamera() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	float CX = (GridWidth  * CellSize) * 0.5f;
	float CY = (GridHeight * CellSize) * 0.5f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACameraActor* Cam = GetWorld()->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(),
		FVector(CX, CY, 2500.f),
		FRotator(-90.f, 0.f, 0.f),
		Params);

	if (Cam)
	{
		Cam->GetCameraComponent()->ProjectionMode = ECameraProjectionMode::Orthographic;
		Cam->GetCameraComponent()->OrthoWidth     = GridWidth * CellSize * 1.1f;
		PC->SetViewTarget(Cam);
	}
}

void ASnakeManager::DrawBoundary() const
{
	float W = GridWidth  * CellSize;
	float H = GridHeight * CellSize;
	float Thickness = 5.f;
	float Duration  = 1e8f; // persistent
	FColor Color = FColor::White;

	DrawDebugLine(GetWorld(), FVector(0,  0,  1), FVector(W, 0,  1), Color, true, Duration, 0, Thickness);
	DrawDebugLine(GetWorld(), FVector(W, 0,  1), FVector(W, H,  1), Color, true, Duration, 0, Thickness);
	DrawDebugLine(GetWorld(), FVector(W, H,  1), FVector(0,  H,  1), Color, true, Duration, 0, Thickness);
	DrawDebugLine(GetWorld(), FVector(0,  H,  1), FVector(0,  0,  1), Color, true, Duration, 0, Thickness);
}

// ─────────────── Save / Load ───────────────

void ASnakeManager::SaveHighScore(int32 Score) const
{
	USnakeSaveGame* Save = Cast<USnakeSaveGame>(
		UGameplayStatics::LoadGameFromSlot(USnakeSaveGame::SlotName, USnakeSaveGame::UserIndex));

	if (!Save) Save = Cast<USnakeSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USnakeSaveGame::StaticClass()));

	if (Save)
	{
		Save->AddScore(Score);
		UGameplayStatics::SaveGameToSlot(Save, USnakeSaveGame::SlotName, USnakeSaveGame::UserIndex);
	}
}

TArray<int32> ASnakeManager::LoadHighScores() const
{
	USnakeSaveGame* Save = Cast<USnakeSaveGame>(
		UGameplayStatics::LoadGameFromSlot(USnakeSaveGame::SlotName, USnakeSaveGame::UserIndex));

	return Save ? Save->TopScores : TArray<int32>();
}
