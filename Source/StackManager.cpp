#include "StackManager.h"
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
		return;

	HouseClass* house = buff->GetActiveOwnerHouse();
	if (!house)
		return;

	TechnoClass* owner = buff->GetOwnerTechno();
	auto& vec = m_data[house][stackId];

	// 去重：检查同一 buff 实例是否已在栈中
	for (const auto& entry : vec)
	{
		if (entry.Buff == buff)
			return;
	}

	vec.push_back({ buff, owner });
}

// ============================================================
// Pop - 弹出栈顶（不移除 buff，仅弹出条目）
// ============================================================
void StackManager::Pop(int stackId, HouseClass* house)
{
	if (!house)
		return;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
		return;

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
		return;

	if (!stackIt->second.empty())
		stackIt->second.pop_back();
}

// ============================================================
// GetTop - 获取栈顶 buff 实例（惰性重建：若 buff 为 null 则通过 owner 查找）
// ============================================================
SIBuffClass* StackManager::GetTop(int stackId, HouseClass* house)
{
	if (!house)
		return nullptr;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
		return nullptr;

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
		return nullptr;

	if (stackIt->second.empty())
		return nullptr;

	auto& entry = stackIt->second.back();

	// 惰性重建：buff 指针为 null 但 owner 有效时，尝试通过 owner 查找 StackPush buff
	if (!entry.Buff && entry.Owner)
	{
		entry.Buff = ResolvePushBuff(entry.Owner);
	}

	return entry.Buff;
}

// ============================================================
// RemoveBuff - 从栈中移除指定的 buff 实例
// ============================================================
bool StackManager::RemoveBuff(SIBuffClass* buff)
{
	if (!buff)
		return false;

	HouseClass* house = buff->GetActiveOwnerHouse();
	if (!house)
		return false;

	int stackId = buff->SIExtraCode_A;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
		return false;

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
		return false;

	auto& vec = stackIt->second;
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it->Buff == buff)
		{
			vec.erase(it);
			return true;
		}
	}
	return false;
}

// ============================================================
// ClearHouse - 清除某一阵营的所有栈
// ============================================================
void StackManager::ClearHouse(HouseClass* house)
{
	if (house)
		m_data.erase(house);
}

// ============================================================
// ClearAll - 清除所有栈
// ============================================================
void StackManager::ClearAll()
{
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
	return GetSize(stackId, house) == 0;
}

// ============================================================
// ResolvePushBuff - 通过 owner 查找其身上的 StackPush buff
// ============================================================
SIBuffClass* StackManager::ResolvePushBuff(TechnoClass* owner) const
{
	if (!owner)
		return nullptr;

	SIInterface_ExtendData* ext = SIClassManager::GetExtendData(owner);
	if (!ext)
		return nullptr;

	SIBuffTypeClass* pushType = SIClassManager::BuffType_Find("StackPush");
	if (!pushType)
		return nullptr;

	return ext->Buff_GetBuff(pushType);
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

	for (uint32_t h = 0; h < houseCount; ++h)
	{
		HouseClass* house = nullptr;
		ECSavegameHelper::ReadStream(stream, house, false);

		uint32_t stackCount = 0;
		ECSavegameHelper::ReadStream(stream, stackCount, false);

		auto& stacks = m_data[house];

		for (uint32_t s = 0; s < stackCount; ++s)
		{
			int stackId = 0;
			ECSavegameHelper::ReadStream(stream, stackId, false);

			uint32_t entryCount = 0;
			ECSavegameHelper::ReadStream(stream, entryCount, false);

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
					entry.Owner = newOwner;
				}
			}
		}
	}
}
