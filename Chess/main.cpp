#include "state.h"
#include "tree_search.h"
#include "nn.h"

#include <iostream>
#include <random>
#include <ctime>
u64 LEAF_COUNT = 0;
u64 HASH_MISS = 0;
u64 HASH_QUERY = 0;
u64 HASH_COLLISIONS = 0;

void benchmark(u32 depth, State& state)
{
	auto start = std::clock();

	find_best_move(state, depth);

	cout << "time: " << (std::clock() - start) / (double)CLOCKS_PER_SEC << "\n";
	cout << "leaves: " << LEAF_COUNT << "\n";
	cout << "hash_table_queries: " << HASH_QUERY << "\n";
	cout << "hash_table_collisions: " << HASH_COLLISIONS << "\n";
	cout << "hash_table_miss: " << (double)(HASH_MISS) / (double)(HASH_QUERY) << "\n";
	cout << "hash_table_fullness: " << hash_table_fullness() << "\n";
}

void train_nn(State& state)
{
	nn_init();
	train(state, 2, 1000);
	nn_free();
}

int main(int argc, char* argv[])
{
	State state;
	bool success = false;
	if (argc > 1)
	{
		if (state.from_FEN(argv[1]) == false)
		{
			return 0;
		}
	}
	else
	{
		state.init();
	}
	
	//train_nn(state);

	benchmark(8, state);

	//auto str = state.print();
	//auto fen = state.to_FEN();
	//std::cout << str << "\n" << fen << "\n";
	//bool end = false;
	//i16 value = 0;
	//do
	//{
	//	bool has_move = false;

	//	do
	//	{
	//		std::string move_str;
	//		cin >> move_str;

	//		if (move_str[0] != 'z')
	//		{
	//			has_move = state.move(move_str.c_str());
	//		}
	//		else
	//		{
	//			has_move = true;
	//			auto result = find_best_move(state, 8);
	//			if (result.value == MIN_VALUE)
	//			{
	//				cout << "RESIGNS\n";
	//				end = true;
	//			}
	//			else
	//			{
	//				if (state.half_moves & 1)
	//				{
	//					result.value *= -1;
	//				}
	//				value = result.value;
	//				state.move(result.best_move);
	//			}
	//		}
	//	} while (!has_move);
	//	
	//	if (end)
	//	{
	//		break;
	//	}
	//	str = state.print();
	//	fen = state.to_FEN();
	//	std::cout << str << "\n" << fen << "\n" << value << "\n";
	//} while (true);
	return 0;
}