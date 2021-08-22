#pragma once
#define USE_DETAILED_EVALUATION (1)
#define ORDER_SEARCH (1)
#define USE_NEW_TABLE (1)
#define DEBUG_COLLISIONS (0)

#if USE_NEW_TABLE
#define ENTRIES_PER_SET 4
#define ENTRY_MASK (ENTRIES_PER_SET - 1)
#define KEY_BITS (22)
#define KEY_MASK ((1 << KEY_BITS) - 1)
#define SET_COUNT (1 << KEY_BITS)
#define LSBS (32 - KEY_BITS)
#define LSB_MASK ((1 << LSBS) - 1)
#define REPLACE_SHALLOWEST (0)
#else
#define KEY_BITS 26
#define KEY_MASK ((1 << KEY_BITS) - 1)
#define LSBS 32
#define LSB_MASK ((1 << LSBS) - 1)
#define ENTRY_COUNT (1 << KEY_BITS)
#endif