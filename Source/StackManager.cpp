#include "StackManager.h"
#include "Debug.h"
#include <WIC.h>
#include <EC.Stream.h>
#include <SwizzleManagerClass.h>
#include <cstdint>

StackManager& StackManager::Get()
{
	static StackManager instance;
	return instance;
}

// ============================================================
// Push - 将 buff 实例入栈
// ============================================================
void StackManager::Push(int stackId, SIBuffClass* buff)
{
	if (!buff)
	{
		DEBUG_LOG("[StackMgr] Push: buff==null\n");
		return;
	}

	HouseClass* house = buff->GetActiveOwnerHouse();
	if (!house)
	{
		DEBUG_LOG("[StackMgr] Push: house==null  stackId=%d\n", stackId);
		return;
	}

	TechnoClass* owner = buff->GetOwnerTechno();
	auto& vec = m_data[house][stackId];

	// 去重：检查同一 buff 实例是否已在栈中
	for (const auto& entry : vec)
	{
		if (entry.Buff == buff)
		{
			DEBUG_LOG("[StackMgr] Push: duplicate buff=%p, skip\n", (void*)buff);
			return;
		}
	}

	vec.push_back({ buff, owner });
	DEBUG_LOG("[StackMgr] Push: stackId=%d  buff=%p  owner=%p  size=%zu\n",
		stackId, (void*)buff, (void*)owner, vec.size());
}

// ============================================================
// Pop - 弹出栈顶（不移除 buff，仅弹出条目）
// ============================================================
void StackManager::Pop(int stackId, HouseClass* house)
{
	if (!house)
	{
		DEBUG_LOG("[StackMgr] Pop: house==null  stackId=%d\n", stackId);
		return;
	}

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
	{
		DEBUG_LOG("[StackMgr] Pop: house not found  stackId=%d\n", stackId);
		return;
	}

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
	{
		DEBUG_LOG("[StackMgr] Pop: stackId=%d not found for house\n", stackId);
		return;
	}

	if (!stackIt->second.empty())
	{
		DEBUG_LOG("[StackMgr] Pop: stackId=%d  size_before=%zu\n", stackId, stackIt->second.size());
		stackIt->second.pop_back();
	}
	else
	{
		DEBUG_LOG("[StackMgr] Pop: stackId=%d already empty\n", stackId);
	}
}

// ============================================================
// GetTop - 获取栈顶 buff 实例（惰性重建：若 buff 为 null 则通过 owner 查找）
// ============================================================
SIBuffClass* StackManager::GetTop(int stackId, HouseClass* house)
{
	if (!house)
	{
		DEBUG_LOG("[StackMgr] GetTop: house==null  stackId=%d\n", stackId);
		return nullptr;
	}

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
	{
		DEBUG_LOG("[StackMgr] GetTop: house not found  stackId=%d\n", stackId);
		return nullptr;
	}

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
	{
		DEBUG_LOG("[StackMgr] GetTop: stackId=%d not found\n", stackId);
		return nullptr;
	}

	if (stackIt->second.empty())
	{
		DEBUG_LOG("[StackMgr] GetTop: stackId=%d is empty\n", stackId);
		return nullptr;
	}

	auto& entry = stackIt->second.back();

	// 惰性重建：buff 指针为 null 但 owner 有效时，尝试通过 owner 查找 StackPush buff
	if (!entry.Buff && entry.Owner)
	{
		DEBUG_LOG("[StackMgr] GetTop: lazy-resolve buff for owner=%p\n", (void*)entry.Owner);
		entry.Buff = ResolvePushBuff(entry.Owner);
		DEBUG_LOG("[StackMgr] GetTop: resolved buff=%p\n", (void*)entry.Buff);
	}

	DEBUG_LOG("[StackMgr] GetTop: stackId=%d -> buff=%p  owner=%p\n",
		stackId, (void*)entry.Buff, (void*)entry.Owner);
	return entry.Buff;
}

