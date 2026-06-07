#include "StackManager.h"
#include <EC.Stream.h>
#include <SwizzleManagerClass.h>

StackManager& StackManager::Get()
{
	static StackManager instance;
	return instance;
}

void StackManager::Push(int stackId, HouseClass* house, TechnoClass* unit)
{
	if (!house || !unit)
		return;

	auto& stacks = m_data[house];
	auto& vec = stacks[stackId];

	// 去重：检查单位是否已在栈中
	for (auto* u : vec)
	{
		if (u == unit)
			return; // 已在栈中，不重复入栈
	}

	vec.push_back(unit);
}

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

TechnoClass* StackManager::GetTop(int stackId, HouseClass* house) const
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

	return stackIt->second.back();
}

bool StackManager::RemoveUnit(int stackId, HouseClass* house, TechnoClass* unit)
{
	if (!house || !unit)
		return false;

	auto houseIt = m_data.find(house);
	if (houseIt == m_data.end())
		return false;

	auto stackIt = houseIt->second.find(stackId);
	if (stackIt == houseIt->second.end())
		return false;

	auto& vec = stackIt->second;
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (*it == unit)
		{
			vec.erase(it);
			return true;
		}
	}
	return false;
}

void StackManager::ClearHouse(HouseClass* house)
{
	if (house)
		m_data.erase(house);
}

void StackManager::ClearAll()
{
	m_data.clear();
}

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

bool StackManager::IsEmpty(int stackId, HouseClass* house) const
{
	return GetSize(stackId, house) == 0;
}

void StackManager::OnPointerGotInvalid(AbstractClass* ptr, bool removed)
{
	if (!ptr)
		return;

	// 遍历所有栈，移除失效的指针
	for (auto& [house, stacks] : m_data)
	{
		for (auto& [stackId, vec] : stacks)
		{
			for (auto it = vec.begin(); it != vec.end(); )
			{
				if (*it == ptr)
					it = vec.erase(it);
				else
					++it;
			}
		}
	}
}

void StackManager::SaveToStream(ECStreamWriter& stream) const
{
	// 格式: [houseCount] -> 对每个 house: [housePtr] [stackCount] -> 对每个 stack: [stackId] [unitCount] [unitPtr...]

	uint32_t houseCount = static_cast<uint32_t>(m_data.size());
	ECSavegameHelper::WriteStream(stream, houseCount);

	for (const auto& [house, stacks] : m_data)
	{
		// 写入 house 指针（会在加载时被 Swizzle 重排）
		ECSavegameHelper::WriteStream(stream, house);

		uint32_t stackCount = static_cast<uint32_t>(stacks.size());
		ECSavegameHelper::WriteStream(stream, stackCount);

		for (const auto& [stackId, vec] : stacks)
		{
			ECSavegameHelper::WriteStream(stream, stackId);

			uint32_t unitCount = static_cast<uint32_t>(vec.size());
			ECSavegameHelper::WriteStream(stream, unitCount);

			for (const auto* unit : vec)
			{
				// 写入单位指针（会在加载时被 Swizzle 重排）
				ECSavegameHelper::WriteStream(stream, unit);
			}
		}
	}
}

void StackManager::LoadFromStream(ECStreamReader& stream)
{
	// 清空现有数据
	ClearAll();

	uint32_t houseCount = 0;
	ECSavegameHelper::ReadStream(stream, houseCount, false);

	for (uint32_t h = 0; h < houseCount; ++h)
	{
		HouseClass* house = nullptr;
		ECSavegameHelper::ReadStream(stream, house, false); // 仅读取, 不在局部变量注册 Swizzle

		uint32_t stackCount = 0;
		ECSavegameHelper::ReadStream(stream, stackCount, false);

		auto& stacks = m_data[house];

		for (uint32_t s = 0; s < stackCount; ++s)
		{
			int stackId = 0;
			ECSavegameHelper::ReadStream(stream, stackId, false);

			uint32_t unitCount = 0;
			ECSavegameHelper::ReadStream(stream, unitCount, false);

			auto& vec = stacks[stackId];
			vec.reserve(unitCount);

			for (uint32_t u = 0; u < unitCount; ++u)
			{
				TechnoClass* unit = nullptr;
				ECSavegameHelper::ReadStream(stream, unit, false); // 仅读取, 不在局部变量注册 Swizzle
				vec.push_back(unit);
			}
		}
	}
}

void StackManager::FinalSwizzle()
{
	// 第一阶段：收集需要重排的 house 指针
	// 注意：不能在迭代时修改 map 的 key，所以先收集再处理
	struct HouseSwap {
		HouseClass* oldPtr;
		HouseClass* newPtr;
	};
	std::vector<HouseSwap> houseSwaps;

	for (auto& [house, stacks] : m_data)
	{
		HouseClass* newHouse = house;
		SwizzleManagerClass::Instance.Swizzle(reinterpret_cast<void**>(&newHouse));
		if (newHouse != house)
		{
			houseSwaps.push_back({ house, newHouse });
		}
	}

	// 应用 house 指针重排
	for (const auto& swap : houseSwaps)
	{
		auto node = m_data.extract(swap.oldPtr);
		if (!node.empty())
		{
			node.key() = swap.newPtr;
			m_data.insert(std::move(node));
		}
	}

	// 第二阶段：重排所有栈中的单位指针
	// 注意: 此时 Swizzle 映射已由 EC 框架的全局 Swizzle 阶段建立完毕
	// 我们在此手动查找每个指针的新地址
	for (auto& [house, stacks] : m_data)
	{
		for (auto& [stackId, vec] : stacks)
		{
			for (auto& unit : vec)
			{
				TechnoClass* newUnit = unit;
				SwizzleManagerClass::Instance.Swizzle(reinterpret_cast<void**>(&newUnit));
				unit = newUnit;
			}
		}
	}
}
