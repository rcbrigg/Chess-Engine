#pragma once
#include "types.h"
#include <memory>

#define KEY_BITS 24
#define KEY_MASK ((1 << KEY_BITS) - 1)
#define ENTRY_COUNT (1 << KEY_BITS)
static const i16 INVALID_MSBS = 0x100;

struct HashTable
{
	struct Entry
	{
		i16 value;
		u16 msbs : 9;
		u16 search_depth : 5;
	};

	Entry entries[ENTRY_COUNT];

	Entry& find_or_create(u32 hash)
	{
		++HASH_QUERY;
		u32 key = hash & KEY_MASK;
		u8 msbs = u8(hash >> KEY_BITS);
		Entry& entry = entries[key];
		if (msbs != entry.msbs)
		{
			assert(entry.msbs != INVALID_MSBS);
			++HASH_MISS;
			entry.msbs = msbs;
			entry.value = 0;
			entry.search_depth = 0;
		}
		return entry;
	}

	void assign(u32 hash, i16 value)
	{
		u32 key = hash & KEY_MASK;
		u8 msbs = u8(hash >> KEY_BITS);
		entries[key] = { value, msbs, u16(search_depth) };
	}

	void clear()
	{
		for (u32 i = 0; i < ENTRY_COUNT; ++i)
		{
			entries[i].value = 0;
			entries[i].msbs = INVALID_MSBS;
		}
	}
};
//#define SET_SIZE 8
//#define KEY_BITS 20
//#define KEY_SHIFT (32 - KEY_BITS)
//#define SET_COUNT (1 << KEY_BITS)
//#define LSB_MASK (KEY_SHIFT - 1)
//#define EMPTY_ENTRY 0xffff;
//
//__declspec(align(256))
//struct HashTable
//{
//	struct Set
//	{
//		u16 key_lsbs[SET_SIZE];
//		u16 values[SET_SIZE];
//	};
//
//	struct Ref
//	{
//		u16 value;
//		u8 way;
//		bool valid() { return way != 0xff; }
//	};
//
//	Set sets[SET_COUNT];
//
//	Ref Find(u32 hash)
//	{
//		Ref ref;
//		u32 key = hash >> KEY_SHIFT;
//		u16 lsbs = hash & LSB_MASK;
//		Set& set = sets[key];
//		for (u32 i = 0; i < SET_SIZE; i++)
//		{
//			if (set.key_lsbs[i] == lsbs)
//			{
//				ref.way = i;
//				ref.value = set.values[i];
//				return ref;
//			}
//		}
//		ref.way = 0xff;
//		return ref;
//	}
//
//	void Insert(u32 hash, u16 value)
//	{
//		u32 key = hash >> KEY_SHIFT;
//		u16 lsbs = hash & LSB_MASK;
//		Set& set = sets[key];
//		for (u32 i = 0; i < SET_SIZE; i++)
//		{
//			if (set.key_lsbs[i] == lsbs)
//			{
//				ref.way = i;
//				ref.value = set.values[i];
//				return ref;
//			}
//		}
//		ref.way = 0xff;
//		return ref;
//	}
//
//	void Init()
//	{
//		memset(&sets, 0xff, sizeof(sets));
//	}
//};
