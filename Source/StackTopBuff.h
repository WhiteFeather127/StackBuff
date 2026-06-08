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
	virtual void EffectDataInit() override;

	// 激发时：执行核心逻辑
	virtual void OnEnterState_Active() override;

	// SIExtraCode_A = 栈 ID（自动存读档）
	// SIEffectMode_0 = 出栈标志（自动存读档）
};
