//
// Created by Ibb on 2017/9/19.
//

#ifndef _CHINESECHESS_ENGINE_H_
#define _CHINESECHESS_ENGINE_H_

#include  <fstream>
#include  <vector>
#include  <memory.h>

// Chess ID
#define KING 	0
#define GUARD 	1
#define BISHOP	2
#define KNIGHT	3
#define ROOK	4
#define CANNON	5
#define PAWN	6

// Max move counts
#define MAX_MOVES		512
// The return value when game draw
#define DRAW_VALUE 		20
// The value of NULL moves
#define NULL_MARGIN 	400
// The MAX value of the game
#define MATE_VALUE 		10000
// The value when a player fouls
#define BAN_VALUE 		(MATE_VALUE - 100)
// The value when the winner is determined
#define WIN_VALUE		(MATE_VALUE - 200)
// MAX search depth
#define LIMIT_DEPTH 	64
// The cut depth of NULL moves
#define NULL_DEPTH		2
// Max size of hash tables
#define HASH_SIZE 		(1 << 20)
// The book size of chess manual
#define BOOK_SIZE 		20000
// The advanced value
#define ADVANCED_VALUE 	3
// Max counts of generated moves
#define MAX_GEN_MOVES 	128
// The Hash table of AlPHA nodes
#define HASH_ALPHA		1
// The Hash table of BETA nodes
#define HASH_BETA		2
// The Hash table of PV nodes
#define HASH_PV			3
// Random value
#define RANDOM_MASK		7

// type define
typedef unsigned char uint8;
typedef char int8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;


// RC4 password stream generator
struct RC4
{
    uint8 s[256];
    int x, y;
    // Initialize the generator by using a NULL private key
    inline RC4(void);
    inline uint8 next8(void);
    inline uint32 next32(void);
};

inline RC4::RC4(void)
{
    int i, j;
    uint8 uc;
    x = y = j = 0;
    for (i = 0; i < 256; i++)
    {
        s[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + s[i]) & 255;
        uc = s[i];
        s[i] = s[j];
        s[j] = uc;
    }
}
inline uint8 RC4::next8(void)
{ // generate the next byte of the password stream
	uint8 uc;
	x = (x + 1) & 255;
	y = (y + s[x]) & 255;
	uc = s[x];
	s[x] = s[y];
	s[y] = uc;
	return s[(s[x] + s[y]) & 255];
}
inline uint32 RC4::next32(void)
{ // generate the next 4 bytes of the password stream
    uint8 uc0, uc1, uc2, uc3;
    uc0 = next8();
    uc1 = next8();
    uc2 = next8();
    uc3 = next8();
    return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}

// Zobrist struct
struct Zobrist{
    uint32 key, lock0, lock1;
    // Fill the Zobrist with 0
    inline void init_zero(void);
    // Fill the Zobrist with password stream
    inline void init_rc4(RC4 & rc4);
    // Do XOR operation
    inline void Xor(const Zobrist & zobr);
    inline void Xor(const Zobrist & zobr1, const Zobrist & zobr2);
};
//'Implements
inline void Zobrist::init_zero(void)
{
    key = lock0 = lock1 = 0;
}
inline void Zobrist::init_rc4(RC4 & rc4)
{
    key = rc4.next32();
    lock0 = rc4.next32();
    lock1 = rc4.next32();
}
inline void Zobrist::Xor(const Zobrist & zobr)
{
    key ^= zobr.key;
    lock0 ^= zobr.lock0;
    lock1 ^= zobr.lock1;
}
inline void Zobrist::Xor(const Zobrist & zobr1, const Zobrist & zobr2)
{
    key ^= zobr1.key ^ zobr2.key;
    lock0 ^= zobr1.lock0 ^ zobr2.lock0;
    lock1 ^= zobr1.lock1 ^ zobr2.lock1;
}

// Zobrist table
struct ZobristStruct{
    // Initialize the Zobrist table
    inline ZobristStruct();
    Zobrist player;
    Zobrist table[14][256];
};
inline ZobristStruct::ZobristStruct()
{
	int i, j;
	RC4 rc4;
	player.init_rc4(rc4);
	for (i = 0; i < 14; i++)
	{
		for (j = 0; j < 256; j++)
		{
			table[i][j].init_rc4(rc4);
		}
	}
}

