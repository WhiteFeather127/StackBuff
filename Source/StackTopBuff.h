#pragma once
#include <WIC.h>
#include "StackManager.h"

/*
 * StackTopBuffClass - 栈顶附加 Buff
 *
 * Effect.Type=StackTop
 * Effect.ExtraCodeA = 栈 ID (数字)
 * Effect.Modes[0] = 0=不弹出, 1=附加后弹出栈顶单位
 *
 * 通用效果属性（与标准 Buff INI 配置一致）：
 *   Effect.Anims[0]        = 生效时在自己身上播放的动画
 *   Effect.Anims[1]        = 生效时在受影响单位身上播放的动画（如果未设置则用 AnimsOthers[0]）
 *   Effect.AnimsOthers[0]  = 生效时在受影响单位身上播放的动画（备选）
 *   Effect.AcceptBuffs     = Buff 列表，挂载这些 Buff，没设置就不挂载
 *   Effect.Counts[0]       = 整数，生效次数
 *                             负数 = 无限次
 *                             0   = 无法生效，直接进入结束状态
 *                             正数 = 生效这么多次后结束
 *                             默认值 -1
 *   Effect.Delay           = 整数，每隔这么多帧生效一次
 *                             小于 0 按 0 算
 *                             每一帧最多生效一次
 *                             默认值 0
 *   Effect.Range           = 浮点数，影响的范围（半径）
 *                             0      = 只影响栈顶单位
 *                             大于 0 = 影响范围内所有单位
 *                             小于 0 按 0 算
 *                             单位：格子
 *
 * 行为：
 *   OnEnterState_Active（激发阶段）→ 重置 SICountLeft/SIDelayLeft
 *   EffectAI（生效阶段）→ 手动管理 SIDelayLeft/SICountLeft，
 *                          按 Delay→执行→Count 的顺序管理。
 *                          延迟归零时：
 *                            1. 获取栈顶 buff 实例，通过 buff->GetOwnerTechno() 获取单位
 *                            2. 在自己身上播放 Effect.Anims[0] 动画
 *                            3. 如果 Effect.Range > 0，查找范围内所有单位；
 *                               否则只处理栈顶单位
 *                            4. 在受影响单位身上播放 Effect.Anims[1]/AnimsOthers[0] 动画
 *                            5. 通过 Buff_CreateOrMerge 为每个单位附加 AcceptBuffs
 *                            6. 如果 Modes[0]==1 则结束栈顶 buff 实例（topBuff->TryAfter()）完成出栈
 *                            7. 次数耗尽后调用 EnterState(结束) 结束自身
 *
 * 实现方式：纯 SDK 接口，不使用弹头
 */
class StackTopBuffClass : public SIBuffClass
{
public:
	StackTopBuffClass() : SIBuffClass() {}

	// ========== 生命周期阶段 ==========

	// 初始化：读取配置
	virtual void EffectDataInit() override;

	// Init — 挂载阶段
	virtual void OnEnterState_Init() override;

	// Mark — 标记阶段
	virtual void OnEnterState_Mark() override;

	// Active — 激发阶段：重置计数器和延迟
	virtual void OnEnterState_Active() override;

	// Effect — 生效阶段
	virtual void OnEnterState_Effect() override;

	// After — 结束阶段
	virtual void OnEnterState_After() override;

	// Remove — 移除阶段
	virtual void OnEnterState_Remove() override;

	// 生效时：每轮触发执行核心逻辑
	virtual void EffectAI(SIBuffClass_EffectData* 生效数据) override;

	// SIExtraCode_A = 栈 ID（自动存读档）
	// SIEffectMode_0 = 出栈标志（自动存读档）
};
