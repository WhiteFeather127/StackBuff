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
 * 每个栈存储 StackEntry（buff 实例 + owner 单位指针，LIFO 顺序）
 * 通过 buff 实例可获取所属单位（GetOwnerTechno()）
 * 支持 Save/Load 序列化
 *
 * Save/Load 策略：
 *   - Save: 保存 owner 指针（可被 Swizzle）
 *   - Load: 读入 owner 指针，FinalSwizzle 中 swizzle owner
 *   - GetTop: 若 buff 指针为 null，通过 owner 惰性重建（确保拿到正确的 buff 实例）
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

private:
	StackManager() = default;
	~StackManager() = default;
	StackManager(const StackManager&) = delete;
	StackManager& operator=(const StackManager&) = delete;

	// 栈条目：同时存储 buff 实例和 owner 指针
	// Save 时保存 Owner，Load 后 Owner 被 Swizzle，Buff 由 GetTop 惰性重建
	struct StackEntry {
		SIBuffClass* Buff;     // buff 实例（运行时有效，加载后可能为 null）
		TechnoClass* Owner;    // owner 指针（用于 Save/Load 和惰性重建）
	};

	// 尝试通过 owner 查找 StackPush buff
	SIBuffClass* ResolvePushBuff(TechnoClass* owner) const;

	// 数据结构: house -> (stackId -> [entry1, entry2, ...])
	// 栈顶是 vector 的最后一个元素
	std::map<HouseClass*, std::map<int, std::vector<StackEntry>>> m_data;
};
