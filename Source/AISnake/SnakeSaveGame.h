#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SnakeSaveGame.generated.h"

UCLASS()
class AISNAKE_API USnakeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	TArray<int32> HighScores;

	// Inserts score, sorts descending, keeps top 10.
	void AddScore(int32 Score);
};
