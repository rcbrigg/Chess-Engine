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
	MovePool() : count(0), king_captures(false) {}

	static const u32 MAX_MOVES = 255;

	Move moves[MAX_MOVES];
	u8 count;
	bool king_captures;

	void add(Move move, const struct State& state, bool attack);
};

struct PositionAnalysis
{
	PositionAnalysis() : score(0) {}

	i16 score;

	void add(Move move, const struct State& state, bool attack);
};

struct State
{
	enum Color : u8
	{
		NONE = 0x0,
		WHITE = 0x1,
		BLACK = 0x2
	};

	enum Type : u8
	{
		P = 0x1 << 2,
		N = 0x2 << 2,
		B = 0x3 << 2,
		R = 0x4 << 2,
		Q = 0x5 << 2,
		K = 0x6 << 2
	};

	enum Castle : u8
	{
		WK = 0x0,
		WQ = 0x1,
		BK = 0x2,
		BQ = 0x3,
	};

	static const u32 COLOR_MASK = 0x3;
	static const u32 TYPE_MASK = 0x7 << 2;
	const char* piece_to_char = "..PpNnBbRrQqKk";
	const char* start_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq - 0 1";
	//const char* start_FEN = "k7/ppp5/ppp5/8/8/8/5R2/6Nr/ b KQkq - 0 1";
	//const char* start_FEN = "k7/ppp5/ppp5/8/5Q2/8/6r1/5K2/ b KQkq - 0 1";

	//const char* start_FEN = "1k6/ppp3r1/ppp5/8/8/8/3Q4/5K2/ w KQkq - 0 3";
	//const char* start_FEN = "1k3Q2/ppp3r1/ppp5/8/8/8/8/5K2/ b KQkq - 0 2";
	//const char* start_FEN = "k7/ppp5/ppp5/8/4Q3/8/6r1/5K2/ w KQkq - 0 3";
	//const char* start_FEN = "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R/ b KQkq - 0 1";
	//const i16 PAWN_VALUE = 100;
	//const i16 KNIGHT_VALUE = 300;
	//const i16 BISHOP_VALUE = 320;
	//const i16 ROOK_VALUE = 500;
	//const i16 QUEEN_VALUE = 900;
	//const i16 KING_VALUE = 20000;

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
		u8 fifty_move_clock;
		Piece captured_piece;
		u64 hash;
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

	i16 value;

	u32 half_moves;

	u64 hash;

	void init();

	const char* print() const;

	bool move(const char* str);

	void get_moves(MovePool& moves) const;

	void move(Move move);

	void move(Move move, u64 new_hash);

	void unmove(Undo& undo);

	u64 hash_move(Move move) const;

	bool from_FEN(string FEN);

	string to_FEN() const;

	i16 evaluate_position() const;

	i16 get_piece_value(Piece piece) const;

private:
	Piece piece_from_char(char piece);

	u8 square_from_notation(const char* str);

	const char* square_to_notation(u8 square) const;

	void set_en_passant(u8 square);

	void set_castle_rights(u8 rights);
};