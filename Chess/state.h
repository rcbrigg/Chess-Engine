#pragma once
#include "types.h"
#include <string>
#include <assert.h>

using namespace std;

struct Move
{
	union
	{
		struct
		{
			u8 from;
			u8 to;
		};
		u16 bits;
	};
	static const u16 PROMOTION = 0x0040;
	static const u16 CASTLE = 0x0080;
	static const u16 ENPASSANT = 0x00c0;
	static const u16 SPECIAL_MASK = 0x00c0;
};

struct MovePool
{
	MovePool() : count(0) {}

	static const u32 MAX_MOVES = 256;

	Move moves[MAX_MOVES];

	u32 count;

	void add(Move move)
	{
		assert(count < MAX_MOVES);
		assert((move.to | move.from) < 64);
		moves[count++] = move;
	}
};

struct State
{
	enum Color : u8
	{
		WHITE = 0x0,
		BLACK = 0x1
	};

	enum Type : u8
	{
		X = 0x0,
		P = 0x2,
		N = 0x4,
		B = 0x6,
		R = 0x8,
		Q = 0xa,
		K = 0xc
	};

	enum Castle : u8
	{
		WK = 0x0,
		WQ = 0x1,
		BK = 0x2,
		BQ = 0x3,
	};

	static const u32 COLOR_MASK = 0x1;
	static const u32 TYPE_MASK = 0xe;
	const char* piece_to_char = "..PpNnBbRrQqKk";
	const char* start_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq - 0 1";
	//const char* start_FEN = "k7/ppp5/ppp5/8/8/8/5R2/6Nr/ b KQkq - 0 1";
	struct Piece
	{
		u8 bits;

		inline Color color() const
		{
			return Color(bits & COLOR_MASK);
		}

		inline Type type() const
		{
			return Type(bits & TYPE_MASK);
		}

		inline bool empty() const
		{
			return bits == 0;
		}
	};

	struct Undo
	{
		Move move;
		Piece captured_piece;
		u32 hash;
	};

	// Each element represents a square, 0 if empty or (Color | Type)
	Piece board[64];

	// En passant target square, 0xff if none
	u8 en_passant;

	// Bit field for castle rights
	// 1 - W O-O 
	// 2 - W O-O-O
	// 3 - B O-O
	// 4 - B O-O-O
	u8 castle_rights;

	u8 fifty_move_clock;

	i8 forward;

	u32 half_moves;

	i16 value;

	u32 hash;

	void init();

	const char* print() const;

	bool move(const char* str);

	void get_valid_moves(MovePool& moves) const;

	void move(Move move);

	void move(Move move, u32 new_hash);

	void unmove(Undo& undo);

	u32 hash_move(Move move) const;

	bool from_FEN(string FEN);

	string to_FEN() const;

	i16 evaluate_position() const;

private:
	void get_pawn_attack(u8 current, u8 target, MovePool& moves) const;

	void get_pawn_moves(u8 current, MovePool& moves) const;

	void get_range_moves(u8 current, u8 x, u8 y, u8 dx, u8 dy, MovePool& moves) const;

	void add_move_if_valid(u8 current, u8 x, u8 y, MovePool& moves) const;

	void get_knight_moves(u8 current, MovePool& moves) const;

	void get_bishop_moves(u8 current, MovePool& moves) const;

	void get_rook_moves(u8 current, MovePool& moves) const;

	void get_queen_moves(u8 current, MovePool& moves) const;

	void get_king_moves(u8 current, MovePool& moves) const;

	Piece piece_from_char(char piece);

	u8 square_from_notation(const char* str);

	const char* square_to_notation(u8 square) const;

	void set_en_passant(u8 square);

	void set_castle_rights(u8 rights);
};