// ============================================================
// RemoveBuff - 从栈中移除指定的 buff 实例
// ============================================================
bool StackManager::RemoveBuff(SIBuffClass* buff)
{
	if (!buff)
	{
		DEBUG_LOG("[StackMgr] RemoveBuff: buff==null\n");
		return false;
	}

	HouseClass* house = buff->GetActiveOwnerHouse();
	if (!house)
	{
		DEBUG_LOG("[StackMgr] RemoveBuff: house==null  buff=%p\n", (void*)buff);
		return false;
	}

	int stackId = buff->SIExtraCode_A;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
	{
		DEBUG_LOG("[StackMgr] RemoveBuff: house not found  buff=%p  stackId=%d\n", (void*)buff, stackId);
		return false;
	}

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
	{
		DEBUG_LOG("[StackMgr] RemoveBuff: stackId=%d not found  buff=%p\n", stackId, (void*)buff);
		return false;
	}

	auto& vec = stackIt->second;
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it->Buff == buff)
		{
			DEBUG_LOG("[StackMgr] RemoveBuff: removed buff=%p  stackId=%d  size_before=%zu\n",
				(void*)buff, stackId, vec.size());
			vec.erase(it);
			return true;
		}
	}

	DEBUG_LOG("[StackMgr] RemoveBuff: buff=%p not found in stackId=%d\n", (void*)buff, stackId);
	return false;
}

// ============================================================
// ClearHouse - 清除某一阵营的所有栈
// ============================================================
void StackManager::ClearHouse(HouseClass* house)
{
	if (house)
	{
		size_t count = m_data.count(house);
		m_data.erase(house);
		DEBUG_LOG("[StackMgr] ClearHouse: house=%p  stacks_removed=%zu\n", (void*)house, count);
	}
}

// ============================================================
// ClearAll - 清除所有栈
// ============================================================
void StackManager::ClearAll()
{
	DEBUG_LOG("[StackMgr] ClearAll: clearing all data\n");
	m_data.clear();
}

// ============================================================
// GetSize - 获取栈大小
// ============================================================
int StackManager::GetSize(int stackId, HouseClass* house) const
{
	if (!house)
		return 0;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
		return 0;

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
		return 0;

	return static_cast<int>(stackIt->second.size());
}

// ============================================================
// IsEmpty - 判断栈是否为空
// ============================================================
bool StackManager::IsEmpty(int stackId, HouseClass* house) const
{
	bool empty = GetSize(stackId, house) == 0;
	DEBUG_LOG("[StackMgr] IsEmpty: stackId=%d -> %s\n", stackId, empty ? "true" : "false");
	return empty;
}

// ============================================================
// ResolvePushBuff - 通过 owner 查找其身上的 StackPush buff
// ============================================================
SIBuffClass* StackManager::ResolvePushBuff(TechnoClass* owner) const
{
	if (!owner)
	{
		DEBUG_LOG("[StackMgr] ResolvePushBuff: owner==null\n");
		return nullptr;
	}

	SIInterface_ExtendData* ext = SIClassManager::GetExtendData(owner);
	if (!ext)
	{
		DEBUG_LOG("[StackMgr] ResolvePushBuff: no extend data for owner=%p\n", (void*)owner);
		return nullptr;
	}

	SIBuffTypeClass* pushType = SIClassManager::BuffType_Find("StackPush");
	if (!pushType)
	{
		DEBUG_LOG("[StackMgr] ResolvePushBuff: StackPush type not found\n");
		return nullptr;
	}

	SIBuffClass* buff = ext->Buff_GetBuff(pushType);
	DEBUG_LOG("[StackMgr] ResolvePushBuff: owner=%p -> buff=%p\n", (void*)owner, (void*)buff);
	return buff;
}

// ============================================================
// SaveToStream - 序列化（保存 owner 指针，它会被 EC 框架自动 Swizzle）
// ============================================================
void StackManager::SaveToStream(ECStreamWriter& stream) const
{
	// 格式: [houseCount] -> 对每个 house: [housePtr] [stackCount]
	//   -> 对每个 stack: [stackId] [entryCount]
	//     -> 对每个 entry: [ownerPtr]

	uint32_t houseCount = static_cast<uint32_t>(m_data.size());
	ECSavegameHelper::WriteStream(stream, houseCount);

	DEBUG_LOG("[StackMgr] SaveToStream: houseCount=%u\n", houseCount);

	for (const auto& [house, stacks] : m_data)
	{
		ECSavegameHelper::WriteStream(stream, house);

		uint32_t stackCount = static_cast<uint32_t>(stacks.size());
		ECSavegameHelper::WriteStream(stream, stackCount);

		for (const auto& [stackId, vec] : stacks)
		{
			ECSavegameHelper::WriteStream(stream, stackId);

			uint32_t entryCount = static_cast<uint32_t>(vec.size());
			ECSavegameHelper::WriteStream(stream, entryCount);

			DEBUG_LOG("[StackMgr] SaveToStream: house=%p  stackId=%d  entries=%u\n",
				(void*)house, stackId, entryCount);

			for (const auto& entry : vec)
			{
				// 保存 owner 指针（会在加载时被 Swizzle 重排）
				ECSavegameHelper::WriteStream(stream, entry.Owner);
			}
		}
	}
}

