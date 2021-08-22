#pragma once
#include "types.h"
#include "config.h"
#include <memory>

#if USE_NEW_TABLE

static const u32 INVALID_LSBS = 0xffffffff;

struct HashTable
{
	struct Entry
	{
		i16 value;
		i8 depth;
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
		++HASH_QUERY;
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
		
		HASH_MISS += set.rr_replace >= ENTRIES_PER_SET;
		u32 index = (set.rr_replace++) & ENTRY_MASK;
		set.lsbs[index] = lsbs;
		Entry& entry = set.entries[index];
		entry.value = 0;
		entry.depth = -1;
		return entry;
	}

	void assign(u64 hash, i16 value, u8 depth)
	{
		u32 key = u32((hash >> 32) & KEY_MASK);
		u32 lsbs = u32(hash);
		Set& set = sets[key];
#if REPLACE_SHALLOWEST
		u32 most_shallow = 0;
		for (u32 i = 0; i < ENTRIES_PER_SET; i++)
		{
			if (set.lsbs[i] == lsbs)
			{
				set.entries[i] = { value, i8(depth) };
				return;
			}
			else if (set.entries[i].depth < set.entries[most_shallow].depth)
			{
				most_shallow = i;
			}
		}

		u32 index = most_shallow;
#else
		for (u32 i = 0; i < ENTRIES_PER_SET; i++)
		{
			if (set.lsbs[i] == lsbs)
			{
				set.entries[i] = { value, i8(depth) };
				return;
			}
		}

		u32 index = (set.rr_replace++) & ENTRY_MASK;
#endif
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
#else

extern u64 HASH_COLLISIONS;

static const u32 INVALID_LSBS = 0xffffffff;

struct HashTable
{
	struct Entry
	{
		i16 value;
		u32 lsbs;
		i8 depth;
#if DEBUG_COLLISIONS
		u8 b[64];
#endif
	};

	Entry entries[ENTRY_COUNT];

	Entry& find_or_create(u64 hash
#if DEBUG_COLLISIONS
						  , const State& state
#endif
						  )
	{
		++HASH_QUERY;
		u32 key = u32((hash >> 32) & KEY_MASK);
		u32 lsbs = u32(hash);
		Entry& entry = entries[key];
		if (lsbs != entry.lsbs)
		{
			HASH_MISS += entry.lsbs != INVALID_LSBS;
			entry.lsbs = lsbs;
			entry.value = 0;
			entry.depth = -1;
		}
#if DEBUG_COLLISIONS
		else if (entry.iteration != 0)
		{
			bool collision = memcmp(entry.b, state.board, 64) != 0;
			HASH_COLLISIONS += collision;
			if (collision)
			{
				std::cout << state.to_FEN();
			}
		}
#endif
		return entry;
	}

	void assign(u64 hash, i16 value, u8 depth
#if DEBUG_COLLISIONS
				, const State& state
#endif				
				)
	{
		u32 key = u32((hash >> 32) & KEY_MASK);
		u32 lsbs = u32(hash);
		entries[key] = { value, lsbs, i8(depth) };
#if DEBUG_COLLISIONS
		memcpy(entries[key].b, state.board, 64) != 0;
#endif
	}

	void clear()
	{
		for (u32 i = 0; i < ENTRY_COUNT; ++i)
		{
			entries[i].lsbs = INVALID_LSBS;
		}
	}
};
#endif
