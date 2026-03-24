#include "SnakeSaveGame.h"

void USnakeSaveGame::AddScore(int32 Score)
{
	HighScores.Add(Score);
	HighScores.Sort([](const int32& A, const int32& B) { return A > B; });
	if (HighScores.Num() > 10)
	{
		HighScores.SetNum(10);
	}
}
