#include "SnakeManager.h"
#include "SnakeHUD.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASnakeManager::ASnakeManager()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Floor plane — scaled to grid size in BeginPlay
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(RootComponent);
	FloorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded()) FloorMesh->SetStaticMesh(PlaneMesh.Object);

	// Camera — top-down, orthographic, centered above grid
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 3000.0f));
	Camera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
}

void ASnakeManager::BeginPlay()
{
	Super::BeginPlay();

	// Setup floor scale and material from grid constants
	float MapSize = GridCount * GridCellSize;            // e.g. 2000
	float PlaneMeshNativeSize = 100.0f;
	float Scale = MapSize / PlaneMeshNativeSize;
	FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -1.0f));
	FloorMesh->SetRelativeScale3D(FVector(Scale, Scale, 1.0f));
	if (FloorMaterial) FloorMesh->SetMaterial(0, FloorMaterial);

	// Ortho width covers full map + 10% padding
	Camera->SetOrthoWidth(MapSize * 1.1f);

	// Load or create save game
	LoadHighScores();

	// Point camera at this manager immediately
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetViewTargetWithBlend(this, 0.0f);
	}

	// Defer HUD setup to next tick so ASnakeHUD::BeginPlay() runs first and widgets are created
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		ASnakeHUD* HUD = GetSnakeHUD();
		if (HUD)
		{
			HUD->SetStartGameCallback(FSimpleDelegate::CreateUObject(this, &ASnakeManager::StartGame));
			HUD->ShowMenu(0, GetHighScores());
		}
	}));
}

// ─────────────────────────────────────────────────────────────────────────────
// Game flow
// ─────────────────────────────────────────────────────────────────────────────

void ASnakeManager::StartGame()
{
	if (GameState == EGameState::Playing) return;

	DestroyAllGameActors();

	Score     = 0;
	GameState = EGameState::Playing;

	// Show game UI
	ASnakeHUD* HUD = GetSnakeHUD();
	if (HUD) HUD->ShowGame();

	// Spawn snake at center, facing right (screen-right = world +Y)
	if (!SnakeClass) return;
	FIntPoint StartPos(GridCount / 2, GridCount / 2);
	FIntPoint StartDir(0, 1);

	SnakeActor = GetWorld()->SpawnActor<ASnake>(SnakeClass, GridToWorld(StartPos), FRotator::ZeroRotator);
	if (!SnakeActor) return;

	SnakeActor->Initialize(StartPos, StartDir, GridCount, GridCellSize, SegmentClass);
	SnakeActor->OnSnakeMoved.AddDynamic(this, &ASnakeManager::HandleSnakeMoved);

	// Possess the snake
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->Possess(SnakeActor);
		// Override viewtarget back to manager so camera stays fixed
		PC->SetViewTargetWithBlend(this, 0.0f);
	}

	// Spawn initial food
	SpawnFood();
}

void ASnakeManager::DestroyAllGameActors()
{
	if (SnakeActor)
	{
		SnakeActor->Cleanup();
		SnakeActor->Destroy();
		SnakeActor = nullptr;
	}
	if (FoodActor)
	{
		FoodActor->Destroy();
		FoodActor = nullptr;
	}
	for (ASnakeObstacle* Obs : Obstacles)
	{
		if (Obs) Obs->Destroy();
	}
	Obstacles.Empty();
}

// ─────────────────────────────────────────────────────────────────────────────
// Snake move callback — collision detection
// ─────────────────────────────────────────────────────────────────────────────

void ASnakeManager::HandleSnakeMoved()
{
	if (!SnakeActor || GameState != EGameState::Playing) return;

	FIntPoint HeadPos = SnakeActor->GetHeadGridPosition();
	TArray<FIntPoint> Body = SnakeActor->GetBodyGridPositions();

	// ---- Check food ----
	if (FoodActor && HeadPos == FoodActor->GridPosition)
	{
		EFoodType Eaten = FoodActor->FoodType;

		// Apply effect
		switch (Eaten)
		{
		case EFoodType::Invisible:
			SnakeActor->ApplyInvisibleEffect(10.0f);
			break;
		case EFoodType::Invincible:
			SnakeActor->ApplyInvincibleEffect(20.0f);
			break;
		default:
			break;
		}

		// Score and body
		Score += 10;
		SnakeActor->AddSegment();

		ASnakeHUD* HUD = GetSnakeHUD();
		if (HUD) HUD->UpdateGameScore(Score);

		// Collect animation; respawn food + obstacle after 0.5 s
		FoodActor->PlayCollectAnimation();

		FTimerHandle FoodTimer;
		GetWorldTimerManager().SetTimer(FoodTimer,
			FTimerDelegate::CreateUObject(this, &ASnakeManager::SpawnFoodAndObstacle),
			0.5f, false);

		return; // skip collision checks this frame after eating
	}

	// ---- Check obstacle collision ----
	for (int32 i = Obstacles.Num() - 1; i >= 0; i--)
	{
		ASnakeObstacle* Obs = Obstacles[i];
		if (!Obs) continue;

		if (HeadPos == Obs->GridPosition)
		{
			if (SnakeActor->IsInvincible())
			{
				// Destroy obstacle
				Obs->Destroy();
				Obstacles.RemoveAt(i);
			}
			else
			{
				// Game over
				EndGame();
				return;
			}
		}
	}

	// ---- Check self collision ----
	if (!SnakeActor->IsInvisible())
	{
		// Body[0] is head; check against 1..N
		for (int32 i = 1; i < Body.Num(); i++)
		{
			if (HeadPos == Body[i])
			{
				EndGame();
				return;
			}
		}
	}
}

