#include <EC.h>
#include <EC.Listener.h>
#include <EC.LoadSave.h>
#include <EC.GameClass.h>
#include <EC.Stream.h>
#include <WIC.h>
#include <Syringe.h>

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
// 我们也通过 ECListener::Listen_AfterLoadGame 做额外恢复
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
		StackManager::Get().OnPointerGotInvalid(pObject, bRemoved);
	}

	virtual void FinalSwizzle() override
	{
		StackManager::Get().FinalSwizzle();
	}

	virtual void Update() override
	{
		// 纯存档辅助对象，不需要每帧更新
	}
};

// 全局实例 — 构造时自动注册到 EC 框架的 GameClassList
static StackSaveLoadHandler g_saveLoadHandler;

// ============================================================
// StackBuffsECInit — .inj 文件中定义的钩子函数
// 在 IHCore 的 ECInitialize 入口 (0x6BC0CD) 处触发，
// 此时 IHLibList 已就绪，可以安全调用 ECInitLibrary。
// ============================================================
extern "C" __declspec(dllexport) DWORD __cdecl StackBuffsECInit(REGISTERS* R)
{
	if (Init::Initialize())
	{
		InitDependency dep_WIC{
			"SIWinterIsComing",
			DoNotCheckVersion,
			WIC_LIBRARY_VERSION,
			true
		};

		std::function<void()> fnFirst = []()
		{
			// 注册对局结束清理回调
			ECListener::Listen_ClearScenario([]()
			{
				StackManager::Get().ClearAll();
			});
		};

		std::function<void()> fnOrdered = []()
		{
			try
			{
				SIClassManager::RegisterBuff<StackPushBuffClass>("StackPush");
				SIClassManager::RegisterBuff<StackTopBuffClass>("StackTopBuff");
			}
			catch (SIException& e)
			{
				// 注册失败时记录错误，防止崩溃
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

	return 0;
}

// ============================================================
// DllMain — 空实现，初始化在钩子函数中完成
// ============================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	return TRUE;
}
