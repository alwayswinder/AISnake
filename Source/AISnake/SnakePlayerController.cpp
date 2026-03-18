#include "SnakePlayerController.h"
#include "Snake.h"

ASnakePlayerController::ASnakePlayerController()
{
	bShowMouseCursor = false;
}

void ASnakePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SnakeRef = Cast<ASnake>(InPawn);
}

void ASnakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Pressed
	InputComponent->BindAction("MoveUp",    IE_Pressed, this, &ASnakePlayerController::OnMoveUpPressed);
	InputComponent->BindAction("MoveDown",  IE_Pressed, this, &ASnakePlayerController::OnMoveDownPressed);
	InputComponent->BindAction("MoveLeft",  IE_Pressed, this, &ASnakePlayerController::OnMoveLeftPressed);
	InputComponent->BindAction("MoveRight", IE_Pressed, this, &ASnakePlayerController::OnMoveRightPressed);

	// Released
	InputComponent->BindAction("MoveUp",    IE_Released, this, &ASnakePlayerController::OnMoveReleased);
	InputComponent->BindAction("MoveDown",  IE_Released, this, &ASnakePlayerController::OnMoveReleased);
	InputComponent->BindAction("MoveLeft",  IE_Released, this, &ASnakePlayerController::OnMoveReleased);
	InputComponent->BindAction("MoveRight", IE_Released, this, &ASnakePlayerController::OnMoveReleased);
}

void ASnakePlayerController::OnMoveUpPressed()    { HandleDirectionPressed(FVector2D( 0.f,  1.f)); }
void ASnakePlayerController::OnMoveDownPressed()  { HandleDirectionPressed(FVector2D( 0.f, -1.f)); }
void ASnakePlayerController::OnMoveLeftPressed()  { HandleDirectionPressed(FVector2D(-1.f,  0.f)); }
void ASnakePlayerController::OnMoveRightPressed() { HandleDirectionPressed(FVector2D( 1.f,  0.f)); }

void ASnakePlayerController::HandleDirectionPressed(FVector2D Dir)
{
	if (SnakeRef) SnakeRef->SetDesiredDirection(Dir);

	// (Re)start hold timer for acceleration
	HeldDirection = Dir;
	GetWorldTimerManager().ClearTimer(HoldTimerHandle);
	GetWorldTimerManager().SetTimer(HoldTimerHandle, this, &ASnakePlayerController::OnHoldFired, 0.5f, false);
}

void ASnakePlayerController::OnMoveReleased()
{
	GetWorldTimerManager().ClearTimer(HoldTimerHandle);
	if (SnakeRef) SnakeRef->StopAcceleration();
}

void ASnakePlayerController::OnHoldFired()
{
	if (SnakeRef) SnakeRef->StartAcceleration();
}
