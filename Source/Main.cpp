#include <EC.h>
#include <EC.Listener.h>
#include <EC.LoadSave.h>
#include <EC.GameClass.h>
#include <EC.Stream.h>
#include <WIC.h>

#include "Debug.h"
#include "StackManager.h"
#include "StackPushBuff.h"
#include "StackTopBuff.h"

// Init::Initialize() 由 SyringeIH 运行时提供，声明在 IH.Loader.cpp 中
// 此处补充声明以便编译
namespace Init {
	bool Initialize();
}

// ============================================================
// SyringeForceLoad — 强制 SyringeIH 加载本 DLL
// ============================================================
extern "C" __declspec(dllexport) void SyringeForceLoad(void) {}

// ============================================================
// ECGameClass_ 派生类 — 自动接入 EC 框架的 Save/Load 系统
// 当游戏存档时, ECGameClass_SaveGame 会遍历所有注册的
// ECGameClass_ 实例并调用其 SaveGame
// 当游戏读档时同理触发 LoadGame → FinalSwizzle
// 
// 重建策略：
//   AfterLoadGame 阶段用 SEH 保护直接调用 TryRebuild()，
//   若 WIC 可用则立即完成重建；若发生访问违例则
//   由主循环 MainLoop 钩子（0x55D360）兜底触发 Update()。
// ============================================================

class StackSaveLoadHandler : public ECGameClass_
{
public:
	virtual void SaveGame(ECStreamWriter& stream) override
	{
		StackManager::Get().SaveToStream(stream);
	}

	virtual void LoadGame(ECStreamReader& stream) override
	{
		StackManager::Get().LoadFromStream(stream);
	}

	virtual size_t GetSizeMax() override
	{
		return 0; // 由 EC 框架管理大小
	}

	virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) override
	{
		// StackPushBuff 已通过 OnEnterState_Remove 自行出栈，无需额外处理
	}

	virtual void FinalSwizzle() override
	{
		StackManager::Get().FinalSwizzle();
	}

	virtual void Update() override
	{
		// EC 框架不会实际调用此函数，重建由 Hooks.MainLoop.cpp 兜底
	}
};

// ============================================================
// DllMain — 在 DLL 加载时直接调用 ECInitLibrary。
// ECInitLibrary 内部会调用 Init::Initialize() 注册到 IHLibList，
// EC 框架稍后会自动回调 SDK 中的 MyInit，进而执行 fnFirst/fnOrdered。
//
// 优点：无需 .inj 钩子文件，无静态初始化顺序问题。
// ============================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		InitDependency dep_WIC{
			"SIWinterIsComing",
			DoNotCheckVersion,
			WIC_LIBRARY_VERSION,
			true
		};

		std::function<void()> fnFirst = []()
		{
			DEBUG_LOG("[Main] Debug system ready\n");

			// 延迟创建 SaveLoadHandler，确保 GameClassList 已初始化
			static StackSaveLoadHandler g_saveLoadHandler;

			// 注册对局结束清理回调
			ECListener::Listen_ClearScenario([]()
			{
				DEBUG_LOG("[Main] ClearScenario -> clearing all stacks\n");
				StackManager::Get().ClearAll();
			});

			// 注册读档完成后的栈重建
			// AfterLoadGame 阶段 WIC 已可用（HasWIC() == true），
			// 用 SEH 保护直接尝试 RebuildFromUIDs。
			// 若 WIC 内部尚未完全就绪（例如 Swizzle 未生效），
			// SEH 会捕获访问违例，重建延后到主循环兜底。
			ECListener::Listen_AfterLoadGame([]()
			{
				DEBUG_LOG("[Main] AfterLoadGame -> attempting immediate rebuild\n");
				StackManager::Get().TryRebuild();
			});
		};

		std::function<void()> fnOrdered = []()
		{
			try
			{
				SIClassManager::RegisterBuff<StackPushBuffClass>("StackPush");
				DEBUG_LOG("[Main] Registered StackPush buff\n");

				SIClassManager::RegisterBuff<StackTopBuffClass>("StackTop");
				DEBUG_LOG("[Main] Registered StackTop buff\n");
			}
			catch (SIException& e)
			{
				// 注册失败时记录错误，防止崩溃
				DEBUG_LOG("[Main] ERROR: RegisterBuff failed: %s\n", e.what());
			}
		};

		ECInitLibrary(
			"StackBuffs",
			1, 1,
			(UTF8_CString)u8"WIC Stack Buff System - Push & TopOfStack",
			fnFirst,
			fnOrdered,
			{ dep_WIC }
		);
	}
	return TRUE;
}
