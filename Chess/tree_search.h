#pragma once
#include "state.h"

Move find_best_move(State& state, u32 depth);

double hash_table_fullness();