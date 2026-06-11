#include "StackManager.h"
#include "Debug.h"
#include <WIC.h>
#include <EC.Stream.h>
#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

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

	vec.push_back({ buff });
	DEBUG_LOG("[StackMgr] Push: stackId=%d  buff=%p  house=%p  size=%zu\n",
		stackId, (void*)buff, (void*)house, vec.size());
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
// GetTop - 获取栈顶 buff 实例
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
	DEBUG_LOG("[StackMgr] GetTop: stackId=%d -> buff=%p\n",
		stackId, (void*)entry.Buff);
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
// ============================================================
// SaveToStream - 保存 [stackId, buffUID] 扁平列表
// ============================================================
void StackManager::SaveToStream(ECStreamWriter& stream) const
{
	size_t totalEntries = 0;
	for (const auto& [house, stacks] : m_data)
		for (const auto& [stackId, vec] : stacks)
			totalEntries += vec.size();

	uint32_t count = static_cast<uint32_t>(totalEntries);
	ECSavegameHelper::WriteStream(stream, count);

	for (const auto& [house, stacks] : m_data)
		for (const auto& [stackId, vec] : stacks)
			for (const auto& entry : vec)
			{
				ECSavegameHelper::WriteStream(stream, stackId);
				int uid = entry.Buff ? entry.Buff->UID : -1;
				ECSavegameHelper::WriteStream(stream, uid);
			}

	DEBUG_LOG("[StackMgr] SaveToStream: totalEntries=%u\n", count);
}

// ============================================================
// LoadFromStream - 读入 [stackId, buffUID] 列表，暂存
// FinalSwizzle 不做任何事，AfterLoadGame 中重建
// ============================================================
void StackManager::LoadFromStream(ECStreamReader& stream)
{
	ClearAll();

	uint32_t count = 0;
	ECSavegameHelper::ReadStream(stream, count, false);

	DEBUG_LOG("[StackMgr] LoadFromStream: totalEntries=%u\n", count);

	if (count == 0 || count > 10000)
		return;

	m_pendingUIDs.clear();
	m_pendingUIDs.reserve(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		int stackId = 0;
		ECSavegameHelper::ReadStream(stream, stackId, false);

		int uid = 0;
		ECSavegameHelper::ReadStream(stream, uid, false);

		m_pendingUIDs.push_back({ stackId, uid });
	}
}

// ============================================================
// FinalSwizzle - WIC 尚未就绪，不做任何操作
// 在 AfterLoadGame 中调用 RebuildFromUIDs
// ============================================================
void StackManager::FinalSwizzle()
{
	// WIC 的 GetExtendData 在此阶段不可用，跳过
	DEBUG_LOG("[StackMgr] FinalSwizzle: deferred to AfterLoadGame (%zu pending)\n",
		m_pendingUIDs.size());
}

// ============================================================
// TryRebuild - 在 AfterLoadGame 中调用，SEH 保护直接尝试
// 如果 WIC 就绪则立即重建，否则设标记等主循环兜底
// ============================================================
void StackManager::TryRebuild()
{
	if (!m_pendingUIDs.empty())
	{
		__try
		{
			RebuildFromUIDs();
			DEBUG_LOG("[StackMgr] TryRebuild: immediate rebuild SUCCEEDED\n");
			m_pendingRebuild = false;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			// WIC 内部未完全就绪（Swizzle 未生效），设标记等主循环兜底
			DEBUG_LOG("[StackMgr] TryRebuild: immediate FAILED, deferring to MainLoop hook\n");
			m_pendingRebuild = true;
		}
	}
	else
	{
		DEBUG_LOG("[StackMgr] TryRebuild: no pending UIDs, nothing to do\n");
	}
}

// ============================================================
// Update - 由 Syringe MainLoop 钩子每帧调用（仅兜底）
// 仅当 TryRebuild 失败（m_pendingRebuild == true）时才有实质工作
// ============================================================
void StackManager::Update()
{
	if (m_pendingRebuild)
	{
		DEBUG_LOG("[StackMgr] Update: fallback rebuild triggered\n");
		RebuildFromUIDs();
	}
}

// ============================================================
// RebuildFromUIDs - 实际重建逻辑（WIC 完全就绪时调用）
// 分两遍：
//   第一遍：ForEach_Techno 建立 UID → SIBuffClass* 映射
//   第二遍：按 m_pendingUIDs 顺序（= 原始入栈顺序）查找并 push
// 保证栈顺序不变
// ============================================================
struct RebuildFromUIDsCtx {
	std::map<int, SIBuffClass*>* UidToBuff;
	int TotalBuffs;
};

static bool CALLBACK RebuildFromUIDsCB(void* param, SIInterface_ExtendData* ext)
{
	auto* ctx = static_cast<RebuildFromUIDsCtx*>(param);
	if (!ext || !ctx->UidToBuff)
		return false;

	for (int j = 0;; ++j)
	{
		SIBuffClass* buff = ext->Buff_GetBuff(j);
		if (!buff)
			break;

		// 只记录我们关心的 UID（在 m_pendingUIDs 中出现的）
		if (ctx->UidToBuff->find(buff->UID) != ctx->UidToBuff->end())
		{
			(*ctx->UidToBuff)[buff->UID] = buff;
			ctx->TotalBuffs++;
		}
	}
	return false;
}

void StackManager::RebuildFromUIDs()
{
	if (m_pendingUIDs.empty())
	{
		DEBUG_LOG("[StackMgr] RebuildFromUIDs: no pending UIDs\n");
		m_pendingRebuild = false;
		return;
	}

	// 第一遍：建立 UID → SIBuffClass* 映射（用 m_pendingUIDs 中的 UID 初始化）
	std::map<int, SIBuffClass*> uidToBuff;
	for (const auto& [stackId, uid] : m_pendingUIDs)
		uidToBuff[uid] = nullptr;

	RebuildFromUIDsCtx ctx;
	ctx.UidToBuff = &uidToBuff;
	ctx.TotalBuffs = 0;

	SITool::ForEach_Techno(&ctx, RebuildFromUIDsCB);

	DEBUG_LOG("[StackMgr] RebuildFromUIDs: found %d buffs for %u pending entries\n",
		ctx.TotalBuffs, (unsigned)m_pendingUIDs.size());

	// 第二遍：按 m_pendingUIDs 的原始顺序依次 push（保证栈顺序不变）
	int pushed = 0;
	for (const auto& [stackId, uid] : m_pendingUIDs)
	{
		auto it = uidToBuff.find(uid);
		if (it != uidToBuff.end() && it->second != nullptr)
		{
			Push(stackId, it->second);
			pushed++;
		}
		else
		{
			DEBUG_LOG("[StackMgr] RebuildFromUIDs: UID=%d not found (buff may have expired)\n", uid);
		}
	}

	DEBUG_LOG("[StackMgr] RebuildFromUIDs: done  pending=%u  pushed=%d\n",
		(unsigned)m_pendingUIDs.size(), pushed);

	m_pendingUIDs.clear();
	m_pendingRebuild = false;
}
