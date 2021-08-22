#include "state.h"
#include "config.h"

static const u64 row_multiplyers[8] =
{
	14111800228027138213,
	5337243228668701261,
	1609124470030152641,
	3634829627332226093,
	13609117125809544071,
	12055898888697354991,
	8017885172068145449,
	8357446957669507043
};

void hash_row(u32 y, u64 row, u64& hash)
{
	u64 row_hash = ((row ^ (row >> 19)) * row_multiplyers[y]) >> 8;
	hash ^= row_hash;
}

inline void hash_piece(State::Piece piece, u8 square, const State::Piece board[], u64& hash)
{
	u64 row = *(u64*)(&board[square & ~0x7]);
	hash_row(square / 8, row, hash);
	((u8*)(&row))[square & 0x7] = piece.bits;
	hash_row(square / 8, row, hash);
}


inline void move_piece_in_row(State::Piece piece, u8 to, u8 from, const State::Piece board[], u64& hash)
{
	u64 row = *(u64*)(&board[to & ~0x7]);
	hash_row(to / 8, row, hash);
	((u8*)(&row))[to & 0x7] = piece.bits;
	((u8*)(&row))[from & 0x7] = 0;
	hash_row(to / 8, row, hash);
}

void State::init()
{
	if (from_FEN(start_FEN) == false)
	{
		printf("Bad FEN!");
	}
}

const char* State::print() const
{
	static u8 str[9][19];
	str[0][0] = ' ';
	str[0][1] = ' ';
	for (u8 j = 0; j < 8; j++)
	{
		str[0][2 * (j + 1)] = 'a' + j;
		str[0][2 * (j + 1) + 1] = ' ';
	}

	str[0][18] = '\n';

	for (u8 i = 0; i < 8; i++)
	{
		str[i + 1][0] = '8' - i;
		str[i + 1][1] = ' ';
		for (u8 j = 0; j < 8; j++)
		{
			str[i+1][2 * (j + 1)] = piece_to_char[board[8 * i + j].bits >> 1];
			str[i+1][2 * (j + 1) + 1] = ' ';
		}
		str[i][18] = '\n';
	}
	return (char*)str;
}

template<class Op>
void process_pawn_attack(u8 current, u8 target, const State& state, Op& op)
{
	if (!state.board[target].empty() && state.board[target].color() != state.board[current].color())
	{
		op.add({ current, target }, state, true);
	}
	else if (target == state.en_passant)
	{
		// TODO:
		assert(0);
		//analysis.add({ Move::ENPASSANT }, *this);
	}
	else
	{
		return;
	}
}

template<class Op>
void process_pawn(u8 current, const State& state, Op& op)
{
	u8 advance = current + state.forward;
	u8 x = current % 8;
	u8 y = current / 8;
	if (u8(current + state.forward) < 64)
	{
		if (state.board[current + state.forward].empty())
		{
			op.add({ current, advance }, state, false);
			u8 advance_2 = advance + state.forward;

			// If pawn is on starting rank it can move two squares
			if (u16(y - (state.forward / 4)) >= 8 && state.board[advance_2].empty())
			{
				op.add({ current, advance }, state, false);
			}
		}

		if (x - 1 >= 0)
		{
			process_pawn_attack(current, advance - 1, state, op);
		}
		if (x + 1 < 8)
		{
			process_pawn_attack(current, advance + 1, state, op);
		}
	}
}

template<class Op>
void process_range(u8 current, u8 x, u8 y, u8 dx, u8 dy, const State& state, Op& op)
{
	i8 ds = dx + 8 * dy;
	u8 target = current;

	while (true)
	{
		x += dx;
		y += dy;
		target += ds;

		if ((x | y) >= 8)
		{
			break;
		}

		op.add({ current, target }, state, true);

		if (!state.board[target].empty())
		{
			break;
		}
	}
}

template<class Op>
void process_move_if_valid(u8 current, u8 x, u8 y, const State& state, Op& op)
{
	if ((x | y) < 8)
	{
		u8 target = x + 8 * y;
		if (state.board[target].empty() || state.board[current].color() != state.board[target].color())
		{
			op.add({ current, x + 8u * y }, state, true);
		}
	}
}

