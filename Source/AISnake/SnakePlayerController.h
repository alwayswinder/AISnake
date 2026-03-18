#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnakePlayerController.generated.h"

class ASnake;

UCLASS()
class AISNAKE_API ASnakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASnakePlayerController();

	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY()
	ASnake* SnakeRef = nullptr;

	// Hold-for-acceleration
	FTimerHandle HoldTimerHandle;
	FVector2D    HeldDirection;

	// Per-direction pressed handlers
	void OnMoveUpPressed();
	void OnMoveDownPressed();
	void OnMoveLeftPressed();
	void OnMoveRightPressed();

	// Released — same for all (any release stops acceleration)
	void OnMoveReleased();

	void HandleDirectionPressed(FVector2D Dir);
	void OnHoldFired();
};
