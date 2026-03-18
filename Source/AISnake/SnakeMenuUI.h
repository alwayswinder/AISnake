#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeMenuUI.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;

UCLASS()
class AISNAKE_API USnakeMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by HUD to populate the score list and show this round's result. */
	UFUNCTION(BlueprintCallable, Category="UI")
	void UpdateScores(const TArray<int32>& TopScores, int32 LastScore);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	UButton* StartButton;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* LastScoreText;

	UPROPERTY(meta=(BindWidget))
	UVerticalBox* ScoreListBox;

private:
	UFUNCTION()
	void OnStartClicked();
};
