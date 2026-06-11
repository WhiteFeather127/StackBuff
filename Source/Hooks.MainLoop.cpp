#include <Syringe.h>

#include "StackManager.h"

// ============================================================
// 加载完成钩子 — 读档/新对局完成后触发（仅一次，非每帧）
//
// 参考 Phobos: LoadGame_UnsetFlag (0x67E68A)
// 和 ScenarioClass_Start_Optimizations (0x683E7F)
// 这两个钩子在主循环开始之前、加载完成之后执行，
// 此时 WIC 内部数据结构已就绪，可安全调用 WIC API。
// ============================================================

// 读档完成后触发
DEFINE_HOOK(0x67E68A, LoadGame_Complete, 5)
{
	StackManager::Get().Update();
	return 0;
}

// 新对局开始时触发（清除栈，确保空栈起始）
DEFINE_HOOK(0x683E7F, ScenarioClass_Start, 7)
{
	StackManager::Get().ClearAll();
	return 0;
}
