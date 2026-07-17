#include "NG2GameplayTags.h"

namespace NG2GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability_Jump, "Ability.Jump")
	UE_DEFINE_GAMEPLAY_TAG(Ability_AttackLight, "Ability.Attack.Light")
	UE_DEFINE_GAMEPLAY_TAG(Ability_AttackHeavy, "Ability.Attack.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(Ability_AttackHeavyHold, "Ability.Attack.HeavyHold")
	UE_DEFINE_GAMEPLAY_TAG(Ability_MeleeExecution, "Ability.MeleeExecution")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Jumping, "State.Jumping")
	UE_DEFINE_GAMEPLAY_TAG(State_Landing, "State.Landing") // May have to remove this.
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking")
	UE_DEFINE_GAMEPLAY_TAG(State_AirAttacking, "State.AirAttacking")
	UE_DEFINE_GAMEPLAY_TAG(State_Combo_AttackWindow, "State.Combo.AttackWindow")
	UE_DEFINE_GAMEPLAY_TAG(State_Combo_InputWindow, "State.Combo.InputWindow")
	UE_DEFINE_GAMEPLAY_TAG(State_Staggered, "State.Staggered")
	UE_DEFINE_GAMEPLAY_TAG(State_KnockedDown, "State.KnockedDown")
	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible")
	UE_DEFINE_GAMEPLAY_TAG(State_MovementBlocked, "State.Movement.Blocked")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Chain, "Event.Combo.Chain")
	UE_DEFINE_GAMEPLAY_TAG(Event_Landed, "Event.Landed")
}