void ASnakeManager::EndGame()
{
	if (GameState != EGameState::Playing) return;
	GameState = EGameState::GameOver;
	LastScore = Score;

	SaveHighScore();

	// Show menu immediately (also switches InputMode to UIOnly)
	ASnakeHUD* HUD = GetSnakeHUD();
	if (HUD) HUD->ShowMenu(LastScore, GetHighScores());

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC) PC->UnPossess();

	DestroyAllGameActors();
	GameState = EGameState::Menu;
}

void ASnakeManager::SpawnFoodAndObstacle()
{
	SpawnFood();
	SpawnObstacle();
}

// ─────────────────────────────────────────────────────────────────────────────
// Spawning helpers
// ─────────────────────────────────────────────────────────────────────────────

void ASnakeManager::SpawnFood()
{
	if (!FoodClass) return;

	FIntPoint Pos = GetRandomFreePosition();
	EFoodType Type = PickRandomFoodType();

	if (!FoodActor)
	{
		FoodActor = GetWorld()->SpawnActor<AFood>(FoodClass, GridToWorld(Pos), FRotator::ZeroRotator);
	}
	if (FoodActor)
	{
		FoodActor->InitFood(Pos, GridToWorld(Pos), Type);
	}
}

void ASnakeManager::SpawnObstacle()
{
	if (!ObstacleClass) return;

	FIntPoint Pos = GetRandomFreePosition();
	ASnakeObstacle* Obs = GetWorld()->SpawnActor<ASnakeObstacle>(
		ObstacleClass, GridToWorld(Pos), FRotator::ZeroRotator);
	if (Obs)
	{
		Obs->InitObstacle(Pos, GridToWorld(Pos));
		Obstacles.Add(Obs);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Position helpers
// ─────────────────────────────────────────────────────────────────────────────

FVector ASnakeManager::GridToWorld(FIntPoint GridPos) const
{
	float HalfSize = GridCount * GridCellSize * 0.5f;
	float X = GridPos.X * GridCellSize - HalfSize + GridCellSize * 0.5f;
	float Y = GridPos.Y * GridCellSize - HalfSize + GridCellSize * 0.5f;
	return FVector(X, Y, 50.0f);
}

FIntPoint ASnakeManager::GetRandomFreePosition() const
{
	// Build set of occupied positions
	TSet<FIntPoint> Occupied;

	if (SnakeActor)
	{
		for (FIntPoint P : SnakeActor->GetBodyGridPositions()) Occupied.Add(P);
	}
	if (FoodActor && !FoodActor->IsHidden())
	{
		Occupied.Add(FoodActor->GridPosition);
	}
	for (const ASnakeObstacle* Obs : Obstacles)
	{
		if (Obs) Occupied.Add(Obs->GridPosition);
	}

	// Try random positions until we find a free one
	FIntPoint Pos;
	int32 Tries = 0;
	do
	{
		Pos = FIntPoint(FMath::RandRange(0, GridCount - 1),
		                FMath::RandRange(0, GridCount - 1));
		Tries++;
	}
	while (Occupied.Contains(Pos) && Tries < GridCount * GridCount);

	return Pos;
}

EFoodType ASnakeManager::PickRandomFoodType() const
{
	// Normal 70%, Invisible 10%, Invincible 20%
	int32 Roll = FMath::RandRange(1, 100);
	if (Roll <= 70)  return EFoodType::Normal;
	if (Roll <= 90)  return EFoodType::Invisible;
	return EFoodType::Invincible;
}

// ─────────────────────────────────────────────────────────────────────────────
// Save game
// ─────────────────────────────────────────────────────────────────────────────

void ASnakeManager::LoadHighScores()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("SnakeHighScores"), 0))
	{
		SaveGame = Cast<USnakeSaveGame>(
			UGameplayStatics::LoadGameFromSlot(TEXT("SnakeHighScores"), 0));
	}
	if (!SaveGame)
	{
		SaveGame = Cast<USnakeSaveGame>(
			UGameplayStatics::CreateSaveGameObject(USnakeSaveGame::StaticClass()));
	}
}

void ASnakeManager::SaveHighScore()
{
	if (!SaveGame) LoadHighScores();
	SaveGame->AddScore(Score);
	UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("SnakeHighScores"), 0);
}

TArray<int32> ASnakeManager::GetHighScores() const
{
	return SaveGame ? SaveGame->HighScores : TArray<int32>();
}

ASnakeHUD* ASnakeManager::GetSnakeHUD() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	return PC ? Cast<ASnakeHUD>(PC->GetHUD()) : nullptr;
}
