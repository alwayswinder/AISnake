#include "SnakeMenuUI.h"
#include "SnakeHUD.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

void USnakeMenuUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &USnakeMenuUI::OnStartClicked);
	}
}

void USnakeMenuUI::UpdateScores(const TArray<int32>& TopScores, int32 LastScore)
{
	if (LastScoreText)
	{
		if (LastScore > 0)
			LastScoreText->SetText(FText::FromString(FString::Printf(TEXT("Last Score: %d"), LastScore)));
		else
			LastScoreText->SetText(FText::FromString(TEXT("Welcome!")));
	}

	if (ScoreListBox)
	{
		ScoreListBox->ClearChildren();
		for (int32 i = 0; i < TopScores.Num(); ++i)
		{
			UTextBlock* Entry = NewObject<UTextBlock>(this);
			Entry->SetText(FText::FromString(
				FString::Printf(TEXT("#%d  %d"), i + 1, TopScores[i])));
			Entry->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			ScoreListBox->AddChild(Entry);
		}
	}
}

void USnakeMenuUI::OnStartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ASnakeHUD* HUD = Cast<ASnakeHUD>(PC->GetHUD()))
		{
			HUD->OnStartGame();
		}
	}
}
