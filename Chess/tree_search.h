#pragma once
#include "state.h"

static const i16 MIN_VALUE = -4000;
static const i16 MAX_VALUE = 4000;
static const u32 MAX_DEPTH = 32;

struct SreachResult
{
	Move best_move;
	i16 value;
};

void train(const State& starting_state, u32 generations, u32 games_per_generation);

SreachResult find_best_move(State& state, u32 depth);

double hash_table_fullness();