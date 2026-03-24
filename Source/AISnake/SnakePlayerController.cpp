#include "SnakePlayerController.h"
#include "Snake.h"

ASnakePlayerController::ASnakePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASnakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("MoveUp",    IE_Pressed,  this, &ASnakePlayerController::OnMoveUpPressed);
	InputComponent->BindAction("MoveDown",  IE_Pressed,  this, &ASnakePlayerController::OnMoveDownPressed);
	InputComponent->BindAction("MoveLeft",  IE_Pressed,  this, &ASnakePlayerController::OnMoveLeftPressed);
	InputComponent->BindAction("MoveRight", IE_Pressed,  this, &ASnakePlayerController::OnMoveRightPressed);

	// All releases go to the same handler
	InputComponent->BindAction("MoveUp",    IE_Released, this, &ASnakePlayerController::OnDirectionReleased);
	InputComponent->BindAction("MoveDown",  IE_Released, this, &ASnakePlayerController::OnDirectionReleased);
	InputComponent->BindAction("MoveLeft",  IE_Released, this, &ASnakePlayerController::OnDirectionReleased);
	InputComponent->BindAction("MoveRight", IE_Released, this, &ASnakePlayerController::OnDirectionReleased);
}

void ASnakePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ASnake* Snake = GetSnake();
	if (!Snake) return;

	if (DirectionsPressedCount > 0 && !Snake->IsAccelerating())
	{
		DirectionHoldTime += DeltaTime;
		if (DirectionHoldTime >= 0.5f)
		{
			Snake->SetAccelerating(true);
		}
	}
}

// Screen-space directions: top-down camera (Pitch=-90, Yaw=0)
// Screen UP  = World +X (FIntPoint(1,0))
// Screen DOWN = World -X (FIntPoint(-1,0))
// Screen LEFT = World -Y (FIntPoint(0,-1))
// Screen RIGHT = World +Y (FIntPoint(0,1))

void ASnakePlayerController::OnMoveUpPressed()
{
	DirectionsPressedCount++;
	if (DirectionsPressedCount == 1) DirectionHoldTime = 0.0f;
	SendDirection(FIntPoint(1, 0));
}

void ASnakePlayerController::OnMoveDownPressed()
{
	DirectionsPressedCount++;
	if (DirectionsPressedCount == 1) DirectionHoldTime = 0.0f;
	SendDirection(FIntPoint(-1, 0));
}

void ASnakePlayerController::OnMoveLeftPressed()
{
	DirectionsPressedCount++;
	if (DirectionsPressedCount == 1) DirectionHoldTime = 0.0f;
	SendDirection(FIntPoint(0, -1));
}

void ASnakePlayerController::OnMoveRightPressed()
{
	DirectionsPressedCount++;
	if (DirectionsPressedCount == 1) DirectionHoldTime = 0.0f;
	SendDirection(FIntPoint(0, 1));
}

void ASnakePlayerController::OnDirectionReleased()
{
	DirectionsPressedCount = FMath::Max(0, DirectionsPressedCount - 1);
	if (DirectionsPressedCount == 0)
	{
		ASnake* Snake = GetSnake();
		if (Snake) Snake->SetAccelerating(false);
		DirectionHoldTime = 0.0f;
	}
}

ASnake* ASnakePlayerController::GetSnake() const
{
	return Cast<ASnake>(GetPawn());
}

void ASnakePlayerController::SendDirection(FIntPoint Dir)
{
	ASnake* Snake = GetSnake();
	if (Snake) Snake->RequestDirection(Dir);
}
