#include "SnakeMenuUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USnakeMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind only once — in NativeConstruct — to prevent duplicate binding on restart
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &USnakeMenuUI::HandleStartButtonClicked);
	}
}

void USnakeMenuUI::HandleStartButtonClicked()
{
	OnStartGameRequested.Broadcast();
}

void USnakeMenuUI::UpdateScoreDisplay(int32 LastScore, const TArray<int32>& HighScores)
{
	if (LastScoreText)
	{
		FString Txt = LastScore > 0
			? FString::Printf(TEXT("Last Score: %d"), LastScore)
			: TEXT("Press Start to Play");
		LastScoreText->SetText(FText::FromString(Txt));
	}

	if (LeaderboardText)
	{
		FString Board = TEXT("TOP 10\n");
		for (int32 i = 0; i < HighScores.Num(); i++)
		{
			Board += FString::Printf(TEXT("%d. %d\n"), i + 1, HighScores[i]);
		}
		if (HighScores.IsEmpty())
		{
			Board += TEXT("No scores yet");
		}
		LeaderboardText->SetText(FText::FromString(Board));
	}
}
