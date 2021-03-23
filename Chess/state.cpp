#include "state.h"

static const i16 piece_scores[] = { 0, 0, 1, -1, 3, -3, 3, -3, 5, -5, 9, -9, 1000, -1000 };

extern u32 rng[896];

inline void hash_piece(State::Piece piece, u8 square, u32& hash)
{
	assert(square + u16(piece.bits << 6) < 896);
	//hash ^= rng[square + u16(piece.bits << 6)];
	//hash ^= hash_u16(square + u16(piece.bits << 6)) * piece.bits;
	hash ^= 2654435761 * (square + u32(piece.bits << 6));
	//hash ^= piece.bits * (square + 1) * 2654435761;
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
	u32 hash = hash_move(move);
	State::move(move, hash);
}

void State::move(Move move, u32 new_hash)
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
		value -= piece_scores[board[move.to].bits]; // capture
		board[move.to] = board[move.from];
		board[move.from].bits = 0;

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

	//if (capture)
	//{
	//	fifty_move_clock = 0;
	//}

	half_moves += 1;
	hash = new_hash;
	//fifty_move_clock += 1;
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
		board[undo.move.from] = board[undo.move.to];
		board[undo.move.to] = undo.captured_piece;
		value += piece_scores[undo.captured_piece.bits];
	}

	//en_passant = old_state.en_passant;
	//castle_rights = old_castle_rights;
	hash = undo.hash;
	half_moves -= 1;
	forward *= -1;
}

u32 State::hash_move(Move move) const
{
	assert((move.from | move.to) < 64);
	u32 new_hash = hash;

	if (move.bits & Move::SPECIAL_MASK)
	{
		assert(0); // TODO
	}
	else
	{
		hash_piece(board[move.to], move.to, new_hash);
		hash_piece(board[move.from], move.to, new_hash);
		hash_piece(board[move.from], move.from, new_hash);
	}
	return new_hash;
}

