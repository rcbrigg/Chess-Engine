#include "state.h"
#include "tree_search.h"

#include <iostream>
#include <random>
#include <ctime>
u64 LEAF_COUNT = 0;
u64 HASH_MISS = 0;
u64 HASH_QUERY = 0;

void benchmark(u32 depth)
{
	State state;
	state.init();
	auto start = std::clock();

	find_best_move(state, depth);

	cout << "time: " << (std::clock() - start) / (double)CLOCKS_PER_SEC << "\n";
	cout << "leaves: " << LEAF_COUNT << "\n";
	cout << "hash_table_queries: " << HASH_QUERY << "\n";
	cout << "hash_table_hit: " << (double)(HASH_MISS) / (double)(HASH_QUERY) << "\n";
	cout << "hash_table_fullness: " << hash_table_fullness() << "\n";
}

int main()
{
	benchmark(9);
	//State state;
	//state.init();
	//auto str = state.print();
	//auto fen = state.to_FEN();
	//std::cout << str << "\n" << fen << "\n";
	//for (int i = 0; i < 100; i++)
	//{
	//	cin.ignore();
	//	auto move = find_best_move(state, 3);
	//	state.move(move);
	//	str = state.print();
	//	fen = state.to_FEN();
	//	std::cout << str << "\n" << fen << "\n";
	//}
	return 0;
}