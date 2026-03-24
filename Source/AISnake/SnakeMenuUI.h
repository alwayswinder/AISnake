#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeMenuUI.generated.h"

class UButton;
class UTextBlock;

// Delegate fired when the player clicks Start
DECLARE_MULTICAST_DELEGATE(FOnStartGameRequested);

UCLASS()
class AISNAKE_API USnakeMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LastScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LeaderboardText;

	FOnStartGameRequested OnStartGameRequested;

	void UpdateScoreDisplay(int32 LastScore, const TArray<int32>& HighScores);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleStartButtonClicked();
};
