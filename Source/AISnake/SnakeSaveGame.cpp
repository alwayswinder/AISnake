#include "SnakeSaveGame.h"
#include "Algo/Sort.h"

const FString USnakeSaveGame::SlotName  = TEXT("SnakeHighScores");
const int32   USnakeSaveGame::UserIndex = 0;
const int32   USnakeSaveGame::MaxScores = 10;

void USnakeSaveGame::AddScore(int32 NewScore)
{
	TopScores.Add(NewScore);
	TopScores.Sort([](const int32& A, const int32& B){ return A > B; });
	if (TopScores.Num() > MaxScores)
	{
		TopScores.SetNum(MaxScores);
	}
}
