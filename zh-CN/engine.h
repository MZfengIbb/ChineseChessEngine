#ifndef ENGINE_H_
#define ENGINE_H_

#include <fstream>
#include <vector>
#include <memory.h>

// ���ӱ��
#define KING 	0
#define GUARD 	1
#define BISHOP	2
#define KNIGHT	3
#define ROOK	4
#define CANNON	5
#define PAWN	6


// ������ʷ�߷���
#define MAX_MOVES 		512
// ����ʱ���صķ���(ȡ��ֵ)
#define DRAW_VALUE 		20
// �ղ��ü��������߽�
#define NULL_MARGIN 	400
// ��߷�ֵ���������ķ�ֵ
#define MATE_VALUE 		10000
// �����и��ķ�ֵ�����ڸ�ֵ����д���û���
#define BAN_VALUE 		(MATE_VALUE - 100)
// ������ʤ���ķ�ֵ���ޣ�������ֵ��˵���Ѿ�������ɱ����
#define WIN_VALUE		(MATE_VALUE - 200)
// �����������
#define LIMIT_DEPTH 	64
// �ղ��ü��Ĳü����
#define NULL_DEPTH		2
// �û����С
#define HASH_SIZE 		(1 << 20)
// ���ֿ��С
#define BOOK_SIZE 		20000
// ����Ȩ��ֵ
#define ADVANCED_VALUE 	3
// ���������߷���
#define MAX_GEN_MOVES 	128
// ALPHA�ڵ���û�����
#define HASH_ALPHA		1
// BETA�ڵ���û�����
#define HASH_BETA		2
// PV�ڵ���û�����
#define HASH_PV			3
// ����Է�ֵ
#define RANDOM_MASK		7
// ���Ͷ���

typedef unsigned char uint8;
typedef char int8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;


// RC4������������
struct RC4
{
	uint8 s[256];
	int x, y;
	// �ÿ���Կ��ʼ��������������
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
{ // ��������������һ���ֽ�
	uint8 uc;
	x = (x + 1) & 255;
	y = (y + s[x]) & 255;
	uc = s[x];
	s[x] = s[y];
	s[y] = uc;
	return s[(s[x] + s[y]) & 255];
}
inline uint32 RC4::next32(void)
{ // ���������������ĸ��ֽ�
	uint8 uc0, uc1, uc2, uc3;
	uc0 = next8();
	uc1 = next8();
	uc2 = next8();
	uc3 = next8();
	return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}




// Zobrist�ṹ
struct Zobrist
{
	uint32 key, lock0, lock1;
	// �������Zobrist
	inline void init_zero(void);
	// �����������Zobrist
	inline void init_rc4(RC4 & rc4);
	// ִ��XOR����
	inline void Xor(const Zobrist & zobr);
	inline void Xor(const Zobrist & zobr1, const Zobrist & zobr2);
};
// �������Zobrist
inline void Zobrist::init_zero(void)
{
	key = lock0 = lock1 = 0;
}
// �����������Zobrist
inline void Zobrist::init_rc4(RC4 & rc4)
{
	key = rc4.next32();
	lock0 = rc4.next32();
	lock1 = rc4.next32();
}
// ִ��XOR����
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

// Zobrist��
struct ZobristStruct
{
	// ��ʼ��Zobrist��
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



// �Ƿ�δ����
inline bool home_side(int idx, int side)
{
	return (idx & 0x80) != (side << 7);
}
// �Ƿ��ѹ���
inline bool away_side(int idx, int side)
{
	return (idx & 0x80) == (side << 7);
}
// �Ƿ��ںӵ�ͬһ��
inline bool same_side(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0x80) == 0;
}
// �Ƿ���ͬһ��
inline bool same_rank(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0xf0) == 0;
}
// �Ƿ���ͬһ��
inline bool same_file(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0x0f) == 0;
}
// ��ú�ڻ�ֵ(������8��������16)
inline uint8 base(uint8 side)
{
	return 8 + (side << 3);
}
// ���������
inline uint32 row(uint32 idx)
{
	return idx >> 4;
}
// ��ú�����
inline uint32 column(uint32 idx)
{
	return idx & 15;
}
// ������������������̺�����������
inline uint32 coord_idx(uint32 x, uint32 y)
{
	return x + (y << 4);
}
// ��ת��������
inline uint32 rotate(uint32 idx)
{
	return 254 - idx;
}
// ���̺�����ˮƽ����
inline uint32 flip_column(uint32 x)
{
	return 14 - x;
}
// ���������괹ֱ����
inline uint32 flip_row(uint32 y)
{
	return 15 - y;
}
// ����������ˮƽ����
inline uint32 mirror_idx(uint32 idx)
{
	return coord_idx(flip_column(column(idx)), row(idx));
}
// �õ��߷������
inline uint32 from(uint32 mv)
{
	return mv & 0xFF;
}
// �õ��߷����յ�
inline uint32 to(uint32 mv)
{
	return mv >> 8;
}
// ���������յ������߷�
inline uint32 move(uint32 from_idx, uint32 to_idx)
{
	return from_idx | (to_idx << 8);
}
// �߷�ˮƽ����
inline uint32 mirror_move(uint32 mv)
{
	return move(mirror_idx(from(mv)), mirror_idx(to(mv)));
}
// �õ�������ĳһ��������ǰһ������
inline uint32 forward(uint32 idx, uint8 side)
{
	return idx - 16 + (side << 5);
}



