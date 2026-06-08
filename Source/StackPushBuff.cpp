#include "StackPushBuff.h"
#include "StackManager.h"

// ============================================================
// EffectDataInit - 读取栈 ID 配置
// ============================================================
void StackPushBuffClass::EffectDataInit()
{
	SIExtraCode_A = Type->SIEffect_ExtraCodeA;
}

// ============================================================
// OnEnterState_Active - 激发时将单位入栈
// ============================================================
void StackPushBuffClass::OnEnterState_Active()
{
	TechnoClass* owner = GetOwnerTechno();
	HouseClass* house = GetActiveOwnerHouse();
	if (owner && house)
	{
		StackManager::Get().Push(SIExtraCode_A, house, owner);
	}
}

// ============================================================
// OnEnterState_After - 结束时将单位出栈
// ============================================================
void StackPushBuffClass::OnEnterState_After()
{
	TechnoClass* owner = GetOwnerTechno();
	HouseClass* house = GetActiveOwnerHouse();
	if (owner && house)
	{
		StackManager::Get().RemoveUnit(SIExtraCode_A, house, owner);
	}
}

// ============================================================
// EffectTriggerPointerGotInvalid - 指针失效时清理栈
// ============================================================
void StackPushBuffClass::EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed)
{
	StackManager::Get().OnPointerGotInvalid(ptr, removed);
}