bool State::from_FEN(string FEN)
{
	try
	{
		value = 0;
		hash = 0;

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
					Piece piece = piece_from_char(FEN[i]);
					value += piece_scores[piece.bits];
					hash_piece(piece, square, hash);
					board[square++] = piece;
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
						hash_piece({ 0 }, square, hash);
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
			moves.add({ current, advance });
			u8 advance_2 = advance + forward;

			// If pawn is on starting rank it can move two squares
			if (u16(y - (forward / 4)) >= 8 && board[advance_2].empty())
			{
				moves.add({ current, advance_2 });
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
				moves.add({ current, target });
			}
			break;
		}
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

u32 rng[896] = {
0x45f3c788,
0xbdccf12d,
0x13b4e0d7,
0xec95b8fe,
0x7cd4f75f,
0x75be237d,
0xd9cd8c4b,
0xc7842db1,
0x4ff9b518,
0xb14a714c,
0x83122308,
0xcf10e219,
0x5e8f322e,
0x008b1219,
0xd7fdb114,
0x564220f5,
0xeb09e538,
0xf8930170,
0x6b9a4469,
0x4bc8fdb9,
0xa7a2703a,
0x98252223,
0xdebfea48,
0x3869d62e,
0x92f73293,
0xed52f769,
0x5b95715b,
0x16a8957e,
0xb7ed1828,
0xae5113a4,
0x19c5c9a7,
0x30d264c5,
0x7a50018c,
0x9898a150,
0xc347710e,
0x72f79780,
0xb65544e7,
0x8d505829,
0xea2ccd6f,
0xa35d01fa,
0x78e0fd82,
0xd529dabc,
0xfd1eda81,
0x0709390b,
0x937b5ebe,
0xa6cea77a,
0xf4caa8f0,
0x991b4902,
0xe3c0dc48,
0x53031215,
0xff7ad252,
0xb204ac84,
0xddaa8c6f,
0xcce38edb,
0x646d3832,
0x4972fd39,
0xd3ffb56f,
0xe0419305,
0x14e657a6,
0x793eabf2,
0x36da096e,
0xbb8df5a4,
0x436c6cd6,
0xb7b36298,
0xf6f97a3c,
0x3b0787fc,
0xb1237dae,
0xc20eb498,
0x8beab916,
0x205081a3,
0x9d96dffd,
0xc4666e75,
0xde8b37cc,
0xab660bdb,
0xdc92622c,
0xb79fb3ff,
0x4885af7f,
0xe03d234e,
0xf86d41b1,
0x6d386cc9,
0x05cbc56d,
0x4a303779,
0x357cbd18,
0x0af62980,
0x8dead37b,
0xe50b9f9c,
0x195ecb73,
0xecaccdbc,
0x7dccdc05,
0xe5f2e225,
0x1fc44039,
0x29cc19bb,
0x67919a34,
0x1ca87069,
0x34143550,
0xf0111a32,
0xda2829e7,
0xb22cdf1b,
0x3865772d,
0xbbd11f19,
0x1ea6604b,
0x5217bd53,
0x25b7ce67,
0xd02a1646,
0x52cad296,
0xd2c35961,
0x87ad1016,
0x8ba9c757,
0xa3b40ca9,
0x49f1f78c,
0x4f055aa5,
0xe5bf1320,
0x3e45a0cb,
0x99ab65f9,
0xb82e28a1,
0xe2bccf84,
0xbfaa8d95,
0x9798e0a5,
0x9bbd319d,
0x71befcee,
0x33002ad1,
0xd7697f2d,
0x69211ad1,
0xc2f7cdfe,
0x0be78b5b,
0x1d43bf11,
0x2f2369a2,
0x192cbd5c,
0xd8310402,
0x6e88bbe8,
0xc2efd826,
0xd73ddf3d,
0x9181e9d5,
0x9ee4fb11,
0xea65a628,
0x80dda6c1,
0x5a6c4325,
0x3eaabed1,
0xa87a00ff,
0x670b0747,
0x911ae2a6,
0x0d6d9838,
0xbfd890cf,
0x8c074a95,
0x17c20bcf,
0xe2f255a2,
0x19a316cc,
0x287e3ecb,
0xcd0325e5,
0x82efcc23,
0x52570ac5,
0xe10c403d,
0xed5013f9,
0x288e1e48,
0x93b8f7b0,
0x54b26922,
0xf4eb8f9c,
0x890664eb,
0x1e8e7a01,
0x8ab137ba,
0xdb58496a,
0x4020d753,
0x0fa359e5,
0xa8061d4a,
0x990703c3,
0xbd4a72f6,
0x7afc87f2,
0x72d8f306,
0xe8cb850b,
0x6cf837c5,
0xc8f5fe9c,
0xdef76198,
0x5996cdfd,
0x23ecb21a,
0xd5c686eb,
0x6db67698,
0x5df8fa89,
0x21e7d148,
0x938f461c,
0xe15d0c29,
0xceb8a6cc,
0x8caf8244,
0x5301df14,
0x26109a8f,
0xe368e3ec,
0xed3acf5e,
0xfdb30905,
0x9050c7d6,
0xbf3967c4,
0xc241da0c,
0x55735d22,
0x67ff1474,
0x34fd9fc7,
0x64468aa7,
0x7d254e97,
0x017ead51,
0xd698e1c5,
0x44db9a58,
0xbda360c3,
0xd3171f2e,
0x65767824,
0xab68f81c,
0x24619780,
0x1faf08f0,
0x5f7ae337,
0x6afda8a7,
0x4c9eb74a,
0xf6ef8b93,
0xf703ecd9,
0xaf0020b0,
0xa5ad129e,
0x65fc2075,
0x0b491520,
0xbe9d6fe0,
0xc938961c,
0xe4080303,
0xb15e5e84,
0xf0518a28,
0x28ba118e,
0xc13eadbf,
0xfd2ee8bf,
0xef0514df,
0x4c603e85,
0xce693c48,
0x7ff9742e,
0xf9985a72,
0x9fe4f0c7,
0x96eb5230,
0x7c3e80cc,
0x5b2e8ba2,
0x1e8a3356,
0xbffe5635,
0x95e81d8a,
0xbdec0360,
0xbb3a9621,
0x6a169f67,
0x8f11a003,
0x5fca5b7e,
0x34e8b071,
0xcea935f5,
0x8f8266ce,
0xe06e5e00,
0x6ee07f07,
0x8a8eb5f2,
0x2dd2ba72,
0x00bfb2a3,
0xc296158e,
0xd25d57a5,
0x2606c4e1,
0xfa500de3,
0x2d1017d3,
0xc3208edb,
0x10769a2a,
0x8a70fe44,
0x649a95c0,
0xd20469fb,
0x158e6314,
0x34b3bc99,
0xa3c9afcb,
0xa6bfc2e9,
0xc1257fe1,
0x21b01d88,
0x1e878782,
0xdad74d43,
0xb2332ad6,
0x41017c1a,
0x3a29aa02,
0xb7ee0e9b,
0x79c726ba,
0xd1366fb6,
0x63004ee3,
0x94715a9b,
0x2586d24b,
0x8fc2e438,
0x375f371a,
0xc83b84ee,
0x20e6b61e,
0xf5181541,
0x0d56f359,
0x1f7f1b76,
0xa83f1120,
0x31453faf,
0x0482d09e,
0x40e92f14,
0x7c4a3af7,
0x9d5a6e61,
0x0073f2bf,
0xb53065ba,
0xb75acb82,
0xb3537d62,
0x3a0ee41f,
0xa1c3a489,
0x8d691d2f,
0x1836bc8f,
0xb8668acd,
0x3c1791e4,
0x45d4be43,
0x7dc59861,
0xa7082412,
0x4bffe7af,
0x24d51538,
0x0fb4de97,
0x7f71ffac,
0x5be26e2d,
0x482be342,
0xba56a95b,
0x98366110,
0x467cf76f,
0xec3508ec,
0x2b841734,
0x57919ed4,
0x6bb8a668,
0xf8b697d7,
0x03204836,
0x05b56cb5,
0x97df9187,
0xa5f04b65,
0xa6dcd6fb,
0xd584db97,
0xc34297c0,
0xbefd0216,
0xa4b817dc,
0x897ccfd7,
0x6b6767e9,
0x237da27c,
0x0f5368c8,
0x1486e4dd,
0x4ea17124,
0x243fea94,
0x39d0ecf1,
0x7459ade0,
0x6a2047e1,
0xb9e14064,
0x9546a127,
0x4b0b6ce1,
0x258654f7,
0xb7b26d33,
0x67f6a8af,
0xb3f5bf3b,
0x41fe97bf,
0xeb83baa6,
0x7a3fee8b,
0xbc48707b,
0x01e43898,
0xb77b7e0d,
0x378440b9,
0x49c33ba7,
0x16ae2c7d,
0x027f197e,
0x7e9dfdb0,
0xa9407d9f,
0x530a843f,
0x8088961e,
0xd210d3bd,
0xa2b84a3d,
0xae4185b4,
0x4d53fbca,
0x4448b438,
0xeb92e762,
0xcfe3f794,
0x57b21804,
0x9e80ffd4,
0xebaa5c03,
0x3582a882,
0x8ba1e8fb,
0x7a0d6ba7,
0x0264d274,
0xf4a935e8,
0x9ca58b43,
0x27478691,
0x0363f5f6,
0xb2016724,
0x57ca1563,
0x19304267,
0x000b4db4,
0xff2a0b3b,
0xa6aa3d7f,
0xc83b699d,
0xa846cfa0,
0x8757259e,
0xa0a6132e,
0x97f29231,
0x93f29f32,
0x3bf81940,
0x93b1b49c,
0xf70ac824,
0x102617be,
0x12f81c7f,
0x46dfa703,
0x4a0f5a79,
0xb9f4ee43,
0x2dbfdb18,
0x3bba5fa1,
0x0bbd70ef,
0xe3944444,
0x9249b633,
0xd7c35f1b,
0xf565c9fa,
0x91b0c177,
0x582a55b8,
0xc11a6aec,
0xb4d3b08d,
0xa494d5a1,
0x21cbd73a,
0x5779a25c,
0xf316bdaa,
0x9572ca63,
0x2b5e7dc0,
0x38b9b272,
0x012f7d4a,
0x38a00e9c,
0x8305b49a,
0x173a7292,
0x7a4d1e24,
0x6d88bc7b,
0x9c7b5cb7,
0x41c40aa2,
0xf34e907a,
0x4982d29d,
0x6b379702,
0x9a91a4c7,
0xf082be46,
0x94dcc89d,
0x67a99b18,
0xf2ab8ac7,
0xfb89a0b7,
0x457c486c,
0x51c7e23c,
0x8271ef66,
0x5b4121c9,
0xc39f58b1,
0x99c72d4d,
0x83affd55,
0xeff54a16,
0x5c49b638,
0x9d81ea65,
0xcc1e4366,
0xf642f637,
0x0f47060c,
0xa0af1eda,
0x6b2289e4,
0x0088b9ac,
0x0f6974d7,
0x438f7d8e,
0x5fb3dd72,
0xa06620f4,
0xaae8f316,
0x2e138756,
0xb3fa913e,
0x0141d3ee,
0xa3905414,
0x6573b3f5,
0xb6b5aa52,
0x6d1c648c,
0x5e28ae05,
0xd609337c,
0xd282b04a,
0x60afe78c,
0x2191e4fc,
0x1f1470ef,
0x2e50889a,
0xf639fbb3,
0x5ac53a8a,
0xfb3f9d9b,
0x4cad5308,
0x68da2850,
0xd315192f,
0x451dad29,
0x9720a605,
0xfe2d6693,
0x05958702,
0xeac6202c,
0x35eebff2,
0xd572679f,
0x64577554,
0x1637bebf,
0x3916ff0c,
0xb7ec8bca,
0xb1dd56ee,
0xd2f77919,
0x62830234,
0xd746c048,
0x0cd4462a,
0x33e434ae,
0x4a5c7274,
0xd2842d2d,
0x7dd70024,
0x35c16534,
0xeca6f3ee,
0x34af1ce9,
0xac58a11f,
0xbce1eae2,
0x89808630,
0xa5615543,
0xa8b38ec7,
0xdbe0a895,
0x99284809,
0x07c4cae1,
0x06539d16,
0xb091237a,
0xbb88022c,
0x9ae4c4ad,
0x831c739f,
0xd0371cc4,
0x59ca10ab,
0xf5f6cf67,
0xdf1a4e85,
0xba81783e,
0xb2dd8e07,
0x862cdaa0,
0xbb87db57,
0x26cff7d3,
0x5af7db89,
0x83b81ee6,
0x8f50b5c0,
0xa8b927a0,
0x33c60ac1,
0x63df758f,
0xb4637b20,
0x9f0dcb59,
0x494d6587,
0x88404a22,
0x0ec00e20,
0xbff31b73,
0x4cc7196f,
0xec16fda0,
0x4db214d3,
0x4622d43c,
0x2f20ca88,
0xcdaabb77,
0x93fd8c1c,
0xf644f90a,
0x3aee4846,
0x294ab82f,
0x43a050d6,
0xdadfac21,
0x0a58e65f,
0x511ffc3f,
0x7985d4e9,
0xb8dc24bb,
0x5c339fcc,
0x9dfeb847,
0x432ddabb,
0x67451bcf,
0xe351c9e7,
0x31b5755a,
0xf46fa90b,
0x9af44823,
0x65df7fcf,
0x5bc85a47,
0x181ae64e,
0x6208c3c9,
0x007bc78e,
0x4be72323,
0x7911999c,
0xcf1d1b65,
0xd1303985,
0x20ef0554,
0xafc0d453,
0xfd090190,
0x6082eb57,
0xcf0ae997,
0x1d1d595e,
0xd47a81e7,
0x0d66fff8,
0x931141ef,
0xc1a943c6,
0xbff5620a,
0x2fcb6365,
0x98fa5ab8,
0x07cdd250,
0x7e00f809,
0xb18b74fa,
0xf3ba4493,
0xe6b8d169,
0x956ec51e,
0xd62af879,
0xf35744b8,
0x157eda75,
0x35c3d130,
0x4c0ba054,
0x5bfb3d67,
0xf5cce945,
0x26edcb15,
0x08b8b2ad,
0xeae57e69,
0x83170b37,
0xe8c5ac06,
0x55603ab2,
0x652bc322,
0xeb4b75bb,
0x818361a7,
0x29483f24,
0xd671de66,
0x6f3299c7,
0x343aefaa,
0x45e260fc,
0x2884d617,
0x0af5a911,
0xb86dbc01,
0xc2394f0e,
0x0e95c863,
0x87581c15,
0x74fa1840,
0xcc78b6f3,
0x4bad3299,
0x020b8808,
0xdff37e3f,
0x8e370481,
0xfe6f22e9,
0x097ab46d,
0x5a92ecb4,
0x77c9703a,
0xb9a786fc,
0x312f115c,
0x3c000cb3,
0x6b2a9276,
0x16b9f966,
0x24352900,
0x91a284d0,
0x5b365501,
0xe8050514,
0x2153df70,
0x37653dc7,
0x9a1e8bda,
0xdfc5d2f6,
0x2a7ac009,
0x1e61f40b,
0x09722d24,
0x0756f075,
0x54399341,
0x36e7f897,
0x6c30d080,
0xd2248c3b,
0xb2812b04,
0x32c80e66,
0x45d6f8af,
0x97372b11,
0xff9edc95,
0xb9b73d7a,
0x9c3b7ee8,
0x86092cd9,
0x75c75198,
0xe5162dda,
0x725ea9ce,
0x680ba3f3,
0x04f4f6f6,
0xf90afe74,
0x00f602bf,
0x66f1e22f,
0x9ddb02e3,
0x829d678b,
0x68988b60,
0xd28d0b45,
0x452e4f58,
0x72151814,
0x829d4e91,
0xfa47c697,
0x574a5a39,
0xa741d177,
0x0c8523c6,
0xbf95d24e,
0x4d3ac36d,
0x4c86c500,
0xa217dbc8,
0xc3a86a26,
0x098012cb,
0x0acca438,
0x2aee4742,
0xd9baa4e7,
0x3fa5477d,
0x6dd8e67e,
0x7b28aa2d,
0x9cdb2d15,
0xab13344c,
0x3944771e,
0xff2c318f,
0x92f4b469,
0xdcef2a3d,
0xbd3bbf2e,
0xd9ca36b0,
0x30d41b77,
0x784815d3,
0x3f9d53a9,
0x92bf1ea5,
0xb3617d98,
0x82853f4d,
0x24a1eeb1,
0xf91a6019,
0x218d13fa,
0xa5e9c9a5,
0x084e620d,
0xad05a321,
0x051a9c28,
0xf5911da4,
0xe762b580,
0xc6f8dc4b,
0xd9a0732a,
0xd5bc39ea,
0x9644fabf,
0x13ec6ad0,
0xac87177a,
0xc0054b00,
0x9c1b88c8,
0x960eee9c,
0x2adfdab6,
0x6bff613b,
0xe84d8eb1,
0xd041eb10,
0x7eef30a6,
0x31e1515a,
0xd51b4b88,
0xb1cbd4c5,
0x320d81ac,
0x32ef23d9,
0x2480e690,
0x26e8ca16,
0x82bc6f6f,
0xce69c233,
0xa48dbd65,
0x15fea5b1,
0x44fb0211,
0x14f3829a,
0x421186c7,
0x82fa475c,
0x1edc56c3,
0x58343a95,
0x14c9fd12,
0xee1715d6,
0x6efebaf9,
0xe8debe9b,
0x870ad274,
0x503eab8a,
0x3829a585,
0x6c23023d,
0x3e1a0791,
0x4ed1d522,
0x97cc2572,
0xa6052da8,
0xbbfc03d3,
0x1bf64006,
0x8566bd0c,
0x2399e65a,
0x4f2ddee5,
0x6afe51db,
0xa7ca5a58,
0x4e2ac32d,
0xf4d20af4,
0x3515158b,
0xcd53eeb6,
0xebaa22fd,
0x5b2f4362,
0x6dff1eff,
0x434175d5,
0x68db9674,
0x08fa2d4d,
0x5db91949,
0xac9ef0ff,
0xff9d232f,
0xb1b17106,
0x7a87dbd2,
0xe3776ddd,
0xa9d5f2d1,
0xcd7a6d4f,
0x5e788828,
0x27c2a609,
0x1df49c64,
0x55321573,
0x12d409fa,
0xcbbccc0c,
0xabbd3915,
0x3550f6ba,
0x1b857706,
0xaf57c768,
0x2d92c71d,
0x46469045,
0x7d2d8b84,
0x8229c5b8,
0x6cd50c86,
0x0eef1324,
0x30d16e39,
0xff3a214f,
0x304a4e10,
0x2fdf897f,
0x0a1c9fba,
0xd3dac1e8,
0x51ff90d1,
0x9959fb70,
0xe5c2af3c,
0x1369f4cb,
0x8292bb0b,
0x67dc5cc9,
0xa1b49e52,
0x6d659c78,
0xcf2dbf59,
0xa26bf0f1,
0x0e334bc6,
0xd6087269,
0xd21a933e,
0x17912542,
0x34f9ba5f,
0xb2046320,
0xbb772720,
0xd3d7c212,
0x76fa6e37,
0xd2c80da5,
0xb237f4e0,
0x70cfb339,
0xf49a5054,
0xac0ccff2,
0xeac50e97,
0x0a16ce48,
0xcd2c76e9,
0x49d3ca1a,
0x1b446331,
0xa677f042,
0x69016528,
0x88d6fcdf,
0xf1485211,
0xb1e6610d,
0x6200ba16,
0x75a946f7,
0x455d0c1a,
0x4785757a,
0x51e28769,
0x692b1f4d,
0xf76d7cbf,
0xc55be2fd,
0x87b102c2,
0x7928c21e,
0x9e6eb064,
0x8f7c6f3b,
0x4167bac7,
0x4b2c92a9,
0x25ac51ec,
0xf35dd02d,
0xbb1ed25a,
0xd3fd2206,
0xc8c3db7e,
0x10c3da7e,
0xe21e2993,
0xe697bd2c,
0xbb528d7b,
0xf7657198,
0x951dc1c2,
0x663976b1,
0x5818aa9b,
0x5b5ab95a,
0xb54b6a0d,
0x687ad89d,
0x070a153d,
0x93370a76,
0x18a08ba8,
0xa6752ff4,
0x19aac1d6,
0xe65ee259,
0x27b07097,
0x40522499,
0x12970da2,
0xbece94c9,
0x855b41c3,
0xf183f650,
0xa208cc5e,
0x1f789f1a,
0x8f81c79b,
0x3588d603,
0xbe1cf632,
0xa096531b,
0x403ff81c,
0x67fb3daa,
0xc375637d,
0x71d0ca2d,
0xc882b937,
0xfb003797,
0x0fb95d7d,
0x0e6819cc,
0xc1c43ca5,
0x4d448886,
0x7cb65300,
0x5ddd4923,
0x28c47663,
0xee855c55,
0x77b593a6,
0x506e3dab,
0xad938484,
0xb856b8f1,
0xb70d554c,
0xed638882,
0x578dc3d9,
0xd3d1cced,
0xc64a23bb,
0x7ef0c712,
0xe6fb99a5,
0xb3583f06,
0x97ff1bb7,
0x791d5aca,

};
