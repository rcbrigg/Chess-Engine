#pragma once
#include "state.h"
#include <vector>

struct TrainingNode
{
	State state;
	//u32 visit_count;
	float score;
};

void nn_init();
void nn_free();
void nn_train(std::vector<TrainingNode>& nodes);
float nn_evaluate(const State& state);