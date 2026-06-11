#include <Syringe.h>

#include "StackManager.h"

// ============================================================
// Syringe 主循环钩子 — 每帧调用 StackManager::Update()
// 读档后的重建在 AfterLoadGame 设置标记后，由下个游戏帧触发
// 参考 Phobos 的 Misc/Hooks.*.cpp 模式
// ============================================================
DEFINE_HOOK(0x55D360, MainLoopHook, 5)
{
	StackManager::Get().Update();
	return 0;
}
