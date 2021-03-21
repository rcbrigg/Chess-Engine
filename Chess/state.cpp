#include "state.h"

void State::init()
{
	if (from_FEN(start_FEN) == false)
	{
		printf("Bad FEN!");
	}
}

const char* State::print() const
{
	static u8 str[8][17];
	for (u8 i = 0; i < 8; i++)
	{
		for (u8 j = 0; j < 8; j++)
		{
			str[i][2 * j] = piece_to_char[board[8 * i + j].bits];
			str[i][2 * j + 1] = ' ';
		}
		str[i][16] = '\n';
	}
	return (char*)str;
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

void State::get_valid_moves(MovePool& moves) const
{
	// TODO: check for check, check mate, stalemate, 50 move rule
	u8 color = half_moves & 0x1;
	for (u8 i = 0; i < 64; ++i)
	{
		if (!board[i].empty() && board[i].color() == color)
		{
			switch (board[i].type())
			{
			case P: get_pawn_moves(i, moves); break;
			case N: get_knight_moves(i, moves); break;
			case B: get_bishop_moves(i, moves); break;
			case R: get_rook_moves(i, moves); break;
			case Q: get_queen_moves(i, moves); break;
			case K: get_king_moves(i, moves); break;
			default: break;
			}
		}
	}
}

void State::move(Move move)
{
	assert((move.from | move.to) < 64);
	//assert(forward * forward == 64);
	Piece piece = board[move.from];
	u8 special = move.bits & Move::SPECIAL_MASK;
	Color color = piece.color();
	bool capture = false;

	if (special)
	{
		if (special == Move::CASTLE)
		{
			auto castle = move.bits >> 14;
			if (castle == WK)
			{
				add_piece({ WHITE | K }, 62);
				add_piece({ WHITE | R }, 61);
				remove_piece(60);
				remove_piece(63);
			}
			else if (castle == WQ)
			{
				add_piece({ WHITE | K }, 58);
				add_piece({ WHITE | R }, 59);
				remove_piece(60);
				remove_piece(56);
			}
			else if (castle == BK)
			{
				add_piece({ BLACK | K }, 6);
				add_piece({ BLACK | R }, 5);
				remove_piece(4);
				remove_piece(7);
			}
			else // (castle == BQ)
			{
				add_piece({ BLACK | K }, 2);
				add_piece({ BLACK | R }, 3);
				remove_piece(4);
				remove_piece(0);
			}
		}
		else if (special == Move::ENPASSANT)
		{
			add_piece(piece, move.to);
			remove_piece(move.from);
			u8 x = move.to % 8;
			u8 y = (move.to / 8) - forward;
			remove_piece(x + 8 * y);
			capture = true;
			fifty_move_clock = 0;
		}
		else // (special == Move::PROMOTION)
		{
			remove_piece(move.from);
			remove_piece(move.to);
			add_piece({ ((move.bits >> 13) + 1u) | color }, move.to);
		}

		set_en_passant(0xff);
	}
	else
	{
		Piece target = board[move.to];
		capture = !target.empty();
		if (capture)
		{
			remove_piece(move.to);
		}

		remove_piece(move.from);
		add_piece(piece, move.to);

		

		set_en_passant(0xff);
		
		if (piece.type() == P)
		{
			fifty_move_clock = 0;

			u8 y_current = move.from / 8;
			u8 y_to = move.to / 8;

			if (color == WHITE)
			{
				if (y_current == 6 && y_to == 4)
				{
					// TODO HASH
					set_en_passant(40 + (move.to / 8));
				}
			}
			else
			{
				if (y_current == 1 && y_to == 3)
				{
					// TODO HASH
					set_en_passant(en_passant = 16 + (move.to / 8));
				}
			}
		}
	}

	if (capture)
	{
		fifty_move_clock = 0;
	}

	half_moves += 1;
	fifty_move_clock += 1;
	forward *= -1;
}

bool State::from_FEN(string FEN)
{
	try
	{
		value = 0;

		u32 i = 0;
		u8 square = 0;
		bool new_row = true;
		while (true)
		{
			if (square % 8 == 0 && !new_row)
			{
				if (FEN[i] != '/')
				{
					throw std::exception();
				}
				if (square == 64)
				{
					++i;
					break;
				}
				new_row = true;
			}
			else
			{
				new_row = false;
				if (FEN[i] >= 'A')
				{
					add_piece(piece_from_char(FEN[i]), square++);
				}
				else
				{
					u8 skip = FEN[i] - '0';
					if (skip == 0 || skip > (8 - (square % 8)))
					{
						throw std::exception();
					}
					for (u8 j = 0; j < skip; ++j)
					{
						board[square++].bits = 0;
					}
				}
			}
			++i;
		}

		if (FEN[i++] != ' ')
		{
			throw std::exception();
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

		if (FEN[i++] != ' ')
		{
			throw std::exception();
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

		if (FEN[i++] != ' ')
		{
			throw std::exception();
		}

		if (FEN[i] == '-')
		{
			en_passant = 0xff;
			++i;
		}
		else
		{
			en_passant = square_from_notation(&FEN.data()[i]);
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

		u32 moves = stoi(&FEN.data()[i]) - 1;
		i += 1 + (fifty_move_clock / 10);
		half_moves = 2 * moves + turn;

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
				FEN.push_back(piece_to_char[piece.bits]);
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

i16 State::evaluate_position() const
{
	return value;
}

void State::get_pawn_attack(u8 current, u8 target, MovePool& moves) const
{
	if (!board[target].empty() && board[target].color() != board[current].color())
	{
		assert(target < 64);
		moves.add({ current, target });
	}
	else if (target == en_passant)
	{
		moves.add({ Move::ENPASSANT });
	}
}

void State::get_pawn_moves(u8 current, MovePool& moves) const
{
	u8 advance = current + forward;
	u8 x = current % 8;
	u8 y = current / 8;
	if (u8(current + forward) < 64)
	{
		if (board[current + forward].empty())
		{
			assert(advance < 64);
			moves.add({ current, advance });

			// If pawn is on starting rank it can move two squares
			if (u16(y - (forward / 4)) >= 8 && board[current + 2 * forward].empty())
			{
				assert(advance < 64);
				moves.add({ current, advance });
			}
		}

		if (x - 1 > 0)
		{
			get_pawn_attack(current, advance - 1, moves);
		}
		if (x + 1 < 8)
		{
			get_pawn_attack(current, advance + 1, moves);
		}
	}
}

void State::get_range_moves(u8 current, u8 x, u8 y, u8 dx, u8 dy, MovePool& moves) const
{
	i8 ds = dx + 8 * dy;
	u8 target = current;
	Color color = board[current].color();

	while (true)
	{
		x += dx;
		y += dy;
		target += ds;

		if ((x | y) >= 8)
		{
			break;
		}

		if (!board[target].empty())
		{
			if (board[target].color() != color)
			{
				assert(target < 64);
				moves.add({ current, target });
			}
			break;
		}
		assert(target < 64);
		moves.add({ current, target });
	}
}

void State::add_move_if_valid(u8 current, u8 x, u8 y, MovePool& moves) const
{
	if ((x | y) < 8)
	{
		u8 target = x + 8 * y;
		if (board[target].empty() || board[current].color() != board[target].color())
		{
			assert(x + 8u * y < 64);
			moves.add({ current, x + 8u * y });
		}
	}
}

void State::get_knight_moves(u8 current, MovePool& moves) const
{
	u8 x = current % 8;
	u8 y = current / 8;
	add_move_if_valid(current, x - 1, y - 2, moves);
	add_move_if_valid(current, x + 1, y - 2, moves);
	add_move_if_valid(current, x - 1, y + 2, moves);
	add_move_if_valid(current, x + 1, y + 2, moves);
	add_move_if_valid(current, x - 2, y - 1, moves);
	add_move_if_valid(current, x + 2, y - 1, moves);
	add_move_if_valid(current, x - 2, y + 1, moves);
	add_move_if_valid(current, x + 2, y + 1, moves);
}

void State::get_bishop_moves(u8 current, MovePool& moves) const
{
	u8 x = current % 8;
	u8 y = current / 8;
	get_range_moves(current, x, y, -1, -1, moves);
	get_range_moves(current, x, y, 1, -1, moves);
	get_range_moves(current, x, y, -1, 1, moves);
	get_range_moves(current, x, y, 1, 1, moves);
}

void State::get_rook_moves(u8 current, MovePool& moves) const
{
	u8 x = current % 8;
	u8 y = current / 8;
	get_range_moves(current, x, y, -1, 0, moves);
	get_range_moves(current, x, y, 1, 0, moves);
	get_range_moves(current, x, y, 0, -1, moves);
	get_range_moves(current, x, y, 0, 1, moves);
}

void State::get_queen_moves(u8 current, MovePool& moves) const
{
	get_rook_moves(current, moves);
	get_bishop_moves(current, moves);
}

void State::get_king_moves(u8 current, MovePool& moves) const
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
					if (board[target].empty() || board[current].color() != board[target].color())
					{
						assert(target < 64);
						moves.add({ current, target });
					}
				}
			}
		}
	}
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

u32 State::hash_piece(Piece piece, u32 square)
{
	return piece.bits * (square + 1) * 0xe123d4f9;
}

void State::set_en_passant(u8 square)
{
	en_passant = square;
	//hash ^= (en_passant + 1) * 0x520381ba;
	//hash ^= (square + 1) * 0x520381ba;
}

void State::set_castle_rights(u8 rights)
{
	castle_rights = rights;
	//hash ^= (castle_rights + 1) * 0xc5f537f6;
	//hash ^= (rights + 1) * 0xc5f537f6;
}

static const i16 piece_scores[] = { 0, 0, 1, -1, 3, -3, 3, -3, 5, -5, 9, -9, 1000, -1000 };

void State::remove_piece(u32 square)
{
	//hash ^= hash_piece(board[square], square);
	value -= piece_scores[board[square].bits];
	board[square].bits = 0;
}

void State::add_piece(Piece piece, u32 square)
{
	//hash ^= hash_piece(piece, square);
	board[square] = piece;
	value += piece_scores[piece.bits];
}

State::Piece State::piece_from_char(char piece)
{
	switch (piece)
	{
	case 'P': return { 0x2 };
	case 'p': return { 0x3 };
	case 'N': return { 0x4 };
	case 'n': return { 0x5 };
	case 'B': return { 0x6 };
	case 'b': return { 0x7 };
	case 'R': return { 0x8 };
	case 'r': return { 0x9 };
	case 'Q': return { 0xa };
	case 'q': return { 0xb };
	case 'K': return { 0xc };
	case 'k': return { 0xd };
	default: throw;
	}
}
