#pragma once
#include <WIC.h>
#include "StackManager.h"

/*
 * StackPushBuffClass - 栈标记 Buff
 *
 * Effect.Type=StackPush
 * Effect.ExtraCodeA = 栈 ID (数字)
 *
 * 行为：
 *   Active 状态 → 将单位入栈
 *   After 状态 → 将单位出栈
 *   单位指针无效时 → 清理栈
 *
 * 这是一个仅栈标记的 Buff，不产生游戏性效果。
 * 支持所有标准 WIC Buff INI 属性（Duration, Anim, Order 等）。
 */
class StackPushBuffClass : public SIBuffClass
{
public:
	StackPushBuffClass() : SIBuffClass() {}

	// 初始化：读取栈 ID 配置
	virtual void EffectDataInit() override;

	// 激发时：将单位入栈
	virtual void OnEnterState_Active() override;

	// 结束时：将单位出栈
	virtual void OnEnterState_After() override;

	// 指针失效时：清理栈
	virtual void EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed) override;

	// SIExtraCode_A 自动存读档，用来存储栈 ID
};
