#pragma once
#include "types.h"
#include <memory>

#define ENTRIES_PER_SET 8
#define ENTRY_MASK (ENTRIES_PER_SET - 1)
#define KEY_BITS (23)
#define KEY_MASK ((1 << KEY_BITS) - 1)
#define SET_COUNT (1 << KEY_BITS)
#define LSBS (32 - KEY_BITS)
#define LSB_MASK ((1 << LSBS) - 1)
#define REPLACE_SHALLOWEST (1)

static const u32 INVALID_LSBS = 0xffffffff;

struct HashTable
{
	struct Entry
	{
		u32 training_node_index;
		float nn_value;
	};

	__declspec(align(64))
		struct Set
	{
		u32 lsbs[ENTRIES_PER_SET];
		Entry entries[ENTRIES_PER_SET];
		u8 rr_replace;
	};

	Set sets[SET_COUNT];

	Entry& find_or_create(u64 hash)
	{
		u32 key = u32((hash >> 32) & KEY_MASK);
		u32 lsbs = u32(hash);
		Set& set = sets[key];
		for (u32 i = 0; i < ENTRIES_PER_SET; i++)
		{
			if (set.lsbs[i] == lsbs)
			{
				return set.entries[i];
			}
		}

		u32 index = (set.rr_replace++) & ENTRY_MASK;
		set.lsbs[index] = lsbs;
		Entry& entry = set.entries[index];
		return entry;
	}

	void assign(u64 hash, float nn_value, u32 training_node_index)
	{
		u32 key = u32((hash >> 32) & KEY_MASK);
		u32 lsbs = u32(hash);
		Set& set = sets[key];

		for (u32 i = 0; i < ENTRIES_PER_SET; i++)
		{
			if (set.lsbs[i] == lsbs)
			{
				set.entries[i] = { value, i8(depth) };
				return;
			}
		}

		u32 index = (set.rr_replace++) & ENTRY_MASK;
		set.lsbs[index] = lsbs;
		set.entries[index] = { value, i8(depth) };
	}

	void clear()
	{
		for (u32 i = 0; i < SET_COUNT; ++i)
		{
			for (u32 j = 0; j < ENTRIES_PER_SET; ++j)
			{
				sets[i].lsbs[j] = INVALID_LSBS;
				sets[i].entries[j].depth = -2;
			}
			sets[i].rr_replace = 0;
		}
	}
};
