#pragma once
#include <map>
#include <vector>
#include <YRPP.h>

class HouseClass;
class TechnoClass;
class SIBuffClass;
class SIInterface_ExtendData;
class ECStreamReader;
class ECStreamWriter;

/*
 * StackManager - 全局单例，管理按阵营、按 ID 的数字栈
 * 每个栈存储 StackEntry（buff 实例，LIFO 顺序）
 * 通过 buff 实例可获取所属单位（GetOwnerTechno()）和阵营（GetActiveOwnerHouse()）
 *
 * Save/Load 策略：
 *   - Save/Load 本身不做持久化（避免指针 Swizzle 不可靠的问题）
 *   - FinalSwizzle 阶段调用 RebuildFromBuffs()
 *     遍历全图 TechnoClass，找到活跃的 StackPush buff 重建栈
 */
class StackManager
{
public:
	static StackManager& Get();

	// 栈操作
	void Push(int stackId, SIBuffClass* buff);
	void Pop(int stackId, HouseClass* house);
	SIBuffClass* GetTop(int stackId, HouseClass* house);
	bool RemoveBuff(SIBuffClass* buff);
	void ClearHouse(HouseClass* house);
	void ClearAll();
	int GetSize(int stackId, HouseClass* house) const;
	bool IsEmpty(int stackId, HouseClass* house) const;

	// Save/Load 序列化
	void SaveToStream(ECStreamWriter& stream) const;
	void LoadFromStream(ECStreamReader& stream);
	void FinalSwizzle();

	// 从 UID 列表匹配 Buff 重建栈（在 AfterLoadGame 中调度，EffectAI 中执行）
	void RebuildFromUIDs();

	// 延迟重建调度：AfterLoadGame 中设置标记，EffectAI 中实际执行
	void ScheduleRebuild() { m_pendingRebuild = true; }
	bool NeedsRebuild() const { return m_pendingRebuild; }

private:
	StackManager() = default;
	~StackManager() = default;
	StackManager(const StackManager&) = delete;
	StackManager& operator=(const StackManager&) = delete;

	// 栈条目
	struct StackEntry {
		SIBuffClass* Buff;
	};

	// 读档后待重建标记（AfterLoadGame 设置，首个 StackPush::EffectAI 中执行重建）
	bool m_pendingRebuild = false;

	// 读档时暂存的 UID 列表：(stackId, buffUID)
	std::vector<std::pair<int, int>> m_pendingUIDs;

	// 数据结构: house -> (stackId -> [entry1, entry2, ...])
	// 栈顶是 vector 的最后一个元素
	std::map<HouseClass*, std::map<int, std::vector<StackEntry>>> m_data;
};