// �������̽ṹ
class Board
{
	friend class Engine;
	friend class CompareMvvLva;
public:
	// ��ʷ�߷���Ϣ(ռ4�ֽ�)
	struct MoveInfo
	{
		uint16 mv;
		uint8 captured;
		uint8 check;
		uint32 key;
		inline void set(uint32 mv_, uint8 captured_, bool check_, uint32 key_);
	};
public:
	inline Board();
	// ��ʼ������
	void init(void);
	// �������
	void clear_board(void);
	// ���(��ʼ��)��ʷ�߷���Ϣ
	void clear_moves(void);
	// �����߷�, �������ɵ��߷�����, capture Ϊ true ��ֻ���ɳ��ӵ��з�
	uint32 gen_moves(uint32 * moves, bool capture = false);
	// �ж��߷��Ƿ����
	bool legal_move(uint32 mv) const;
	// ɨ�������ж��Ƿ񱻽���
	bool checked(void) const;
	// �ж��Ƿ񱻽���
	bool is_mate(void);
	// �������ӷ�
	void change_side(void);
	// �������Ϸ�һö����
	void add_piece(uint32 idx, uint8 piece);
	// ������������һö����
	void del_piece(uint32 idx, uint8 piece);
	// �������ۺ���
	int32 evaluate(void) const;
	// �Ƿ񱻽���
	bool in_check(void) const;
	// ��һ���Ƿ����
	bool captured(void) const;
	// ����
	bool do_move(uint32 mv);
	// ����
	void undo_move();
	// �����ֵ
	inline int draw_value(void) const;
	// ����ظ�����
	int rep_status(int recur = 1) const;
	// �ظ������ֵ
	inline int rep_value(int32 rep_status) const;
private:
	// �ƶ�һ������
	uint8 move_piece(uint32 mv);
	// �����ƶ�����
	void undo_move_piece(uint32 mv, uint8 captured);
	// ��һ����
	bool make_move(uint32 mv);
	// ������һ����
	void undo_make_move(void);
	// ��һ���ղ�
	void make_null_move(void);
	// ������һ���ղ�
	void undo_make_null_move(void);
	// �ж��Ƿ�����ղ��ü�
	inline bool null_okay(void) const;
	// �Ծ��澵��
	inline void mirror(Board & board_mirror) const;
	// ��MVV/LVAֵ
	inline int32 mvv_lva(uint32 mv);
public:
	// 16 * 16 �����̰���˳���ſ���
	// ����ֻ�дӵ�4�е�4�п�ʼ����13��12�з�Χ�ڵĸ�������Ч��
	uint8 _board[256];
	// �ֵ�˭��
	uint8 _player;
	// Zobrist
	Zobrist _zobr;
	// �졢��˫����������ֵ
	int32 _red_val, _black_val;
	// �������ʷ�б�
	MoveInfo _move_list[MAX_MOVES];
	// ������ڵ�Ĳ���
	uint32 _distance;
	// ��ʷ���еĲ���
	uint32 _move_count;
	// �������ʷ�б�
	std::vector<MoveInfo> _records;
};

inline Board::Board()
{
	init();
}

// �������
inline void Board::clear_board(void)
{
	_player = _red_val = _black_val = _distance = 0;
	memset(_board, 0, 256);
	_zobr.init_zero();
}

// ���(��ʼ��)��ʷ�߷���Ϣ
inline void Board::clear_moves(void)
{
	_move_list[0].set(0, 0, checked(), _zobr.key);
	_move_count = 1;
}

// �Ƿ񱻽���
inline bool Board::in_check(void) const
{
	return _move_list[_move_count - 1].check;
}

// ��һ���Ƿ����
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
// �����ֵ
inline int Board::draw_value(void) const
{
	return (_distance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
}
// �ظ������ֵ
inline int Board::rep_value(int32 rep_status) const
{
	int32 ret;
	ret = ((rep_status & 2) == 0 ? 0 : _distance - BAN_VALUE)
		+ ((rep_status & 4) == 0 ? 0 : BAN_VALUE - _distance);
	return ret == 0 ? draw_value() : ret;
}
// �ж��Ƿ�����ղ��ü�
inline bool Board::null_okay(void) const
{
	return (_player == 0 ? _red_val : _black_val) > NULL_MARGIN;
}
// �Ծ��澵��
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
// ��MVV/LVAֵ
inline int32 Board::mvv_lva(uint32 mv)
{
	return (__mvv_lva[_board[to(mv)]] << 3) - __mvv_lva[_board[from(mv)]];
}


// �û�����ṹ
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


// ���ֿ���ṹ
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
	// static tlib::RefPtr<Engine> create();
	inline void init();
	// װ�뿪�ֿ�
	void load_book(void);
	// �������ֿ�
	int32 search_book(void);
	// ��ȡ�û�����
	int32 probe_hash(int32 alpha, int32 beta, int32 depth, uint32 &mv);
	// �����û�����
	void record_hash(int32 flag, int32 vl, int32 depth, uint32 mv);
	// ������߷��Ĵ���
	inline void set_best_move(uint32 mv, int32 depth);
	// ��̬(Quiescence)��������
	int32 search_quiesc(int alpha, int beta);
	// �����߽�(Fail-Soft)��Alpha-Beta��������
	int32 search_full(int32 alpha, int32 beta, int32 depth, bool no_null = false);
	// ���ڵ��Alpha-Beta��������
	int32 search_root(int depth);
	// ����������������, sec ˼����ʱ��(��)
	void search_main(float sec = 1);
	// �����ߵ���
	uint32 _mv_result;
private:
	// �û���
	HashItem _hash_table[HASH_SIZE];
	// ���ֿ�
	BookItem _book_table[BOOK_SIZE];
	// ���ֿ��С
	uint32 _book_size;
public:
	// ɱ���߷���
	uint32 _killers[LIMIT_DEPTH][2];
	// ��ʷ��
	int32 _history_table[65536];
	// ����ʵ��
	Board _board;
};

inline void Engine::init()
{
	_board.init();
}

// ������߷��Ĵ���
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


#endif /* ENGINE_H_ */