template<class Op>
void process_knight(u8 current, const State& state, Op& op)
{
	u8 x = current % 8;
	u8 y = current / 8;
	process_move_if_valid(current, x - 1, y - 2, state, op);
	process_move_if_valid(current, x + 1, y - 2, state, op);
	process_move_if_valid(current, x - 1, y + 2, state, op);
	process_move_if_valid(current, x + 1, y + 2, state, op);
	process_move_if_valid(current, x - 2, y - 1, state, op);
	process_move_if_valid(current, x + 2, y - 1, state, op);
	process_move_if_valid(current, x - 2, y + 1, state, op);
	process_move_if_valid(current, x + 2, y + 1, state, op);
}

template<class Op>
void process_bishop(u8 current, const State& state, Op& op)
{
	u8 x = current % 8;
	u8 y = current / 8;
	process_range(current, x, y, -1, -1, state, op);
	process_range(current, x, y, 1, -1, state, op);
	process_range(current, x, y, -1, 1, state, op);
	process_range(current, x, y, 1, 1, state, op);
}

template<class Op>
void process_rook(u8 current, const State& state, Op& op)
{
	u8 x = current % 8;
	u8 y = current / 8;
	process_range(current, x, y, -1, 0, state, op);
	process_range(current, x, y, 1, 0, state, op);
	process_range(current, x, y, 0, -1, state, op);
	process_range(current, x, y, 0, 1, state, op);
}

template<class Op>
void process_queen(u8 current, const State& state, Op& op)
{
	process_rook(current, state, op);
	process_bishop(current, state, op);
}

template<class Op>
void process_king(u8 current, const State& state, Op& op)
{
	u8 x = current % 8;
	u8 y = current / 8;
	for (i8 i = -1; i <= 1; ++i)
	{
		if (u8(y + i) < 8)
		{
			for (i8 j = -1; j <= 1; ++j)
			{
				if (u8(x + j) < 8)
				{
					u8 target = (x + j) + 8 * (y + i);
					if (state.board[target].empty() || state.board[current].color() != state.board[target].color())
					{
						assert(target < 64);
						op.add({ current, target }, state, true);
					}
				}
			}
		}
	}

	// TODO: King safety
}

