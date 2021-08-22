#include "types.h"

extern u64 LEAF_COUNT;
extern u64 HASH_MISS;
extern u64 HASH_QUERY;
#include <iostream>
#include "tree_search.h"
#include "hash_table.h"


static HashTable hash_table;

double hash_table_fullness()
{
#if USE_NEW_TABLE
	u64 count = 0;
	for (u32 i = 0; i < SET_COUNT; ++i)
	{
		for (u32 j = 0; j < ENTRIES_PER_SET; ++j)
		{
			count += hash_table.sets[i].lsbs[j] != INVALID_LSBS;
		}
	}
	return double(count) / (SET_COUNT * ENTRIES_PER_SET);
#else
	u64 count = 0;
	for (u32 i = 0; i < ENTRY_COUNT; ++i)
	{
		count += hash_table.entries[i].lsbs != INVALID_LSBS;
	}
	return double(count) / ENTRY_COUNT;
#endif
}

struct MoveHashValue
{
	Move move;
	i16 value;
	u64 hash;
};

void extract_move_hash_values(MoveHashValue move_hash_values[], const MovePool& moves, const State& state, i16& best_value, u32& count, u8 depth)
{
	count = 0;
	for (u32 i = 0; i < moves.count; i++)
	{
		Move move = moves.moves[i];
		u64 hash = state.hash_move(move);

#if DEBUG_COLLISIONS
		State new_state = state;
		new_state.move(move, hash);
#endif
		HashTable::Entry& entry = hash_table.find_or_create(hash
#if DEBUG_COLLISIONS
															, new_state
#endif
															);
		if (entry.depth >= depth)
		{
			// already visited this position at a higher point in the tree
			if (entry.value > best_value)
			{
				best_value = entry.value;
			}
			continue;
		}
		move_hash_values[count++] = { move, entry.value, hash };
	}
}

MoveHashValue take_next_best_move(MoveHashValue move_hash_values[], u32& count)
{
	u32 best = 0;
	for (u32 j = 1; j < count; j++)
	{
		if (move_hash_values[best].value < move_hash_values[j].value)
		{
			best = j;
		}
	}

	MoveHashValue best_move = move_hash_values[best];
	move_hash_values[best] = move_hash_values[--count];
	return best_move;
}

i16 min_max_search_final(State& state, i16 alpha, i16 beta)
{
	MovePool moves;
	state.get_moves(moves);


	if (moves.king_captures)
	{
		// This is either checkmate or the previous move was illegal.
		// Either way we should stop searching this branch.
		return MAX_VALUE;
	}

	if (moves.count == 0)
	{
		return 0;
	}

	i16 best_value = MIN_VALUE;
	i16 sign = (state.half_moves & 1) ? -1 : 1;

	for (u32 i = 0; i < moves.count; i++)
	{
		Move move = moves.moves[i];
		State::Undo undo = { move, state.fifty_move_clock, state.board[move.to], state.hash };
		state.move(move);
		i16 value = state.evaluate_position() * sign;
		hash_table.assign(state.hash, value, 0
#if DEBUG_COLLISIONS
						  , state
#endif
						  );
		state.unmove(undo);

		++LEAF_COUNT;

		if (value > best_value)
		{
			best_value = value;
			if (value > alpha)
			{
				alpha = value;
				if (alpha >= beta)
				{
					break;
				}
			}
		}
	}

	return best_value;
}


SreachResult min_max_search(State& state, i16 alpha, i16 beta, u8 depth)
{
	Move best_move;
#ifdef _DEBUG
	best_move = { 0xff, 0xff };
#endif

	MovePool moves;
	state.get_moves(moves);

	if (moves.king_captures)
	{
		// This is either checkmate or the previous move was illegal.
		// Either way we should stop searching this branch.
		return { {0,0}, MAX_VALUE };
	}

	if (moves.count == 0)
	{
		return { {0, 0}, 0 };
	}

	i16 best_value = MIN_VALUE;
	u32 count;

	MoveHashValue move_hash_values[MovePool::MAX_MOVES];
	extract_move_hash_values(move_hash_values, moves, state, best_value, count, depth);

	if (best_value > alpha)
	{
		alpha = best_value;
		if (alpha >= beta)
		{
			return { {0, 0}, best_value };
		}
	}

	while (count > 0)
	{
#if ORDER_SEARCH
		MoveHashValue mhv = take_next_best_move(move_hash_values, count);
#else
		MoveHashValue mhv = move_hash_values[--count];
#endif
		State::Undo undo = { mhv.move, state.fifty_move_clock, state.board[mhv.move.to], state.hash };
		state.move(mhv.move, mhv.hash);
		i16 value;

		if (depth == 1)
		{
			value = -min_max_search_final(state, -beta, -alpha);
		}
		else
		{
			value = -min_max_search(state, -beta, -alpha, depth - 1).value;
		}

		hash_table.assign(state.hash, value, depth
#if DEBUG_COLLISIONS
						  , state
#endif
						  );
		state.unmove(undo);

		if (value > best_value)
		{
			best_value = value;
			best_move = mhv.move;
			if (best_value > alpha)
			{
				alpha = value;
				if (alpha >= beta)
				{
					break;
				}
			}
		}
	}

	return { best_move, best_value };
}

SreachResult find_best_move(State& state, u32 depth)
{
	assert(depth > 1);

#if _DEBUG
	u64 hash = state.hash;
#endif

	hash_table.clear();
	min_max_search_final(state, MIN_VALUE, MAX_VALUE);
	SreachResult result;
	for (u32 i = 1; i < depth; ++i)
	{
		result = min_max_search(state, MIN_VALUE, MAX_VALUE, i);
	}
	assert(hash == state.hash);
	return result;
}