#include "types.h"

static u32 search_depth;
extern u64 LEAF_COUNT;
extern u64 HASH_MISS;
extern u64 HASH_QUERY;

#include "tree_search.h"
#include "hash_table.h"

static const i16 MIN_VALUE = -4000;
static const i16 MAX_VALUE = 4000;
static const u32 MAX_DEPTH = 32;

static HashTable hash_table;

double hash_table_fullness()
{
	u64 count = 0;
	for (u32 i = 0; i < ENTRY_COUNT; ++i)
	{
		count += hash_table.entries[i].msbs != INVALID_MSBS;
	}
	return double(count) / ENTRY_COUNT;
}
struct Stack
{
	struct Entry
	{
		//u32 hash;
		State::Undo undo;
		//u8 castle_rights;
		//u8 en_passant;
	};

	u32 depth;

	Entry& push()
	{
		assert(depth < MAX_DEPTH);
		return stack[depth++];
	}

	Entry& pop()
	{
		assert(depth > 0);
		return stack[--depth];
	}

	void init()
	{
		depth = 0;
	}

	Entry stack[MAX_DEPTH];

};

static Stack stack;

struct SreachResult
{
	Move best_move;
	i16 value;
};

struct MoveHashValue
{
	Move move;
	i16 value;
	u32 hash;
};

void extract_move_hash_values(MoveHashValue move_hash_values[], const MovePool& moves, const State& state, i16& best_value, u32& count)
{
	count = 0;
	for (u32 i = 0; i < moves.count; i++)
	{
		Move move = moves.moves[i];
		u32 hash = state.hash_move(move);
		HashTable::Entry& entry = hash_table.find_or_create(hash);
		if (entry.search_depth == search_depth)
		{
			// already visited this position on this iteration
			if (entry.value < best_value)
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
	state.get_valid_moves(moves);
	if (moves.count == 0)
	{
		return 0;
	}

	i16 best_value = MIN_VALUE;
	i16 sign = (state.half_moves & 1) ? -1 : 1;

	for (u32 i = 0; i < moves.count; i++)
	{
		Move move = moves.moves[i];
		stack.push() = { State::Undo{ move, state.board[move.to], state.hash } };
		state.move(move);
		i16 value = state.evaluate_position() * sign;
		hash_table.assign(state.hash, value);
		state.unmove(stack.pop().undo);
		
		++LEAF_COUNT;

		if (value > best_value)
		{
			best_value = value;
			if (value > alpha)
			{
				alpha = value;
				if (alpha >= beta)
				{
					//break;
				}
			}
		}
	}
	return best_value;
}


SreachResult min_max_search(State& state, i16 alpha, i16 beta)
{
	MovePool moves;
	state.get_valid_moves(moves);
	if (moves.count == 0)
	{
		return { {0, 0}, 0 };
	}

	i16 best_value = MIN_VALUE;
	u32 count;

	MoveHashValue move_hash_values[MovePool::MAX_MOVES];
	extract_move_hash_values(move_hash_values, moves, state, best_value, count);
	assert(best_value <= alpha);
	
	Move best_move;

#ifdef _DEBUG
	best_move = { 0xff, 0xff };
#endif

	while (count > 0)
	{
		MoveHashValue mhv = take_next_best_move(move_hash_values, count);
		stack.push() = { State::Undo{ mhv.move, state.board[mhv.move.to], state.hash } };
		state.move(mhv.move, mhv.hash);
		i16 value;

		if (stack.depth == search_depth - 1)
		{
			value = -min_max_search_final(state, -beta, -alpha);
		}
		else
		{
			value = -min_max_search(state, -beta, -alpha).value;
		}

		state.unmove(stack.pop().undo);

		hash_table.assign(state.hash, value);

		if (value > best_value)
		{
			best_value = value;
			best_move = mhv.move;
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
	return { best_move, best_value };
}

Move find_best_move(State& state, u32 depth)
{
	assert(depth > 1);

#if _DEBUG
	u32 hash = state.hash;
#endif

	hash_table.clear();
	stack.init();
	min_max_search_final(state, MIN_VALUE, MAX_VALUE);
	for (search_depth = 2; search_depth < depth; ++search_depth)
	{
		min_max_search(state, MIN_VALUE, MAX_VALUE).best_move;
	}
	search_depth = depth;
	Move best_move = min_max_search(state, MIN_VALUE, MAX_VALUE).best_move;
	assert(hash == state.hash);
	return best_move;
}