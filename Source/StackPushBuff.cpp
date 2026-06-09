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
// OnEnterState_Active - 激发时将自身入栈
// ============================================================
void StackPushBuffClass::OnEnterState_Active()
{
	StackManager::Get().Push(SIExtraCode_A, this);
}

// ============================================================
// OnEnterState_After - After 状态时将自身出栈（Buff_TryAfter 触发）
// ============================================================
void StackPushBuffClass::OnEnterState_After()
{
	StackManager::Get().RemoveBuff(this);
}

// ============================================================
// OnEnterState_Remove - Remove 状态时将自身出栈（Buff_TryRemove 等直接移除路径）
// ============================================================
void StackPushBuffClass::OnEnterState_Remove()
{
	StackManager::Get().RemoveBuff(this);
}

// ============================================================
// EffectTriggerPointerGotInvalid - 指针失效时清理栈（已由 OnEnterState_Remove 覆盖，保留为空防止基类误操作）
// ============================================================
void StackPushBuffClass::EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed)
{
}
