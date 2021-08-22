#include "kann/kann.h"
#include "state.h"
#include "nn.h"

static const u32 NN_INPUT_SIZE = 64;
static kann_t* nn = nullptr;

void nn_init()
{
	kad_node_t* t;
	t = kann_layer_input(NN_INPUT_SIZE), t->ext_flag |= KANN_F_IN;
	t = kad_sigm(kann_layer_dense(t, 128));
	t = kad_sigm(kann_layer_dense(t, 64));
	t = kad_sigm(kann_layer_dense(t, 32));
	t = kad_sigm(kann_layer_dense(t, 1));
	nn = kann_new(kann_layer_cost(t, 1, KANN_C_MSE), 0);
}

void nn_free()
{
	kann_delete(nn);
}

void get_input_vector(const State& state, float* out)
{
	if (state.half_moves & 1)
	{
		for (u32 i = 0; i < 64; ++i)
		{
			out[63 - i] = float(state.board[i].type()) * (1.5f - float(state.board[i].color())) * 0.25;
		}
	}
	else
	{
		for (u32 i = 0; i < 64; ++i)
		{
			out[i] = float(state.board[i].type()) * (float(state.board[i].color()) - 1.5f) * 0.25;
		}
	}
}

void nn_train(std::vector<TrainingNode>& nodes)
{
	float* xs = new float[NN_INPUT_SIZE * nodes.size()];
	float** px = new float* [nodes.size()];
	float* ys = new float[nodes.size()];
	float** py = new float* [nodes.size()];

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		get_input_vector(nodes[i].state, xs);
		*ys = nodes[i].score;
		px[i] = xs;
		py[i] = ys;
		xs += NN_INPUT_SIZE;
		ys += 1;
	}

	kann_train_fnn1(nn, 0.001, 64, 1, 0, 0, nodes.size(), px, py);
	delete[] * px;
	delete[] px;
}

float nn_evaluate(const State& state)
{
	float x[NN_INPUT_SIZE];
	get_input_vector(state, x);
	return *kann_apply1(nn, x);
}
