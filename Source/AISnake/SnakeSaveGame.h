#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SnakeSaveGame.generated.h"

UCLASS()
class AISNAKE_API USnakeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;
	static const int32 UserIndex;
	static const int32 MaxScores;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Score")
	TArray<int32> TopScores;

	/** Insert score and keep only top MaxScores entries (descending). */
	void AddScore(int32 NewScore);
};
