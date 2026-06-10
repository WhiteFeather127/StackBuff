#include "StackTopBuff.h"
#include "StackManager.h"
#include <AnimClass.h>

// ============================================================
// EffectDataInit - 读取配置参数
// ============================================================
void StackTopBuffClass::EffectDataInit()
{
	SIExtraCode_A = Type->SIEffect_ExtraCodeA;
	SIEffectMode_0 = Type->SIEffect_Modes.GetItem(0);
}

// ============================================================
// OnEnterState_Init - 挂载阶段
// ============================================================
void StackTopBuffClass::OnEnterState_Init()
{
}

// ============================================================
// OnEnterState_Mark - 标记阶段
// ============================================================
void StackTopBuffClass::OnEnterState_Mark()
{
}

// ============================================================
// OnEnterState_Active - 激发阶段重置计数器和延迟
// ============================================================
void StackTopBuffClass::OnEnterState_Active()
{
	SICountLeft = Type->SIEffect_Counts.GetItem(0);
	SIDelayLeft = Type->SIEffect_Delay;
	if (SIDelayLeft < 0)
		SIDelayLeft = 0;

	// 次数等于 0 → 无法生效，直接结束
	if (SICountLeft == 0)
	{
		EnterState(SIBuffClass_State::结束);
	}
}

// ============================================================
// OnEnterState_Effect - 生效阶段
// ============================================================
void StackTopBuffClass::OnEnterState_Effect()
{
}

// ============================================================
// OnEnterState_After - 结束阶段
// ============================================================
void StackTopBuffClass::OnEnterState_After()
{
}

// ============================================================
// OnEnterState_Remove - 移除阶段
// ============================================================
void StackTopBuffClass::OnEnterState_Remove()
{
}

// ============================================================
// PlayAnimOnCoord - 在指定坐标播放动画（辅助函数）
// ============================================================
static void PlayAnimOnCoord(AnimTypeClass* animType, const CoordStruct& coord)
{
	if (!animType)
		return;
	GameCreate<AnimClass>(animType, coord);
}

// ============================================================
// ApplyBuffsToExt - 对单个目标播放动画 + 挂载 Buff
// ============================================================
static void ApplyBuffsToTarget(
	SIInterface_ExtendData* ext,
	AnimTypeClass* targetAnim,
	const CoordStruct& targetCoord,
	SIDataList<SIBuffTypeClass*>& buffList,
	AbstractClass* source,
	HouseClass* sourceHouse)
{
	// 播放目标动画
	if (targetAnim)
		PlayAnimOnCoord(targetAnim, targetCoord);

	// 挂载 Buff
	for (size_t i = 0; i < buffCount; ++i)
	{
		SIBuffTypeClass* buffType = buffList.GetItem(i);
		if (!buffType || !source || !sourceHouse)
			continue;

		SIPack_BuffSetting_FromStatic emptyPack = {};
		ext->Buff_CreateOrMerge(buffType, &emptyPack, source, sourceHouse);
	}
}

// ============================================================
// ForEach 回调参数
// ============================================================
struct ForEachRangeCtx
{
	CoordStruct CenterCoord;
	double RangeLeptonsSq;                 // 距离平方（避免开根号）
	TechnoClass* Exclude;                  // 排除的单位（栈顶单位自身）
	AnimTypeClass* TargetAnim;
	SIDataList<SIBuffTypeClass*>* BuffList;
	AbstractClass* Source;
	HouseClass* SourceHouse;
};

