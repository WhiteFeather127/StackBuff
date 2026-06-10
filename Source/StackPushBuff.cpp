#include "StackPushBuff.h"
#include "StackManager.h"
#include "Debug.h"

// ============================================================
// EffectDataInit - 读取栈 ID 配置
// ============================================================
void StackPushBuffClass::EffectDataInit()
{
	SIExtraCode_A = Type->SIEffect_ExtraCodeA;
	DEBUG_LOG("[StackPush] %s: stackId=%d\n", __FUNCTION__, SIExtraCode_A);
}

// ============================================================
// OnEnterState_Active - 激发时将自身入栈
// ============================================================
void StackPushBuffClass::OnEnterState_Active()
{
	DEBUG_LOG("[StackPush] %s: push stackId=%d  UID=%d\n", __FUNCTION__, SIExtraCode_A, UID);
	StackManager::Get().Push(SIExtraCode_A, this);
}

// ============================================================
// OnEnterState_After - After 状态时将自身出栈（Buff_TryAfter 触发）
// ============================================================
void StackPushBuffClass::OnEnterState_After()
{
	DEBUG_LOG("[StackPush] %s: remove (After) stackId=%d  UID=%d\n", __FUNCTION__, SIExtraCode_A, UID);
	StackManager::Get().RemoveBuff(this);
}

// ============================================================
// OnEnterState_Remove - Remove 状态时将自身出栈（Buff_TryRemove 等直接移除路径）
// ============================================================
void StackPushBuffClass::OnEnterState_Remove()
{
	DEBUG_LOG("[StackPush] %s: remove (Remove) stackId=%d  UID=%d\n", __FUNCTION__, SIExtraCode_A, UID);
	StackManager::Get().RemoveBuff(this);
}

// ============================================================
// EffectTriggerPointerGotInvalid - 指针失效时清理栈（已由 OnEnterState_Remove 覆盖，保留为空防止基类误操作）
// ============================================================
void StackPushBuffClass::EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed)
{
	DEBUG_LOG("[StackPush] %s: ptr=%p  removed=%d  UID=%d\n", __FUNCTION__, (void*)ptr, removed, UID);
}