bool State::move(const char* str)
{
	try
	{
		u8 from = square_from_notation(str);
		u8 to = square_from_notation(str + 2);
		move({ from, to });
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void State::get_moves(MovePool& moves) const
{
	u8 color = (half_moves & 0x1) + 1;
	for (u8 i = 0; i < 64; ++i)
	{
		if (board[i].color() == color)
		{
			switch (board[i].type())
			{
			case P: process_pawn(i, *this, moves); break;
			case N: process_knight(i, *this, moves); break;
			case B: process_bishop(i, *this, moves); break;
			case R: process_rook(i, *this, moves); break;
			case Q: process_queen(i, *this, moves); break;
			case K: process_king(i, *this, moves); break;
			default: break;
			}
		}
	}
}

i16 State::evaluate_position() const
{
#if USE_DETAILED_EVALUATION
	PositionAnalysis analysis;

	for (u8 i = 0; i < 64; ++i)
	{
		if (!board[i].empty())
		{
			analysis.score += get_piece_value(board[i]);
			switch (board[i].type())
			{
			case P: process_pawn(i, *this, analysis); break;
			case N: process_knight(i, *this, analysis); break;
			case B: process_bishop(i, *this, analysis); break;
			case R: process_rook(i, *this, analysis); break;
			case Q: process_queen(i, *this, analysis); break;
			case K: process_king(i, *this, analysis); break;
			default: break;
			}
		}
	}

	return analysis.score;
#else#
	return value;
#endif
}

i16 State::get_piece_value(Piece piece) const
{
	static const i16 piece_scores[] = { 0, 0, 100, -100, 300, -300, 300, -300, 500, -500, 900, -900, 2000, -2000 };
	return piece_scores[piece.bits >> 1];
}

void State::move(Move move)
{
	u64 hash = hash_move(move);
	State::move(move, hash);
}

void State::move(Move move, u64 new_hash)
{
	assert((move.from | move.to) < 64);

	if (move.bits & Move::SPECIAL_MASK)
	{
		assert(0); // TODO
		//if (special == Move::CASTLE)
		//{
		//	auto castle = move.bits >> 14;
		//	if (castle == WK)
		//	{
		//		add_piece({ WHITE | K }, 62);
		//		add_piece({ WHITE | R }, 61);
		//		remove_piece(60);
		//		remove_piece(63);
		//	}
		//	else if (castle == WQ)
		//	{
		//		add_piece({ WHITE | K }, 58);
		//		add_piece({ WHITE | R }, 59);
		//		remove_piece(60);
		//		remove_piece(56);
		//	}
		//	else if (castle == BK)
		//	{
		//		add_piece({ BLACK | K }, 6);
		//		add_piece({ BLACK | R }, 5);
		//		remove_piece(4);
		//		remove_piece(7);
		//	}
		//	else // (castle == BQ)
		//	{
		//		add_piece({ BLACK | K }, 2);
		//		add_piece({ BLACK | R }, 3);
		//		remove_piece(4);
		//		remove_piece(0);
		//	}
		//}
		//else if (special == Move::ENPASSANT)
		//{
		//	add_piece(piece, move.to);
		//	remove_piece(move.from);
		//	u8 x = move.to % 8;
		//	u8 y = (move.to / 8) - forward;
		//	remove_piece(x + 8 * y);
		//	capture = true;
		//	fifty_move_clock = 0;
		//}
		//else // (special == Move::PROMOTION)
		//{
		//	remove_piece(move.from);
		//	remove_piece(move.to);
		//	add_piece({ ((move.bits >> 13) + 1u) | color }, move.to);
		//}
		//set_en_passant(0xff);
	}
	else
	{
#if !USE_DETAILED_EVALUATION
		value -= get_piece_value(board[move.to]);
#endif
		board[move.to] = board[move.from];
		board[move.from].bits = 0;

		if (board[move.from].type() == P || !board[move.to].empty())
		{
			fifty_move_clock = 0;
		}
		//set_en_passant(0xff);

		//if (piece.type() == P)
		//{
		//	fifty_move_clock = 0;

		//	u8 y_current = move.from / 8;
		//	u8 y_to = move.to / 8;

		//	if (color == WHITE)
		//	{
		//		if (y_current == 6 && y_to == 4)
		//		{
		//			set_en_passant(40 + (move.to / 8));
		//		}
		//	}
		//	else
		//	{
		//		if (y_current == 1 && y_to == 3)
		//		{
		//			set_en_passant(en_passant = 16 + (move.to / 8));
		//		}
		//	}
		//}
	}

	half_moves += 1;
	hash = new_hash;
	fifty_move_clock += 1;
	forward *= -1;
}

void State::unmove(Undo& undo)
{
	assert((undo.move.from | undo.move.to) < 64);

	if (undo.move.bits & Move::SPECIAL_MASK)
	{
		assert(0); // TODO
		//if (special == Move::CASTLE)
		//{
		//	auto castle = move.bits >> 14;
		//	if (castle == WK)
		//	{
		//		remove_piece(62);
		//		remove_piece(61);
		//		add_piece({ WHITE | K }, 60);
		//		add_piece({ WHITE | R }, 63);
		//	}
		//	else if (castle == WQ)
		//	{
		//		remove_piece(58);
		//		remove_piece(59);
		//		add_piece({ WHITE | K }, 60);
		//		add_piece({ WHITE | R }, 56);				
		//	}
		//	else if (castle == BK)
		//	{
		//		remove_piece(6);
		//		remove_piece(5);
		//		add_piece({ BLACK | K }, 4);
		//		add_piece({ BLACK | R }, 7);
		//		
		//	}
		//	else // (castle == BQ)
		//	{
		//		remove_piece(2);
		//		remove_piece(3);
		//		add_piece({ BLACK | K }, 4);
		//		add_piece({ BLACK | R }, 0);				
		//	}
		//}
		//else if (special == Move::ENPASSANT)
		//{
		//	add_piece(piece, move.to);
		//	remove_piece(move.from);
		//	u8 x = move.to % 8;
		//	u8 y = (move.to / 8) - forward;
		//	remove_piece(x + 8 * y);
		//	capture = true;
		//	fifty_move_clock = 0;
		//}
		//else // (special == Move::PROMOTION)
		//{
		//	remove_piece(move.from);
		//	remove_piece(move.to);
		//	add_piece({ ((move.bits >> 13) + 1u) | color }, move.to);
		//}

		//set_en_passant(0xff);
	}
	else
	{
#if !USE_DETAILED_EVALUATION
		value += get_piece_value(undo.captured_piece);
#endif
		board[undo.move.from] = board[undo.move.to];
		board[undo.move.to] = undo.captured_piece;
	}

	//en_passant = old_state.en_passant;
	//castle_rights = old_castle_rights;
	fifty_move_clock = undo.fifty_move_clock;
	hash = undo.hash;
	half_moves -= 1;
	forward *= -1;
}

u64 State::hash_move(Move move) const
{
	assert((move.from | move.to) < 64);
	u64 new_hash = hash;

	if (move.bits & Move::SPECIAL_MASK)
	{
		assert(0); // TODO
	}
	else
	{
		if ((move.from / 8) == (move.to / 8))
		{
			move_piece_in_row(board[move.from], move.to, move.from, board, new_hash);
		}
		else
		{
			hash_piece(board[move.from], move.to, board, new_hash);
			hash_piece({ 0 }, move.from, board, new_hash);
		}
	}
	return ~new_hash;
}

bool State::from_FEN(string FEN)
{
	try
	{
		hash = 0;
		value = 0;
		memset(board, 0, sizeof(board));
		u32 i = 0;
		u8 square = 0;
		bool new_row = true;
		while (true)
		{
			if (square % 8 == 0 && !new_row)
			{
				hash_row((square / 8)-1, *(u64*)(&board[square & 0x7]), hash);

				if (square == 64)
				{
					if (FEN[i] == '/')
					{
						++i;
					}
					break;
				}
				if (FEN[i] != '/')
				{
					throw std::exception();
				}
				
				new_row = true;
			}
			else
			{
				new_row = false;
				if (FEN[i] >= 'A')
				{
					Piece piece = piece_from_char(FEN[i]);
					//hash_piece(piece, square, board, hash);
					value += get_piece_value(piece);
					board[square++] = piece;
				}
				else
				{
					u8 skip = FEN[i] - '0';
					if (skip == 0 || skip > (8 - (square % 8)))
					{
						throw std::exception();
					}
					square += skip;
					//for (u8 j = 0; j < skip; ++j)
					//{
					//	//hash_piece({ 0 }, square, board, hash);
					//	//board[square++].bits = 0;
					//}
				}
			}
			++i;
		}

		while (FEN[i] == ' ')
		{
			++i;
		}

		u8 turn = FEN[i++];

		if (turn == 'w')
		{
			forward = -8;
			turn = 0;
		}
		else if (turn == 'b')
		{
			forward = 8;
			turn = 1;
		}
		else
		{
			throw std::exception();
		}

		while (FEN[i] == ' ')
		{
			++i;
		}

		castle_rights = 0;
		if (FEN[i] == '-')
		{
			++i;
		}
		else
		{
			if (FEN[i] == 'K')
			{
				i++;
				castle_rights |= 0x1;
			}
			if (FEN[i] == 'Q')
			{
				i++;
				castle_rights |= 0x2;
			}
			if (FEN[i] == 'k')
			{
				i++;
				castle_rights |= 0x4;
			}
			if (FEN[i] == 'q')
			{
				i++;
				castle_rights |= 0x8;
			}
		}

		while (FEN[i] == ' ')
		{
			++i;
		}

		if (FEN[i] == '-')
		{
			en_passant = 0xff;
			++i;
		}
		else
		{
			set_en_passant(square_from_notation(&FEN.data()[i]));
			i += 2;
		}

		if (FEN[i++] != ' ')
		{
			throw std::exception();
		}

		fifty_move_clock = stoi(&FEN.data()[i]);
		i += 1 + (fifty_move_clock / 10);

		if (FEN[i++] != ' ')
		{
			throw std::exception();
		}

		u32 moves = stoi(&FEN.data()[i]);
		i += 1 + u32(log10(moves));
		half_moves = 2 * (moves - 1) + turn;

		if (turn)
		{
			hash = ~hash;
		}
		if (i != FEN.size())
		{
			throw std::exception();
		}

	}
	catch (...)
	{
		return false;
	}

	return true;
}

string State::to_FEN() const
{
	string FEN;

	for (u8 i = 0; i < 8; i++)
	{
		bool last_square_empty = false;
		for (u8 j = 0; j < 8; j++)
		{
			Piece piece = board[8 * i + j];
			if (piece.empty())
			{
				if (last_square_empty)
				{
					++FEN.back();
				}
				else
				{
					FEN.push_back('1');
				}
				last_square_empty = true;
			}
			else
			{
				last_square_empty = false;
				FEN.push_back(piece_to_char[piece.bits >> 1]);
			}
		}
		FEN.push_back('/');
	}

	FEN.push_back(' ');

	FEN.push_back(half_moves % 2 == 0 ? 'w' : 'b');

	FEN.push_back(' ');

	if (castle_rights == 0)
	{
		FEN.push_back('-');
	}
	else
	{
		if (castle_rights & 0x1)
		{
			FEN.push_back('K');
		}
		if (castle_rights & 0x2)
		{
			FEN.push_back('Q');
		}
		if (castle_rights & 0x4)
		{
			FEN.push_back('k');
		}
		if (castle_rights & 0x8)
		{
			FEN.push_back('q');
		}
	}

	FEN.push_back(' ');

	if (en_passant != 0xff)
	{
		FEN.append(square_to_notation(en_passant));
	}
	else
	{
		FEN.push_back('-');
	}

	FEN.push_back(' ');

	FEN.append(to_string(fifty_move_clock));

	FEN.push_back(' ');

	FEN.append(to_string(1 + (half_moves / 2)));

	return FEN;
}




u8 State::square_from_notation(const char* str)
{
	if (str[0] < 'a' || str[0] > 'h' || str[1] < '1' || str[1] > '8')
	{
		throw std::exception();
	}
	return 8 * ('8' - str[1]) + (str[0] - 'a');
}

const char* State::square_to_notation(u8 square) const
{
	static char buffer[2];
	buffer[0] = 'a' + (square % 8);
	buffer[1] = 8 - (square / 8);
	return buffer;
}

void State::set_en_passant(u8 square)
{
	en_passant = square;
	//hash ^= (en_passant + u8(1)) * 0x520381ba;
	//hash ^= (square + u8(1)) * 0x520381ba;
}

void State::set_castle_rights(u8 rights)
{
	castle_rights = rights;
	//hash ^= (castle_rights + 1) * 0xc5f537f6;
	//hash ^= (rights + 1) * 0xc5f537f6;
}

State::Piece State::piece_from_char(char piece)
{
	switch (piece)
	{
	case 'P': return { P | WHITE };
	case 'p': return { P | BLACK };
	case 'N': return { N | WHITE };
	case 'n': return { N | BLACK };
	case 'B': return { B | WHITE };
	case 'b': return { B | BLACK };
	case 'R': return { R | WHITE };
	case 'r': return { R | BLACK };
	case 'Q': return { Q | WHITE };
	case 'q': return { Q | BLACK };
	case 'K': return { K | WHITE };
	case 'k': return { K | BLACK };
	default: throw;
	}
}

void MovePool::add(Move move, const State& state, bool)
{
	assert(count < MAX_MOVES);
	assert((move.to | move.from) < 64);

	State::Color color = state.board[move.from].color();

	if (state.board[move.to].color() != color)
	{
		king_captures |= state.board[move.to].type() == State::K;
		moves[count++] = move;
	}
}

void PositionAnalysis::add(Move move, const State& state, bool attack)
{
	assert((move.to | move.from) < 64);

	State::Color color = state.board[move.from].color();
	i16 value = 0;

	if (attack)
	{
		// Control: +5 points for each empty sqaure attacked
		value += 5;

		// Protection: +10 points for protecting another piece
		value += 5 * (state.board[move.to].color() == color);

		// Attack: +(piece / 32) points for each peice attacked
		value += (state.get_piece_value(state.board[move.to]) * ((state.board[move.to].color() == (color ^ State::COLOR_MASK))) / 32);
	}
	else
	{
		// Mobility: +1 points for each sqaure accessable
		value += 1;
	}

	score += value * (3 - ((i16)color << 1));
}
