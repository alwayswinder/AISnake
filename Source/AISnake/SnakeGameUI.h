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
	UFUNCTION(BlueprintCallable, Category="UI")
	void UpdateScore(int32 Score);

protected:
	// Must exist in the Widget Blueprint with the same name
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;
};