/// Implements of Situation judgement
// Haven't across the river ?
inline bool home_side(int idx, int side){
    return (idx & 0x80) != (side << 7);
}
// Already across the river ?
inline bool away_side(int idx, int side)
{
    return (idx & 0x80) == (side << 7);
}
// Whether stand on one side of the river
inline bool same_side(uint32 from_idx, uint32 to_idx)
{
    return ((from_idx ^ to_idx) & 0x80) == 0;
}
// Whether stand on the same row
inline bool same_rank(uint32 from_idx, uint32 to_idx)
{
    return ((from_idx ^ to_idx) & 0xf0) == 0;
}
// Whether stand on the same column
inline bool same_file(uint32 from_idx, uint32 to_idx)
{
    return ((from_idx ^ to_idx) & 0x0f) == 0;
}
// Get the base value of chess (Red:8, Black:16)
inline uint8 base(uint8 side)
{
    return (uint8) (8 + (side << 3));
}
// Get the Y-axis
inline uint32 row(uint32 idx)
{
    return idx >> 4;
}
// Get the X-axis
inline uint32 column(uint32 idx)
{
    return idx & 15;
}
// Get coordinate value
inline uint32 coord_idx(uint32 x, uint32 y)
{
    return x + (y << 4);
}
// Get index after rotating the board
inline uint32 rotate(uint32 idx)
{
    return 254 - idx;
}
// Get column value after flipping the board
inline uint32 flip_column(uint32 x)
{
    return 14 - x;
}
// Get row value after flipping the board
inline uint32 flip_row(uint32 y)
{
    return 15 - y;
}
// The index of board's horizontal mirror
inline uint32 mirror_idx(uint32 idx)
{
    return coord_idx(flip_column(column(idx)), row(idx));
}
// Get the starting point of the move
inline uint32 from(uint32 mv)
{
    return mv & 0xFF;
}
// Get the terminal point of the move
inline uint32 to(uint32 mv)
{
    return mv >> 8;
}
// Get the move using the starting point and the terminal point
inline uint32 move(uint32 from_idx, uint32 to_idx)
{
    return from_idx | (to_idx << 8);
}
// Get the horizontal mirror of the move
inline uint32 mirror_move(uint32 mv)
{
    return move(mirror_idx(from(mv)), mirror_idx(to(mv)));
}
// Get the forward index of current index
inline uint32 forward(uint32 idx, uint8 side)
{
    return idx - 16 + (side << 5);
}

// The chessboard class
class Board{
    friend class Engine;
    friend class CompareMvvLva;

public:
    // History move info
    struct MoveInfo{
        uint16 mv;
        uint8 captured;
        uint8 check;
        uint32 key;
        inline void set(uint32 mv_, uint8 captured_, bool check_, uint32 key_);
    };
public:
    inline Board();
    // Initialize the board
    void init(void);
    // Clear the board
    void clear_board(void);
    // Clear the moves
    void clear_moves(void);
    // Generate moves,return the count , only generate moves that can capture a chess when capture is true
    uint32 gen_moves(uint32 * moves, bool capture = false);
    // Return true if the move is legal
    bool legal_move(uint32 mv) const;
    // Return true if the KING is checked
    bool checked(void) const;
    // Return true if checkmate
    bool is_mate(void);
    // Change the turn
    void change_side(void);
    // Put a chess on the board
    void add_piece(uint32 idx, uint8 piece);
    // Take away a chess from the board
    void del_piece(uint32 idx, uint8 piece);
    // Evaluate current status
    int32 evaluate(void) const;
    // Return true if the KING is checked
    bool in_check(void) const;
    // Return true when last move captured a chess
    bool captured(void) const;
    // Do a move
    bool do_move(uint32 mv);
    // Undo a move
    void undo_move();
    // Return the value when draw
    inline int draw_value(void) const;
    // Judge whether the status recurs
	int rep_status(int32 recur = 1) const;
    // Return the value of recured status
    inline int rep_value(int32 rep_status) const;
private:
    // Move a chess
    uint8 move_piece(uint32 mv);
    // Undo it
    void undo_move_piece(uint32 mv, uint8 captured);
    // Make a move
    bool make_move(uint32 mv);
    // Undo it
    void undo_make_move(void);
    // Make a NULL move
    void make_null_move(void);
    // Undo it
    void undo_make_null_move(void);
    // Return true if the NULL move cut is permitted
    inline bool null_okay(void) const;
    // Mirror the board
    inline void mirror(Board & board_mirror) const;
    // Get MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
    inline int32 mvv_lva(uint32 mv);
public:
    // Board of 16 * 16
    uint8 _board[256];
    // Who's turn
    uint8 _player;
    // Zobrist
    Zobrist _zobr;
    // Total value of Red and Black
    int32 _red_val, _black_val;
    // History move list
    MoveInfo _move_list[MAX_MOVES];
    // Distance to ROOT node
    uint32 _distance;
    // Move counts of history move list
    uint32 _move_count;
    // Records for history move lists
    std::vector<MoveInfo> _records;
};

