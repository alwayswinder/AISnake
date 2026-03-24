#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SnakeGameUI.h"
#include "SnakeMenuUI.h"
#include "SnakeHUD.generated.h"

UCLASS()
class AISNAKE_API ASnakeHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<USnakeMenuUI> MenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<USnakeGameUI> GameWidgetClass;

	// Called by SnakeManager to inject the start-game handler
	void SetStartGameCallback(FSimpleDelegate Callback);

	void ShowMenu(int32 LastScore, const TArray<int32>& HighScores);
	void ShowGame();
	void UpdateGameScore(int32 Score);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	USnakeMenuUI* MenuWidget = nullptr;

	UPROPERTY()
	USnakeGameUI* GameWidget = nullptr;

	FSimpleDelegate StartGameCallback;

	void HandleStartGameRequested();
};
