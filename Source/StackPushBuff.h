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
 *   Active 状态 → 将自身（buff 实例）入栈
 *   After / Remove 状态 → 将自身（buff 实例）出栈
 *
 * StackTopBuff 通过栈顶的 buff 实例获取所属单位（GetOwnerTechno()），
 * 并可直接结束 buff 实例完成出栈，无需按类型查找。
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

	// 结束（After）时：将单位出栈
	virtual void OnEnterState_After() override;

	// 移除（Remove）时：将单位出栈（双保险，保证任何路径都出栈）
	virtual void OnEnterState_Remove() override;

	// 每帧 AI — 用于读档后延迟重建栈（此时 WIC 已完全就绪）
	virtual void EffectAI(SIBuffClass_EffectData* 生效数据) override;

	// 指针失效时：清理栈
	virtual void EffectTriggerPointerGotInvalid(AbstractClass* ptr, bool removed) override;

	// SIExtraCode_A 自动存读档，用来存储栈 ID
};
