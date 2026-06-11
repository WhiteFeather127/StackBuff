#include <Syringe.h>

#include "StackManager.h"

// ============================================================
// Syringe 主循环钩子 — 每帧调用 StackManager::Update()
//
// 主要重建路径：AfterLoadGame → TryRebuild()（SEH 保护直接尝试）
// 本钩子仅作为兜底：当 TryRebuild 因 WIC 未就绪而失败时，
// 下个游戏帧由 Update() 检查 m_pendingRebuild 并触发重建。
//
// 正常情况（TryRebuild 成功）下 m_pendingRebuild == false，
// Update() 立即返回，无实质开销。
// ============================================================
DEFINE_HOOK(0x55D360, MainLoopHook, 5)
{
	StackManager::Get().Update();
	return 0;
}
