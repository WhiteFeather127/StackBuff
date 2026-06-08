#include "StackTopBuff.h"
#include "StackManager.h"

// ============================================================
// EffectDataInit - 读取配置参数
// ============================================================
void StackTopBuffClass::EffectDataInit()
{
	if (Type)
	{
		SIExtraCode_A = Type->SIEffect_ExtraCodeA;			   // 栈 ID
		SIEffectMode_0 = Type->SIEffect_Modes.TryGetItem(0, 0); // 出栈标志
	}
}

// ============================================================
// OnEnterState_Active - 激发时：查找栈顶单位并附加 Buff
// ============================================================
void StackTopBuffClass::OnEnterState_Active()
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
			ext->Buff_CreateOrMerge(buffType, nullptr, source, sourceHouse);
		}
	}

	// 4. 出栈模式：移除栈顶单位的 StackPushBuff
	if (SIEffectMode_0 != 0)
	{
		SIBuffTypeClass* pushBuffType = SIClassManager::BuffType_Find("StackPush");
		if (pushBuffType)
		{
			ext->Buff_TryRemove(pushBuffType);
		}
	}
}