// ============================================================
// LoadFromStream - 反序列化（读入 owner 指针，FinalSwizzle 中 Swizzle）
// ============================================================
void StackManager::LoadFromStream(ECStreamReader& stream)
{
	ClearAll();

	uint32_t houseCount = 0;
	ECSavegameHelper::ReadStream(stream, houseCount, false);

	DEBUG_LOG("[StackMgr] LoadFromStream: houseCount=%u\n", houseCount);

	for (uint32_t h = 0; h < houseCount; ++h)
	{
		HouseClass* house = nullptr;
		ECSavegameHelper::ReadStream(stream, house, false);

		uint32_t stackCount = 0;
		ECSavegameHelper::ReadStream(stream, stackCount, false);

		DEBUG_LOG("[StackMgr] LoadFromStream: [%u] house=%p  stackCount=%u\n", h, (void*)house, stackCount);

		auto& stacks = m_data[house];

		for (uint32_t s = 0; s < stackCount; ++s)
		{
			int stackId = 0;
			ECSavegameHelper::ReadStream(stream, stackId, false);

			uint32_t entryCount = 0;
			ECSavegameHelper::ReadStream(stream, entryCount, false);

			DEBUG_LOG("[StackMgr] LoadFromStream:   [%u/%u] stackId=%d  entries=%u\n",
				s, stackCount, stackId, entryCount);

			auto& vec = stacks[stackId];
			vec.reserve(entryCount);

			for (uint32_t u = 0; u < entryCount; ++u)
			{
				TechnoClass* owner = nullptr;
				ECSavegameHelper::ReadStream(stream, owner, false);
				// Buff 先置 null，FinalSwizzle 中 swizzle owner，GetTop 惰性重建 buff
				vec.push_back({ nullptr, owner });
			}
		}
	}
}

// ============================================================
// FinalSwizzle - 加载后重排 house 和 owner 指针（不重建 buff，由 GetTop 惰性完成）
// ============================================================
void StackManager::FinalSwizzle()
{
	DEBUG_LOG("[StackMgr] FinalSwizzle: start  total_houses=%zu\n", m_data.size());

	// 第一阶段：收集需要重排的 house 指针
	struct HouseSwap {
		HouseClass* oldPtr;
		HouseClass* newPtr;
		std::map<int, std::vector<StackEntry>> stacks;
	};
	std::vector<HouseSwap> houseSwaps;

	for (auto& [house, stacks] : m_data)
	{
		HouseClass* newHouse = house;
		SwizzleManagerClass::Instance.Swizzle(reinterpret_cast<void**>(&newHouse));
		if (newHouse != house)
		{
			DEBUG_LOG("[StackMgr] FinalSwizzle: house swizzle %p -> %p\n", (void*)house, (void*)newHouse);
			houseSwaps.push_back({ house, newHouse, std::move(stacks) });
		}
	}

	// 应用 house 指针重排
	for (auto& swap : houseSwaps)
	{
		m_data[swap.newPtr] = std::move(swap.stacks);
		m_data.erase(swap.oldPtr);
	}

	// 第二阶段：Swizzle 每个 entry 中的 owner 指针
	// buff 指针不在这里重建，留到 GetTop 惰性重建以确保拿到正确的实例
	int totalEntries = 0;
	for (auto& [house, stacks] : m_data)
	{
		for (auto& [stackId, vec] : stacks)
		{
			for (auto& entry : vec)
			{
				if (entry.Owner)
				{
					TechnoClass* newOwner = entry.Owner;
					SwizzleManagerClass::Instance.Swizzle(reinterpret_cast<void**>(&newOwner));
					if (newOwner != entry.Owner)
					{
						DEBUG_LOG("[StackMgr] FinalSwizzle: owner swizzle %p -> %p\n",
							(void*)entry.Owner, (void*)newOwner);
					}
					entry.Owner = newOwner;
				}
				totalEntries++;
			}
		}
	}

	DEBUG_LOG("[StackMgr] FinalSwizzle: done  house_swaps=%zu  total_entries=%d\n",
		houseSwaps.size(), totalEntries);
}