// ============================================================
// ForEach_Techno 回调 — 距离筛选 → 播放动画 → 挂载 Buff
// ============================================================
static bool CALLBACK ForEachRangeCB(void* param, SIInterface_ExtendData* ext)
{
	auto* ctx = static_cast<ForEachRangeCtx*>(param);

	AbstractClass* entity = ext->OwnerEntity();
	if (!entity || entity == ctx->Exclude)
		return false;

	CoordStruct coord;
	entity->GetCenterCoords(&coord);

	// 平方距离比较，避免开根号
	int dx = coord.X - ctx->CenterCoord.X;
	int dy = coord.Y - ctx->CenterCoord.Y;
	int dz = coord.Z - ctx->CenterCoord.Z;
	double distSq = static_cast<double>(dx * dx + dy * dy + dz * dz);

	if (distSq > ctx->RangeLeptonsSq)
		return false;

	ApplyBuffsToTarget(ext, ctx->TargetAnim, coord, *ctx->BuffList,
		ctx->Source, ctx->SourceHouse);

	return false; // 继续遍历
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

	// --------------------------------------------------------
	// 延迟管理
	// --------------------------------------------------------
	if (SIDelayLeft > 0)
	{
		--SIDelayLeft;
		if (SIDelayLeft > 0)
			return;
	}

	// --------------------------------------------------------
	// 获取栈顶相关对象
	// --------------------------------------------------------
	int stackId = SIExtraCode_A;

	SIBuffClass* topBuff = StackManager::Get().GetTop(stackId, ownerHouse);
	if (!topBuff)
		return;

	TechnoClass* topUnit = topBuff->GetOwnerTechno();
	if (!topUnit)
		return;

	TechnoClass* source = GetSourceTechno();
	HouseClass* sourceHouse = GetActiveSourceHouse();

	// --------------------------------------------------------
	// Effect.Anims[0] — 在自己身上播放动画
	// --------------------------------------------------------
	if (Type->SIEffect_Anims.size() > 0)
	if (Type->SIEffect_Anims.size() > 0)
	{
		AnimTypeClass* selfAnim = Type->SIEffect_Anims.GetItem(0);
		PlayAnimOnCoord(selfAnim, GetCenterCoords());
	}

	// --------------------------------------------------------
	// 获取目标动画（优先 Anims[1]，否则 AnimsOthers[0]）
	// --------------------------------------------------------
	AnimTypeClass* targetAnim = nullptr;
	if (Type->SIEffect_Anims.size() > 1)
		targetAnim = Type->SIEffect_Anims.GetItem(1);
	else if (Type->SIEffect_AnimsOthers.size() > 0)
		targetAnim = Type->SIEffect_AnimsOthers.GetItem(0);

	double range = Type->SIEffect_Range;
	if (range < 0.0)
		range = 0.0;

	if (range > 0.0)
	{
		// ----------------------------------------------------
		// Effect.Range > 0 — WIC 风格：ForEach_Techno 遍历全图
		// ----------------------------------------------------
		CoordStruct centerCoord = topUnit->GetCenterCoords();

		ForEachRangeCtx ctx;
		ctx.CenterCoord = centerCoord;
		double rangeLeptons = range * 256.0; // 格子 → leptons
		ctx.RangeLeptonsSq = rangeLeptons * rangeLeptons;
		ctx.Exclude = topUnit;
		ctx.TargetAnim = targetAnim;
		ctx.BuffList = &Type->SIEffect_AcceptBuffs;
		ctx.Source = source;
		ctx.SourceHouse = sourceHouse;

		SITool::ForEach_Techno(&ctx, ForEachRangeCB);
	}
	else
	{
		// ----------------------------------------------------
		// Effect.Range == 0 — 只处理栈顶单位
		// ----------------------------------------------------
		SIInterface_ExtendData* topExt = SIClassManager::GetExtendData(topUnit);
		if (topExt)
		{
			ApplyBuffsToTarget(topExt, targetAnim, topUnit->GetCenterCoords(),
				Type->SIEffect_AcceptBuffs, source, sourceHouse);
		}
	}

	// --------------------------------------------------------
	// 出栈模式：结束栈顶 buff 实例（触发 After/Remove 出栈）
	// --------------------------------------------------------
	if (SIEffectMode_0 != 0)
	{
		topBuff->TryAfter();
	}

	// --------------------------------------------------------
	// 次数管理
	// --------------------------------------------------------
	if (SICountLeft < 0)
	{
		// 负数 = 无限次，不递减，重置延迟等待下次触发
		SIDelayLeft = Type->SIEffect_Delay;
		if (SIDelayLeft < 0)
			SIDelayLeft = 0;
	}
	else if (SICountLeft > 0)
	{
		--SICountLeft;
		if (SICountLeft == 0)
		{
			// 次数耗尽 → 结束自己
			EnterState(SIBuffClass_State::结束);
		}
		else
		{
			// 重置延迟，等待下一次触发
			SIDelayLeft = Type->SIEffect_Delay;
			if (SIDelayLeft < 0)
				SIDelayLeft = 0;
		}
	}
	// SICountLeft == 0 的情况已在 OnEnterState_Active 中处理，不会到达此处
}


