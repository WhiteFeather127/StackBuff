#include <EC.h>
#include <EC.Listener.h>
#include <EC.LoadSave.h>
#include <EC.GameClass.h>
#include <EC.Stream.h>
#include <WIC.h>

#include "StackManager.h"
#include "StackPushBuff.h"
#include "StackTopBuff.h"

// Init::Initialize() 由 SyringeIH 运行时提供，声明在 IH.Loader.cpp 中
// 此处补充声明以便编译
namespace Init {
	bool Initialize();
}

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
// Callbacks
// ============================================================

void OnFirstInit()
{
	// 注册读档后恢复回调（此时 Swizzle 已完成）
	ECListener::Listen_AfterLoadGame([]()
	{
		// 如需要可在读档完成后做额外处理
	});

	// 注册对局结束清理回调
	ECListener::Listen_ClearScenario([]()
	{
		StackManager::Get().ClearAll();
	});
}

void OnOrderedInit()
{
	// 注册自定义 Buff 效果类型（此时 WIC 已可用）
	int pushId = SIClassManager::RegisterBuff<StackPushBuffClass>("StackPush");
	int topId = SIClassManager::RegisterBuff<StackTopBuffClass>("StackTopBuff");
}

// ============================================================
// DllMain
// ============================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		// 初始化 EC/Syringe 框架
		if (!Init::Initialize())
			return FALSE;

		// 声明依赖：需要 WIC（SIWinterIsComing）
		InitDependency dep_WIC{
			"SIWinterIsComing",
			DoNotCheckVersion,
			WIC_LIBRARY_VERSION,
			true // 必需依赖
		};

		// 注册本库
		std::function<void()> fnFirst = OnFirstInit;
		std::function<void()> fnOrdered = OnOrderedInit;
		ECInitLibrary(
			"MyWICBuffs",          // 库名
			1,                      // 版本
			1,                      // 最低兼容版本
			(UTF8_CString)u8"WIC Stack Buff System - Push & TopOfStack",
			fnFirst,
			fnOrdered,
			{ dep_WIC }
		);
	}

	return TRUE;
}
