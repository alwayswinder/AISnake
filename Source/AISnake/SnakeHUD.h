#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SnakeHUD.generated.h"

class USnakeGameUI;
class USnakeMenuUI;

UCLASS()
class AISNAKE_API ASnakeHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	/** Show the main/game-over menu, populate scores. */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowMenu(const TArray<int32>& TopScores, int32 LastScore);

	/** Hide menu, show in-game score bar. */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowGameUI();

	UFUNCTION(BlueprintCallable, Category="HUD")
	void UpdateScore(int32 Score);

	/** Called by SnakeMenuUI's Start button. */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void OnStartGame();

	// Set in Blueprint subclass BP_SnakeHUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<USnakeGameUI> GameUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<USnakeMenuUI> MenuUIClass;

private:
	UPROPERTY()
	USnakeGameUI* GameUIInstance = nullptr;

	UPROPERTY()
	USnakeMenuUI* MenuUIInstance = nullptr;
};
