#include "Snake.h"
#include "SnakeManager.h"
#include "SnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ASnake::ASnake()
{
	PrimaryActorTick.bCanEverTick = false;

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	RootComponent = HeadMesh;
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ASnake::BeginPlay()
{
	Super::BeginPlay();
}

void ASnake::Initialize(ASnakeManager* Manager, FVector2D StartPos)
{
	ManagerRef = Manager;

	// Build initial body: 3 segments facing +X
	BodyPositions.Empty();
	BodyPositions.Add(StartPos);
	BodyPositions.Add(StartPos + FVector2D(-1.f, 0.f));
	BodyPositions.Add(StartPos + FVector2D(-2.f, 0.f));

	// Spawn segment actors for tail (index 1+)
	for (int32 i = 1; i < BodyPositions.Num(); ++i)
	{
		GrowBody(); // spawns segment at correct position below
	}
	// Override positions that GrowBody set incorrectly (it uses BodyPositions tail)
	// — re-sync all segment world positions
	for (int32 i = 1; i < Segments.Num() + 1; ++i)
	{
		if (Segments.IsValidIndex(i - 1) && IsValid(Segments[i - 1]))
		{
			Segments[i - 1]->SetActorLocation(GridToWorld(BodyPositions[i]));
			Segments[i - 1]->GridPosition = BodyPositions[i];
		}
	}

	// Set head position
	SetActorLocation(GridToWorld(BodyPositions[0]));

	// Notify manager of initial cells
	ManagerRef->UpdateSnakeCells(BodyPositions);

	CurrentDirection = FVector2D(1.f, 0.f);
	DirQueue.Empty();

	ScheduleMoveTimer(NormalMoveInterval);
}

void ASnake::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Segments are independent actors; destroy them before the snake is removed
	for (ASnakeSegment* Seg : Segments)
	{
		if (IsValid(Seg)) Seg->Destroy();
	}
	Segments.Empty();
	Super::EndPlay(EndPlayReason);
}



void ASnake::SetDesiredDirection(FVector2D NewDir)
{
	// Ignore zero vector
	if (NewDir.IsNearlyZero()) return;

	// Prevent 180-degree reversal against the last queued (or current) direction
	FVector2D CheckAgainst = DirQueue.IsEmpty() ? CurrentDirection : DirQueue.Last();
	if ((NewDir + CheckAgainst).IsNearlyZero()) return;

	if (DirQueue.Num() < 2) // buffer max 2 turns
	{
		DirQueue.Add(NewDir);
	}
}

// ─────────────── Acceleration ───────────────

void ASnake::StartAcceleration()
{
	if (bAccelerating) return;
	bAccelerating = true;
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	ScheduleMoveTimer(AccelMoveInterval);
}

void ASnake::StopAcceleration()
{
	if (!bAccelerating) return;
	bAccelerating = false;
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	ScheduleMoveTimer(NormalMoveInterval);
}

// ─────────────── Move Timer ───────────────

void ASnake::ScheduleMoveTimer(float Interval)
{
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ASnake::MoveStep, Interval, true);
}

// ─────────────── Core Move Logic ───────────────

