#include "NG2GameMode.h"

#include "NG2/Player/NG2PlayerController.h"

ANG2GameMode::ANG2GameMode()
{
	PlayerControllerClass = ANG2PlayerController::StaticClass();
}
