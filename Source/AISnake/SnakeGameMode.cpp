#include "SnakeGameMode.h"
#include "SnakePlayerController.h"
#include "SnakeHUD.h"

ASnakeGameMode::ASnakeGameMode()
{
	PlayerControllerClass = ASnakePlayerController::StaticClass();
	HUDClass              = ASnakeHUD::StaticClass();
}
