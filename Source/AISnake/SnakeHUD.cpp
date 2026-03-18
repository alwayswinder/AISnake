#include "SnakeHUD.h"
#include "SnakeGameUI.h"
#include "SnakeMenuUI.h"
#include "SnakeManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void ASnakeHUD::BeginPlay()
{
	Super::BeginPlay();

	// Pre-create both widgets (they start hidden)
	if (GameUIClass)
	{
		GameUIInstance = CreateWidget<USnakeGameUI>(GetOwningPlayerController(), GameUIClass);
		if (GameUIInstance) GameUIInstance->AddToViewport();
	}
	if (MenuUIClass)
	{
		MenuUIInstance = CreateWidget<USnakeMenuUI>(GetOwningPlayerController(), MenuUIClass);
		if (MenuUIInstance) MenuUIInstance->AddToViewport();
	}

	// Hide game UI initially
	if (GameUIInstance) GameUIInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (MenuUIInstance) MenuUIInstance->SetVisibility(ESlateVisibility::Collapsed);

	// Find manager and show menu
	if (ASnakeManager* Manager = Cast<ASnakeManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeManager::StaticClass())))
	{
		ShowMenu(Manager->LoadHighScores(), 0);
	}
}

void ASnakeHUD::ShowMenu(const TArray<int32>& TopScores, int32 LastScore)
{
	if (GameUIInstance) GameUIInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (MenuUIInstance)
	{
		MenuUIInstance->SetVisibility(ESlateVisibility::Visible);
		MenuUIInstance->UpdateScores(TopScores, LastScore);
	}

	// Re-enable cursor for menu navigation
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		if (MenuUIInstance)
		{
			InputMode.SetWidgetToFocus(MenuUIInstance->TakeWidget());
		}
		PC->SetInputMode(InputMode);
	}
}

void ASnakeHUD::ShowGameUI()
{
	if (MenuUIInstance) MenuUIInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (GameUIInstance)
	{
		GameUIInstance->SetVisibility(ESlateVisibility::Visible);
		GameUIInstance->UpdateScore(0);
	}

	// Switch to game input
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ASnakeHUD::UpdateScore(int32 Score)
{
	if (GameUIInstance) GameUIInstance->UpdateScore(Score);
}

void ASnakeHUD::OnStartGame()
{
	if (ASnakeManager* Manager = Cast<ASnakeManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASnakeManager::StaticClass())))
	{
		Manager->StartGame();
	}
}
