#pragma once
#include <WIC.h>
#include "StackManager.h"

/*
 * StackTopBuffClass - 栈顶附加 Buff
 *
 * Effect.Type=StackTopBuff
 * Effect.ExtraCodeA = 栈 ID (数字)
 * Effect.AcceptBuffs = 要给栈顶单位附加的 Buff 名称列表
 * Effect.Modes[0] = 0=不弹出, 1=附加后弹出栈顶单位
 *
 * 行为：
 *   Active 状态 → 查找栈顶单位，通过 SIInterface_ExtendData::Buff_CreateOrMerge
 *                 直接给栈顶单位附加指定的 Buff
 *                 如果 Modes[0]==1，移除栈顶单位的 StackPushBuff（出栈）
 *
 * 实现方式：纯 SDK 接口，不使用弹头
 */
class StackTopBuffClass : public SIBuffClass
{
public:
	StackTopBuffClass() : SIBuffClass() {}

	// 初始化：读取配置
	virtual void EffectDataInit() override
	{
		if (Type)
		{
			SIExtraCode_A = Type->SIEffect_ExtraCodeA;          // 栈 ID
			SIEffectMode_0 = Type->SIEffect_Modes.TryGetItem(0, 0); // 出栈标志
		}
	}

	// 激发时：执行核心逻辑
	virtual void OnEnterState_Active() override
	{
		if (!Type)
			return;

		int stackId = SIExtraCode_A;
		HouseClass* ownerHouse = GetActiveOwnerHouse();
		if (!ownerHouse)
			return;

		// 1. 查找栈顶单位
		TechnoClass* topUnit = StackManager::Get().GetTop(stackId, ownerHouse);
		if (!topUnit)
			return;

		// 2. 获取栈顶单位的 WIC 扩展数据接口
		SIInterface_ExtendData* ext = SIClassManager::GetExtendData(topUnit);
		if (!ext)
			return;

		TechnoClass* source = GetSourceTechno();
		HouseClass* sourceHouse = GetActiveSourceHouse();

		// 3. 遍历 Effect.AcceptBuffs，为栈顶单位创建/合并 Buff
		auto& buffList = Type->SIEffect_AcceptBuffs;
		for (size_t i = 0; i < buffList.size(); ++i)
		{
			SIBuffTypeClass* buffType = buffList.GetItem(i);
			if (buffType)
			{
				// 使用默认设置包（nullptr）附加 Buff
				// 如有需要可通过 BuffSetting_FindOrAllocate 获取预制设置包
				ext->Buff_CreateOrMerge(buffType, nullptr, source, sourceHouse);
			}
		}

		// 4. 出栈模式：移除栈顶单位的 StackPushBuff
		if (SIEffectMode_0 != 0)
		{
			SIBuffTypeClass* pushBuffType = SIClassManager::BuffType_Find("StackPush");
			if (pushBuffType)
			{
				// Buff_TryRemove 会触发 StackPushBuff 的 OnEnterState_Remove
				// 进而自动调用 StackManager::RemoveUnit 完成出栈
				ext->Buff_TryRemove(pushBuffType);
			}
		}
	}

private:
	// SIExtraCode_A = 栈 ID（自动存读档）
	// SIEffectMode_0 = 出栈标志（自动存读档）
};
