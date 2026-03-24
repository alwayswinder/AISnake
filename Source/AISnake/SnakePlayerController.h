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

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void SetupInputComponent() override;

private:
	// Direction pressed handlers
	void OnMoveUpPressed();
	void OnMoveDownPressed();
	void OnMoveLeftPressed();
	void OnMoveRightPressed();
	void OnDirectionReleased();

	ASnake* GetSnake() const;
	void SendDirection(FIntPoint Dir);

	// Acceleration hold tracking
	int32 DirectionsPressedCount = 0;
	float DirectionHoldTime      = 0.0f;
};
