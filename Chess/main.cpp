#include "state.h"
#include "tree_search.h"

#include <iostream>
#include <random>
#include <ctime>

void benchmark(u32 depth)
{
	State state;
	state.init();
	auto start = std::clock();

	find_best_move(state, depth);

	cout << (std::clock() - start) / (double)CLOCKS_PER_SEC;
}
int main()
{
	benchmark(7);
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