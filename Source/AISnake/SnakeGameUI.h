#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeGameUI.generated.h"

class UTextBlock;

UCLASS()
class AISNAKE_API USnakeGameUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// Must match the widget name in the Blueprint (WBP_SnakeGameUI)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	void UpdateScore(int32 Score);
};
