#include "Debug.h"

#include <YRPPCore.h>
#include <ASMMacros.h>
#include <cstdio>
#include <cstdarg>

// ============================================================
// 游戏内部日志函数地址 0x4068E0
// 这是 YR 引擎自带的 printf 风格日志输出（stdcall）
// 输出到 Debug 输出 / 日志文件，由游戏内部管理
// ============================================================
static bool g_enabled = true;

// ============================================================
// SetEnabled - 启用/禁用调试输出
// ============================================================
void Debug::SetEnabled(bool enabled)
{
	g_enabled = enabled;
}

// ============================================================
// Log - 核心日志函数（printf 风格）
// 内部调用游戏 0x4068E0 函数输出，不需要外部文件
// ============================================================
void Debug::Log(const char* pFormat, ...)
{
	if (!g_enabled || !pFormat)
		return;

	// 格式化消息
	char msgBuf[4096];
	va_list args;
	va_start(args, pFormat);
	vsnprintf_s(msgBuf, sizeof(msgBuf), _TRUNCATE, pFormat, args);
	va_end(args);

	// 调用游戏内部日志函数输出
	// 0x4068E0 是 YR 引擎的 _Log 函数（stdcall, printf 风格）
	typedef void(__stdcall* GameLogFunc)(const char*);
	reinterpret_cast<GameLogFunc>(0x4068E0)(msgBuf);
}
