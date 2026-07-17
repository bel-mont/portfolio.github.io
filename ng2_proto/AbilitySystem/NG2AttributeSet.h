#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "NG2AttributeSet.generated.h"

// See the AttributeSet.h file for an explanation on this macro
// You can debug in-editor with `showdebug abilitysystem`
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class NG2_API UNG2AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
	UNG2AttributeSet();
public:
	ATTRIBUTE_ACCESSORS(UNG2AttributeSet, Health)
	UPROPERTY()
	FGameplayAttributeData Health;

	/**
	 * Serves a secondary max health bar, and it is affected by damage the player receives.
	 * It can be healed by using consumables or a blue dragon statue.
	 */
	ATTRIBUTE_ACCESSORS(UNG2AttributeSet, HealthCurrentMax)
	UPROPERTY()
	FGameplayAttributeData HealthCurrentMax;
	
	ATTRIBUTE_ACCESSORS(UNG2AttributeSet, HealthMax)
	UPROPERTY()
	FGameplayAttributeData HealthMax;
};
