#include "SnakeHUD.h"
#include "Blueprint/UserWidget.h"

void ASnakeHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (MenuWidgetClass)
	{
		MenuWidget = CreateWidget<USnakeMenuUI>(PC, MenuWidgetClass);
		if (MenuWidget)
		{
			MenuWidget->AddToViewport();
			// Bind once — delegate is a C++ multicast, no duplicate risk
			MenuWidget->OnStartGameRequested.AddUObject(this, &ASnakeHUD::HandleStartGameRequested);
			MenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (GameWidgetClass)
	{
		GameWidget = CreateWidget<USnakeGameUI>(PC, GameWidgetClass);
		if (GameWidget)
		{
			GameWidget->AddToViewport();
			GameWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ASnakeHUD::SetStartGameCallback(FSimpleDelegate Callback)
{
	StartGameCallback = Callback;
}

void ASnakeHUD::HandleStartGameRequested()
{
	if (StartGameCallback.IsBound())
	{
		StartGameCallback.Execute();
	}
}

void ASnakeHUD::ShowMenu(int32 LastScore, const TArray<int32>& HighScores)
{
	if (GameWidget) GameWidget->SetVisibility(ESlateVisibility::Hidden);

	if (MenuWidget)
	{
		MenuWidget->UpdateScoreDisplay(LastScore, HighScores);
		MenuWidget->SetVisibility(ESlateVisibility::Visible);
	}

	APlayerController* PC = GetOwningPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->SetShowMouseCursor(true);
	}
}

void ASnakeHUD::ShowGame()
{
	if (MenuWidget) MenuWidget->SetVisibility(ESlateVisibility::Hidden);

	if (GameWidget)
	{
		GameWidget->UpdateScore(0);
		GameWidget->SetVisibility(ESlateVisibility::Visible);
	}

	APlayerController* PC = GetOwningPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

void ASnakeHUD::UpdateGameScore(int32 Score)
{
	if (GameWidget)
	{
		GameWidget->UpdateScore(Score);
	}
}
