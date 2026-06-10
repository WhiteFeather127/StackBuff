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
// EffectAI - 每帧 AI
// 读档后 WIC 已完全就绪，在此触发栈重建（延迟到游戏正常运行后）
// ============================================================
void StackPushBuffClass::EffectAI(SIBuffClass_EffectData* 生效数据)
{
	auto& mgr = StackManager::Get();
	if (mgr.NeedsRebuild())
	{
		mgr.RebuildFromUIDs();
	}
}

// ============================================================
// EffectTriggerPointerGotInvalid - 指针失效通知
// 注意：此回调仅通知 buff 某个外部指针已失效，buff 实例本身仍存活。
// 不应在此处从 StackManager 移除 buff，否则栈会被错误清空。
// 移除操作由 OnEnterState_After / OnEnterState_Remove 负责。
// ============================================================
void StackPushBuffClass::EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed)
{
}
