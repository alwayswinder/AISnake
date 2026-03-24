#include "Snake.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASnake::ASnake()
{
	PrimaryActorTick.bCanEverTick = true;

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	RootComponent = HeadMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		HeadMesh->SetStaticMesh(CubeMesh.Object);
	}
	HeadMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.5f));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = false;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ASnake::Initialize(FIntPoint StartPos, FIntPoint StartDir,
                        int32 InGridCount, float InCellSize,
                        TSubclassOf<ASnakeSegment> InSegmentClass)
{
	GridCount    = InGridCount;
	GridCellSize = InCellSize;
	SegmentClass = InSegmentClass;

	BodyPositions.Empty();
	BodyPositions.Add(StartPos);
	CurrentDirection = StartDir;
	bHasPendingDir   = false;
	MoveTimer        = 0.0f;
	bAccelerating    = false;
	bIsInvisible     = false;
	bIsInvincible    = false;

	SetActorLocation(GridToWorld(StartPos));
	ApplyCurrentEffectMaterial();
}

void ASnake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick effect timers
	if (bIsInvisible)
	{
		InvisibleTimer -= DeltaTime;
		if (InvisibleTimer <= 0.0f)
		{
			bIsInvisible = false;
			ApplyCurrentEffectMaterial();
		}
	}
	if (bIsInvincible)
	{
		InvincibleTimer -= DeltaTime;
		if (InvincibleTimer <= 0.0f)
		{
			bIsInvincible = false;
			ApplyCurrentEffectMaterial();
		}
	}

	// Grid movement tick
	float Interval = bAccelerating ? FastInterval : NormalInterval;
	MoveTimer += DeltaTime;
	if (MoveTimer >= Interval)
	{
		MoveTimer -= Interval;
		DoMove();
	}
}

void ASnake::DoMove()
{
	// Consume queued direction
	if (bHasPendingDir)
	{
		CurrentDirection = PendingDirection;
		bHasPendingDir   = false;
	}

	// New head position with boundary wrap
	FIntPoint NewHead = BodyPositions[0] + CurrentDirection;
	NewHead.X = (NewHead.X % GridCount + GridCount) % GridCount;
	NewHead.Y = (NewHead.Y % GridCount + GridCount) % GridCount;

	// Shift body: each cell takes the previous cell's position
	for (int32 i = BodyPositions.Num() - 1; i > 0; i--)
	{
		BodyPositions[i] = BodyPositions[i - 1];
	}
	BodyPositions[0] = NewHead;

	// Update visual positions
	SetActorLocation(GridToWorld(BodyPositions[0]));
	for (int32 i = 0; i < Segments.Num(); i++)
	{
		if (Segments[i] && i + 1 < BodyPositions.Num())
		{
			Segments[i]->SetActorLocation(GridToWorld(BodyPositions[i + 1]));
		}
	}

	OnSnakeMoved.Broadcast();
}

void ASnake::RequestDirection(FIntPoint NewDir)
{
	// Anti-reverse: cannot go opposite to current direction
	if (NewDir == FIntPoint(-CurrentDirection.X, -CurrentDirection.Y)) return;

	// Also check against pending direction to prevent double-reverse in one tick window
	if (bHasPendingDir && NewDir == FIntPoint(-PendingDirection.X, -PendingDirection.Y)) return;

	PendingDirection = NewDir;
	bHasPendingDir   = true;
}

void ASnake::SetAccelerating(bool bAccel)
{
	bAccelerating = bAccel;
}

void ASnake::AddSegment()
{
	if (!SegmentClass) return;

	// New segment spawns at the tail's current position
	FIntPoint TailPos = BodyPositions.Last();
	ASnakeSegment* Seg = GetWorld()->SpawnActor<ASnakeSegment>(
		SegmentClass, GridToWorld(TailPos), FRotator::ZeroRotator);

	if (Seg)
	{
		Segments.Add(Seg);
		BodyPositions.Add(TailPos);  // will be updated next move
		// Sync the new segment's material to the current effect
		ApplyCurrentEffectMaterial();
	}
}

void ASnake::ApplyInvisibleEffect(float Duration)
{
	bIsInvisible   = true;
	InvisibleTimer = Duration;
	bIsInvincible  = false;  // effects are mutually exclusive visually
	ApplyCurrentEffectMaterial();
}

void ASnake::ApplyInvincibleEffect(float Duration)
{
	bIsInvincible   = true;
	InvincibleTimer = Duration;
	bIsInvisible    = false;
	ApplyCurrentEffectMaterial();
}

void ASnake::ApplyCurrentEffectMaterial()
{
	UMaterialInterface* Mat = NormalMaterial;
	if (bIsInvisible && InvisibleMaterial)
	{
		Mat = InvisibleMaterial;
	}
	else if (bIsInvincible && InvincibleMaterial)
	{
		Mat = InvincibleMaterial;
	}

	if (!Mat) return;

	HeadMesh->SetMaterial(0, Mat);
	for (ASnakeSegment* Seg : Segments)
	{
		if (Seg)
		{
			Seg->SetSegmentMaterial(Mat);
		}
	}
}

void ASnake::Cleanup()
{
	for (ASnakeSegment* Seg : Segments)
	{
		if (Seg) Seg->Destroy();
	}
	Segments.Empty();
	BodyPositions.Empty();
}

FIntPoint ASnake::GetHeadGridPosition() const
{
	return BodyPositions.Num() > 0 ? BodyPositions[0] : FIntPoint(0, 0);
}

TArray<FIntPoint> ASnake::GetBodyGridPositions() const
{
	return BodyPositions;
}

FVector ASnake::GridToWorld(FIntPoint GridPos) const
{
	float HalfSize = GridCount * GridCellSize * 0.5f;
	float X = GridPos.X * GridCellSize - HalfSize + GridCellSize * 0.5f;
	float Y = GridPos.Y * GridCellSize - HalfSize + GridCellSize * 0.5f;
	return FVector(X, Y, 50.0f);
}

void ASnake::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Input is handled by SnakePlayerController
}

