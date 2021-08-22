#include "types.h"
#include "nn.h"

#include <vector>

static std::vector<TrainingNode> training_nodes;
static double gamma = 0;

struct MoveHashValue
{
	float value;
	u64 hash;
};

void extract_move_hash_values(MoveHashValue move_hash_values[], const MovePool& moves, const State& state)
{
	for (u32 i = 0; i < moves.count; ++i)
	{
		Move move = moves.moves[i];
		State new_state = state;
		new_state.move(move);
		float value = nn_evaluate(new_state);
		move_hash_values[i] = { value, new_state.hash };
	}
}

u32 select_move(MoveHashValue move_hash_values[], u32 count)
{
	double reweighting[MovePool::MAX_MOVES];
	double total = 0;

	for (u32 i = 0; i < count; ++i)
	{
		double value = pow(double(move_hash_values[i].value + 1), gamma);
		reweighting[i] = value;
		total += value;
	}

	double threshold = total * (std::rand() / RAND_MAX);
	double acc = 0;
	for (u32 i = 0; i < count; ++i)
	{
		acc += reweighting[i];
		if (acc >= threshold)
		{
			return i;
		}
	}

	// only here due to very rare floating point rounding errors
	return count - 1;
}

void monte_carlo_play(State& state)
{
	if (state.fifty_move_clock > 2 * 50)
	{
		training_nodes.emplace_back(TrainingNode{ state, 0 });
		return;
	}

	MovePool moves;

	state.get_moves(moves);

	if (moves.king_captures)
	{
		training_nodes.emplace_back(TrainingNode{ state, 1 });
		return;
	}

	MoveHashValue move_hash_values[MovePool::MAX_MOVES];

	extract_move_hash_values(move_hash_values, moves, state);

	u32 i = select_move(move_hash_values, moves.count);

	State::Undo undo{ moves.moves[i], state.fifty_move_clock, state.board[moves.moves[i].to], state.hash };

	state.move(moves.moves[i], move_hash_values[i].hash);

	monte_carlo_play(state);

	training_nodes.emplace_back(TrainingNode{ state, -training_nodes.back().score });

	state.unmove(undo);
}

void train(const State& starting_state, u32 generations, u32 games_per_generation)
{
	State state = starting_state;

	for (u32 i = 0; i < generations; ++i)
	{
		for (u32 j = 0; j < games_per_generation; ++j)
		{
			monte_carlo_play(state);
		}
		nn_train(training_nodes);
		training_nodes.clear();
	}
}