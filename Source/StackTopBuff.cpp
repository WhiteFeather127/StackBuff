#include "StackTopBuff.h"
#include "StackManager.h"

// ============================================================
// EffectDataInit - 读取配置参数
// ============================================================
void StackTopBuffClass::EffectDataInit()
{
	SIExtraCode_A = Type->SIEffect_ExtraCodeA;
	SIEffectMode_0 = Type->SIEffect_Modes.GetItem(0);
}

// ============================================================
// OnEnterState_Active - 激发阶段重置计数器和延迟
// ============================================================
void StackTopBuffClass::OnEnterState_Active()
{
	SICountLeft = Type->SIEffect_Counts.GetItem(0);
	SIDelayLeft = Type->SIEffect_Delay;
}

// ============================================================
// EffectAI - 手动管理 Delay / Count，到时执行
// ============================================================
void StackTopBuffClass::EffectAI(SIBuffClass_EffectData* 生效数据)
{
	if (!Type)
		return;

	HouseClass* ownerHouse = GetActiveOwnerHouse();
	if (!ownerHouse)
		return;

	// 延迟递减（SIDelayLeft == 0 时视为无需等待，每帧触发）
	if (SIDelayLeft > 0)
	{
		--SIDelayLeft;
		if (SIDelayLeft > 0)
			return;
	}
	// 计时归零 → 执行核心逻辑

	int stackId = SIExtraCode_A;

	// 3. 查找栈顶单位
	TechnoClass* topUnit = StackManager::Get().GetTop(stackId, ownerHouse);
	if (!topUnit)
		return;

	// 4. 获取栈顶单位的 WIC 扩展数据接口
	SIInterface_ExtendData* ext = SIClassManager::GetExtendData(topUnit);
	if (!ext)
		return;

	TechnoClass* source = GetSourceTechno();
	HouseClass* sourceHouse = GetActiveSourceHouse();

	// 5. 遍历 Effect.AcceptBuffs，为栈顶单位创建/合并 Buff
	auto& buffList = Type->SIEffect_AcceptBuffs;
	for (size_t i = 0; i < buffList.size(); ++i)
	{
		SIBuffTypeClass* buffType = buffList.GetItem(i);
		if (buffType)
		{
			ext->Buff_CreateOrMerge(buffType, nullptr, source, sourceHouse);
		}
	}

	// 6. 出栈模式：移除栈顶单位的 StackPushBuff
	if (SIEffectMode_0 != 0)
	{
		SIBuffTypeClass* pushBuffType = SIClassManager::BuffType_Find("StackPush");
		if (pushBuffType)
		{
			ext->Buff_TryRemove(pushBuffType);
		}
	}

	// 7. 减少剩余次数
	--SICountLeft;

	// 8. 次数耗尽 → 结束自己
	if (SICountLeft <= 0)
	{
		EnterState(SIBuffClass_State::结束);
	}
	else
	{
		// 重置延迟，等待下一次触发
		SIDelayLeft = Type->SIEffect_Delay;
	}
}
