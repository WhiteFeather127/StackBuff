#pragma once

/*
 * Debug.h — StackBuffs 自带的调试日志系统
 *
 * 原理：
 *   内部调用 YR 引擎 0x4068E0 处的 _Log 函数（游戏自带的 printf 风格日志输出）
 *   不需要额外的文件操作或外部工具
 *
 * 用法：
 *   Debug::Log("format %d %s\n", value, str);
 *   Debug::Log("Something happened\n");
 *
 * 运行时开关：
 *   Debug::SetEnabled(false);  // 关闭调试输出
 *   Debug::SetEnabled(true);   // 重新开启
 */

namespace Debug
{
	// 设置调试输出启用/禁用（默认启用）
	void SetEnabled(bool enabled);

	// 核心日志函数（printf 风格）
	void Log(const char* pFormat, ...);
}

// ============================================================
// DEBUG_LOG 宏 — 始终展开为 Debug::Log
// -------------------------------------------------------
// 运行时可通过 Debug::SetEnabled(false) 关闭
// 如果需要编译期完全移除，定义 STACKBUFFS_NO_DEBUG
// ============================================================
#ifndef STACKBUFFS_NO_DEBUG
	#define DEBUG_LOG(...)	Debug::Log(__VA_ARGS__)
#else
	#define DEBUG_LOG(...)	((void)0)
#endif
