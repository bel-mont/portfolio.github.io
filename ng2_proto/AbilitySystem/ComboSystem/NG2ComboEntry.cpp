#include "NG2ComboEntry.h"

FName FNG2ComboEntry::GetComboKey() const
{
	FString Key;
	
	for (const auto& [Button, bForward, bHold] : Sequence)
	{
		if (bForward) Key += TEXT(">");
		if (bHold) Key += TEXT("H");
		switch (Button)
		{
		case EAttackButton::Light: Key += TEXT("X"); break;
		case EAttackButton::Heavy: Key += TEXT("Y"); break;
		case EAttackButton::Jump:  Key += TEXT("J"); break;
		case EAttackButton::JumpForward:
			break;
		}
		Key += TEXT(" ");
	}
	Key.TrimEndInline();
	return FName(*Key);
}