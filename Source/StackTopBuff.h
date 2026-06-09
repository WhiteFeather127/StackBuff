#pragma once
#include <WIC.h>
#include "StackManager.h"

/*
 * StackTopBuffClass - 栈顶附加 Buff
 *
 * Effect.Type=StackTop
 * Effect.ExtraCodeA = 栈 ID (数字)
 * Effect.AcceptBuffs = 要给栈顶单位附加的 Buff 名称列表
 * Effect.Modes[0] = 0=不弹出, 1=附加后弹出栈顶单位
 *
 * 行为：
 *   OnEnterState_Active（激发阶段）→ 重置 SICountLeft/SIDelayLeft
 *   EffectAI（生效阶段）→ 手动管理 SIDelayLeft/SICountLeft，
 *                          按 Delay→执行→Count 的顺序逐帧递减。
 *                          延迟归零时：获取栈顶 buff 实例，
 *                          通过 buff->GetOwnerTechno() 获取单位，
 *                          通过 Buff_CreateOrMerge 附加 Buff，
 *                          如果 Modes[0]==1 则直接结束 buff 实例（topBuff->TryAfter()）完成出栈。
 *                          次数耗尽后调用 EnterState(结束) 结束自身。
 *   配置：Effect.Counts[0] = 次数, Effect.Delay = 间隔帧数
 *
 * 实现方式：纯 SDK 接口，不使用弹头
 */
class StackTopBuffClass : public SIBuffClass
{
public:
	StackTopBuffClass() : SIBuffClass() {}

	// 初始化：读取配置
	virtual void EffectDataInit() override;

	// 激发阶段：重置计数器和延迟
	virtual void OnEnterState_Active() override;

	// 生效时：每轮触发执行核心逻辑
	virtual void EffectAI(SIBuffClass_EffectData* 生效数据) override;

	// SIExtraCode_A = 栈 ID（自动存读档）
	// SIEffectMode_0 = 出栈标志（自动存读档）
};
