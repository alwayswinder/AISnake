#include "SnakeGameUI.h"
#include "Components/TextBlock.h"

void USnakeGameUI::UpdateScore(int32 Score)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), Score)));
	}
}
