#pragma once
#include <map>
#include <vector>
#include <YRPP.h>

class HouseClass;
class TechnoClass;
class ECStreamReader;
class ECStreamWriter;

/*
 * StackManager - 全局单例，管理按阵营、按 ID 的数字栈
 * 每个栈存储 TechnoClass* 指针（LIFO 顺序）
 * 支持 Save/Load 序列化
 */
class StackManager
{
public:
	static StackManager& Get();

	// 栈操作
	void Push(int stackId, HouseClass* house, TechnoClass* unit);
	void Pop(int stackId, HouseClass* house);
	TechnoClass* GetTop(int stackId, HouseClass* house) const;
	bool RemoveUnit(int stackId, HouseClass* house, TechnoClass* unit);
	void ClearHouse(HouseClass* house);
	void ClearAll();
	int GetSize(int stackId, HouseClass* house) const;
	bool IsEmpty(int stackId, HouseClass* house) const;

	// 无效指针清理（由 EffectTriggerPointerGotInvalid 触发）
	void OnPointerGotInvalid(AbstractClass* ptr, bool removed);

	// Save/Load 序列化
	void SaveToStream(ECStreamWriter& stream) const;
	void LoadFromStream(ECStreamReader& stream);
	void FinalSwizzle();

private:
	StackManager() = default;
	~StackManager() = default;
	StackManager(const StackManager&) = delete;
	StackManager& operator=(const StackManager&) = delete;

	// 数据结构: house -> (stackId -> [unit1, unit2, ...])
	// 栈顶是 vector 的最后一个元素
	std::map<HouseClass*, std::map<int, std::vector<TechnoClass*>>> m_data;
};
