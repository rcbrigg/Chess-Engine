#pragma once
#include "state.h"

static const i16 MIN_VALUE = -4000;
static const i16 MAX_VALUE = 4000;

struct search_result
{
	i16 score;
	Move move;
};


i16 min_max_search(State& state, u32 depth, i16 alpha, i16 beta)
{
	if (depth == 0)
	{
		if (state.half_moves & 1)
		{
			return -state.evaluate_position();
		}
		else
		{
			return state.evaluate_position();
		}
	}

	MovePool moves;
	state.get_valid_moves(moves);
	if (moves.count == 0)
	{
		return 0;
	}
	i16 best_value = MIN_VALUE;

	for (u32 i = 0; i < moves.count; i++)
	{
		State new_state = state;
		new_state.move(moves.moves[i]); // TODO: avoid copies by making actions reverasble
		i16 value = -min_max_search(new_state, depth - 1, -beta, -alpha);
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

Move find_best_move(State& state, u32 depth)
{
	MovePool moves;
	state.get_valid_moves(moves);
	assert(moves.count> 0);
	i16 best_value = MIN_VALUE;
	Move best_move = {};
	for (u32 i = 0; i < moves.count; i++)
	{
		State new_state = state;
		new_state.move(moves.moves[i]);
		i16 value = -min_max_search(new_state, depth, MIN_VALUE, MAX_VALUE);
		if (value >= best_value)
		{
			best_value = value;
			best_move = moves.moves[i];
		}
	}
	return best_move;
}


