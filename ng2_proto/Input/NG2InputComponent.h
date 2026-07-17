#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "NG2InputConfig.h"
#include "NG2InputComponent.generated.h"


struct FGameplayTag;
class UNG2InputConfig;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NG2_API UNG2InputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserClass, typename FuncType>
	void BindAbilityActions(UNG2InputConfig* InputConfig, UserClass* Object,
	                       FuncType Func);
};

template <class UserClass, typename FuncType>
	void UNG2InputComponent::BindAbilityActions(UNG2InputConfig* InputConfig,
                                           UserClass* Object, FuncType Func)
{	
	check(InputConfig);
	for (const auto& [InputAction, InputTag, TriggerEvents] : InputConfig->InputActions)
	{
		if (InputAction && InputTag.IsValid() && Func)
		{
			for (ETriggerEvent Event : TriggerEvents)
			{
				BindAction(InputAction, Event, Object, Func, InputTag);
			}
		}
	}
}