// Implements
inline Board::Board() {

}

inline void Board::clear_board(void)
{
    _player = _red_val = _black_val = _distance = 0;
    memset(_board, 0, 256);
    _zobr.init_zero();
}
inline void Board::clear_moves(void)
{
    _move_list[0].set(0, 0, checked(), _zobr.key);
    _move_count = 1;
}
inline bool Board::in_check(void) const
{
    return _move_list[_move_count - 1].check;
}
inline bool Board::captured(void) const
{
    return _move_list[_move_count - 1].captured != 0;
}
inline void Board::MoveInfo::set(uint32 mv_, uint8 captured_, bool check_, uint32 key_)
{
    mv = mv_;
    captured = captured_;
    check = check_;
    key = key_;
}
inline int Board::draw_value(void) const
{
    return (_distance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
}
inline int Board::rep_value(int32 rep_status) const
{
    int32 ret;
    ret = ((rep_status & 2) == 0 ? 0 : _distance - BAN_VALUE)
          + ((rep_status & 4) == 0 ? 0 : BAN_VALUE - _distance);
    return ret == 0 ? draw_value() : ret;
}
inline bool Board::null_okay(void) const
{
    return (_player == 0 ? _red_val : _black_val) > NULL_MARGIN;
}
inline void Board::mirror(Board & board_mirror) const
{
    uint32 idx;
    uint8 val;
    board_mirror.clear_board();
    for (idx = 0; idx < 256; idx++)
    {
        val = _board[idx];
        if (val != 0)
            board_mirror.add_piece(mirror_idx(idx), val);
    }
    if (_player == 1)
        board_mirror.change_side();
    board_mirror.clear_moves();
}
extern const uint8 __mvv_lva[];
inline int32 Board::mvv_lva(uint32 mv)
{
    return (__mvv_lva[_board[to(mv)]] << 3) - __mvv_lva[_board[from(mv)]];
}

// HashTableItem
struct HashItem
{
    uint8 depth;
    uint8 flag;
    int16 vl;
    uint16 mv;
    uint16 reserved;
    uint32 lock0;
    uint32 lock1;
};

// The book item of chess manual
struct BookItem
{
    uint32 lock;
    uint16 mv;
    uint16 vl;
};


class Engine
{
public:
    Engine();
    Engine(const Engine&);

    inline void init();
    // Load book of chess manual
    void load_book(void);
    // Search the book
    int32 search_book(void);
    // Get HashItem
    int32 probe_hash(int32 alpha, int32 beta, int32 depth, uint32 &mv);
    // Save HashItem
    void record_hash(int32 flag, int32 vl, int32 depth, uint32 mv);
    // Process the best move
    inline void set_best_move(uint32 mv, int32 depth);
    // Quiescence search process
    int32 search_quiesc(int alpha, int beta);
    // The Alpha-Beta search process beyond the Fail-Soft
    int32 search_full(int32 alpha, int32 beta, int32 depth, bool no_null = false);
    // The root node of the Alpha-Beta search process
    int32 search_root(int depth);
    // Iterations deepen the search process, seconds think of the time
    void search_main(float sec = 1);
    // NGet next move result after computer search.
    uint32 _mv_result;
private:
    // HashTable
    HashItem _hash_table[HASH_SIZE];
    // BookTable
    BookItem _book_table[BOOK_SIZE];
    // BookSize
    uint32 _book_size;
public:
    // The killers' move lists
    uint32 _killers[LIMIT_DEPTH][2];
    // History table
    int32 _history_table[65536];
    // Board instance
    Board _board;
};

inline void Engine::init()
{
    _board.init();
}

// Process the best move
inline void Engine::set_best_move(uint32 mv, int32 depth)
{
    uint32* mv_killers;
    _history_table[mv] += depth * depth;
    mv_killers = _killers[_board._distance];
    if (mv_killers[0] != mv)
    {
        mv_killers[1] = mv_killers[0];
        mv_killers[0] = mv;
    }
}

#endif //_CHINESECHESS_ENGINE_H_