void ASnake::MoveStep()
{
	if (!ManagerRef || ManagerRef->GameState != EGameState::Playing) return;

	// Consume next buffered direction
	if (!DirQueue.IsEmpty())
	{
		CurrentDirection = DirQueue[0];
		DirQueue.RemoveAt(0);
	}

	// Compute new head position with boundary wrap
	FVector2D NewHead = WrapPosition(BodyPositions[0] + CurrentDirection);

	// ── Collision check ──
	EFoodType HitFoodType;
	bool bHitFood     = ManagerRef->IsFoodAt(NewHead, HitFoodType);
	bool bHitObstacle = !bHitFood && ManagerRef->IsObstacleAt(NewHead);
	bool bHitSelf     = !bHitFood && !bHitObstacle && ManagerRef->IsSnakeBodyAt(NewHead);

	// Invisible: ignore self and obstacles (but still eat food normally)
	if (bInvisible)
	{
		bHitObstacle = false;
		bHitSelf     = false;
	}

	// Obstacle hit
	if (bHitObstacle)
	{
		if (bInvincible)
		{
			ManagerRef->DestroyObstacleAt(NewHead);
			// continue moving into that cell
		}
		else
		{
			ManagerRef->TriggerGameOver();
			return;
		}
	}

	// Self hit (Invincible does NOT protect against self)
	if (bHitSelf)
	{
		ManagerRef->TriggerGameOver();
		return;
	}

	// ── Advance body ──
	bool bGrow = bHitFood;

	// Shift body: each segment follows the one ahead
	if (!bGrow)
	{
		// Tail segment moves to previous head position (already done by shifting array)
		ASnakeSegment* TailSeg = Segments.IsEmpty() ? nullptr : Segments.Last();
		BodyPositions.Insert(NewHead, 0);
		BodyPositions.RemoveAt(BodyPositions.Num() - 1);

		// Move last segment to the new head-adjacent slot (tail follows)
		if (TailSeg)
		{
			TailSeg->SetActorLocation(GridToWorld(BodyPositions[1]));
			TailSeg->GridPosition = BodyPositions[1];
			// Re-order segments to match body
			Segments.Remove(TailSeg);
			Segments.Insert(TailSeg, 0);
		}
	}
	else
	{
		// Grow: insert new head, do NOT remove tail
		BodyPositions.Insert(NewHead, 0);
		GrowBody(); // spawns new segment at index 1 (behind head)
	}

	// Move head actor
	SetActorLocation(GridToWorld(NewHead));

	// Sync all segment positions to body array
	for (int32 i = 0; i < Segments.Num(); ++i)
	{
		if (IsValid(Segments[i]))
		{
			int32 BodyIdx = i + 1;
			if (BodyPositions.IsValidIndex(BodyIdx))
			{
				Segments[i]->SetActorLocation(GridToWorld(BodyPositions[BodyIdx]));
				Segments[i]->GridPosition = BodyPositions[BodyIdx];
			}
		}
	}

	// Update manager cell map
	ManagerRef->UpdateSnakeCells(BodyPositions);

	// Food consumed
	if (bHitFood)
	{
		ApplyFoodEffect(HitFoodType);
		ManagerRef->HandleFoodEaten(NewHead);
	}
}

// ─────────────── Helpers ───────────────

FVector2D ASnake::WrapPosition(FVector2D Pos) const
{
	int32 W = ManagerRef->GridWidth;
	int32 H = ManagerRef->GridHeight;
	int32 X = FMath::FloorToInt(Pos.X);
	int32 Y = FMath::FloorToInt(Pos.Y);
	X = ((X % W) + W) % W;
	Y = ((Y % H) + H) % H;
	return FVector2D((float)X, (float)Y);
}

void ASnake::GrowBody()
{
	if (!ManagerRef || !ManagerRef->SegmentClass) return;

	int32 TailIdx = BodyPositions.Num() - 1;
	FVector SpawnPos = GridToWorld(BodyPositions[TailIdx]);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASnakeSegment* Seg = GetWorld()->SpawnActor<ASnakeSegment>(
		ManagerRef->SegmentClass, SpawnPos, FRotator::ZeroRotator, Params);

	if (Seg)
	{
		Seg->GridPosition = BodyPositions[TailIdx];
		Segments.Add(Seg);
	}
}

void ASnake::ApplyFoodEffect(EFoodType Type)
{
	if (Type == EFoodType::Invisible)
	{
		bInvisible = true;
		GetWorldTimerManager().SetTimer(InvisTimerHandle,  this, &ASnake::EndInvisible,  10.f, false);
	}
	else if (Type == EFoodType::Invincible)
	{
		bInvincible = true;
		GetWorldTimerManager().SetTimer(InvincTimerHandle, this, &ASnake::EndInvincible, 10.f, false);
	}
}

FVector ASnake::GridToWorld(FVector2D GridPos) const
{
	float CellSize = ManagerRef ? ManagerRef->CellSize : 100.f;
	return FVector(GridPos.X * CellSize, GridPos.Y * CellSize, 0.f);
}

void ASnake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
