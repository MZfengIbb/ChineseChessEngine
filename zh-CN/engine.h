#ifndef ENGINE_H_
#define ENGINE_H_

#include <fstream>
#include <vector>
#include <memory.h>

// 棋子编号
#define KING 	0
#define GUARD 	1
#define BISHOP	2
#define KNIGHT	3
#define ROOK	4
#define CANNON	5
#define PAWN	6


// 最大的历史走法数
#define MAX_MOVES 		512
// 和棋时返回的分数(取负值)
#define DRAW_VALUE 		20
// 空步裁剪的子力边界
#define NULL_MARGIN 	400
// 最高分值，即将死的分值
#define MATE_VALUE 		10000
// 长将判负的分值，低于该值将不写入置换表
#define BAN_VALUE 		(MATE_VALUE - 100)
// 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
#define WIN_VALUE		(MATE_VALUE - 200)
// 最大的搜索深度
#define LIMIT_DEPTH 	64
// 空步裁剪的裁剪深度
#define NULL_DEPTH		2
// 置换表大小
#define HASH_SIZE 		(1 << 20)
// 开局库大小
#define BOOK_SIZE 		20000
// 先行权分值
#define ADVANCED_VALUE 	3
// 最大的生成走法数
#define MAX_GEN_MOVES 	128
// ALPHA节点的置换表项
#define HASH_ALPHA		1
// BETA节点的置换表项
#define HASH_BETA		2
// PV节点的置换表项
#define HASH_PV			3
// 随机性分值
#define RANDOM_MASK		7
// 类型定义

typedef unsigned char uint8;
typedef char int8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;


// RC4密码流生成器
struct RC4
{
	uint8 s[256];
	int x, y;
	// 用空密钥初始化密码流生成器
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
{ // 生成密码流的下一个字节
	uint8 uc;
	x = (x + 1) & 255;
	y = (y + s[x]) & 255;
	uc = s[x];
	s[x] = s[y];
	s[y] = uc;
	return s[(s[x] + s[y]) & 255];
}
inline uint32 RC4::next32(void)
{ // 生成密码流的下四个字节
	uint8 uc0, uc1, uc2, uc3;
	uc0 = next8();
	uc1 = next8();
	uc2 = next8();
	uc3 = next8();
	return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}




// Zobrist结构
struct Zobrist
{
	uint32 key, lock0, lock1;
	// 用零填充Zobrist
	inline void init_zero(void);
	// 用密码流填充Zobrist
	inline void init_rc4(RC4 & rc4);
	// 执行XOR操作
	inline void Xor(const Zobrist & zobr);
	inline void Xor(const Zobrist & zobr1, const Zobrist & zobr2);
};
// 用零填充Zobrist
inline void Zobrist::init_zero(void)
{
	key = lock0 = lock1 = 0;
}
// 用密码流填充Zobrist
inline void Zobrist::init_rc4(RC4 & rc4)
{
	key = rc4.next32();
	lock0 = rc4.next32();
	lock1 = rc4.next32();
}
// 执行XOR操作
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

// Zobrist表
struct ZobristStruct
{
	// 初始化Zobrist表
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



// 是否未过河
inline bool home_side(int idx, int side)
{
	return (idx & 0x80) != (side << 7);
}
// 是否已过河
inline bool away_side(int idx, int side)
{
	return (idx & 0x80) == (side << 7);
}
// 是否在河的同一边
inline bool same_side(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0x80) == 0;
}
// 是否在同一行
inline bool same_rank(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0xf0) == 0;
}
// 是否在同一列
inline bool same_file(uint32 from_idx, uint32 to_idx)
{
	return ((from_idx ^ to_idx) & 0x0f) == 0;
}
// 获得红黑基值(红子是8，黑子是16)
inline uint8 base(uint8 side)
{
	return 8 + (side << 3);
}
// 获得纵坐标
inline uint32 row(uint32 idx)
{
	return idx >> 4;
}
// 获得横坐标
inline uint32 column(uint32 idx)
{
	return idx & 15;
}
// 根据棋盘纵坐标和棋盘横坐标获得索引
inline uint32 coord_idx(uint32 x, uint32 y)
{
	return x + (y << 4);
}
// 旋转棋盘索引
inline uint32 rotate(uint32 idx)
{
	return 254 - idx;
}
// 棋盘横坐标水平镜像
inline uint32 flip_column(uint32 x)
{
	return 14 - x;
}
// 棋盘纵坐标垂直镜像
inline uint32 flip_row(uint32 y)
{
	return 15 - y;
}
// 棋盘索引的水平镜像
inline uint32 mirror_idx(uint32 idx)
{
	return coord_idx(flip_column(column(idx)), row(idx));
}
// 得到走法的起点
inline uint32 from(uint32 mv)
{
	return mv & 0xFF;
}
// 得到走法的终点
inline uint32 to(uint32 mv)
{
	return mv >> 8;
}
// 根据起点和终点生成走法
inline uint32 move(uint32 from_idx, uint32 to_idx)
{
	return from_idx | (to_idx << 8);
}
// 走法水平镜像
inline uint32 mirror_move(uint32 mv)
{
	return move(mirror_idx(from(mv)), mirror_idx(to(mv)));
}
// 得到棋盘上某一方索引的前一格索引
inline uint32 forward(uint32 idx, uint8 side)
{
	return idx - 16 + (side << 5);
}



// 象棋棋盘结构
class Board
{
	friend class Engine;
	friend class CompareMvvLva;
public:
	// 历史走法信息(占4字节)
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
	// 初始化棋盘
	void init(void);
	// 清空棋盘
	void clear_board(void);
	// 清空(初始化)历史走法信息
	void clear_moves(void);
	// 产生走法, 返回生成的走法数量, capture 为 true 则只生成吃子的招法
	uint32 gen_moves(uint32 * moves, bool capture = false);
	// 判断走法是否合理
	bool legal_move(uint32 mv) const;
	// 扫描棋盘判断是否被将军
	bool checked(void) const;
	// 判断是否被将死
	bool is_mate(void);
	// 交换走子方
	void change_side(void);
	// 在棋盘上放一枚棋子
	void add_piece(uint32 idx, uint8 piece);
	// 从棋盘上拿走一枚棋子
	void del_piece(uint32 idx, uint8 piece);
	// 局面评价函数
	int32 evaluate(void) const;
	// 是否被将军
	bool in_check(void) const;
	// 上一步是否吃子
	bool captured(void) const;
	// 走棋
	bool do_move(uint32 mv);
	// 悔棋
	void undo_move();
	// 和棋分值
	inline int draw_value(void) const;
	// 检测重复局面
	int rep_status(int recur = 1) const;
	// 重复局面分值
	inline int rep_value(int32 rep_status) const;
private:
	// 移动一个棋子
	uint8 move_piece(uint32 mv);
	// 撤消移动棋子
	void undo_move_piece(uint32 mv, uint8 captured);
	// 走一步棋
	bool make_move(uint32 mv);
	// 撤消走一步棋
	void undo_make_move(void);
	// 走一步空步
	void make_null_move(void);
	// 撤消走一步空步
	void undo_make_null_move(void);
	// 判断是否允许空步裁剪
	inline bool null_okay(void) const;
	// 对局面镜像
	inline void mirror(Board & board_mirror) const;
	// 求MVV/LVA值
	inline int32 mvv_lva(uint32 mv);
public:
	// 16 * 16 的棋盘按行顺序排开，
	// 其中只有从第4行第4列开始到第13行12列范围内的格子是有效的
	uint8 _board[256];
	// 轮到谁走
	uint8 _player;
	// Zobrist
	Zobrist _zobr;
	// 红、黑双方的子力价值
	int32 _red_val, _black_val;
	// 走棋的历史列表
	MoveInfo _move_list[MAX_MOVES];
	// 距离根节点的步数
	uint32 _distance;
	// 历史表中的步数
	uint32 _move_count;
	// 走棋的历史列表
	std::vector<MoveInfo> _records;
};

inline Board::Board()
{
	init();
}

// 清空棋盘
inline void Board::clear_board(void)
{
	_player = _red_val = _black_val = _distance = 0;
	memset(_board, 0, 256);
	_zobr.init_zero();
}

// 清空(初始化)历史走法信息
inline void Board::clear_moves(void)
{
	_move_list[0].set(0, 0, checked(), _zobr.key);
	_move_count = 1;
}

// 是否被将军
inline bool Board::in_check(void) const
{
	return _move_list[_move_count - 1].check;
}

// 上一步是否吃子
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
// 和棋分值
inline int Board::draw_value(void) const
{
	return (_distance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
}
// 重复局面分值
inline int Board::rep_value(int32 rep_status) const
{
	int32 ret;
	ret = ((rep_status & 2) == 0 ? 0 : _distance - BAN_VALUE)
		+ ((rep_status & 4) == 0 ? 0 : BAN_VALUE - _distance);
	return ret == 0 ? draw_value() : ret;
}
// 判断是否允许空步裁剪
inline bool Board::null_okay(void) const
{
	return (_player == 0 ? _red_val : _black_val) > NULL_MARGIN;
}
// 对局面镜像
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
// 求MVV/LVA值
inline int32 Board::mvv_lva(uint32 mv)
{
	return (__mvv_lva[_board[to(mv)]] << 3) - __mvv_lva[_board[from(mv)]];
}


// 置换表项结构
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


// 开局库项结构
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
	// 装入开局库
	void load_book(void);
	// 搜索开局库
	int32 search_book(void);
	// 提取置换表项
	int32 probe_hash(int32 alpha, int32 beta, int32 depth, uint32 &mv);
	// 保存置换表项
	void record_hash(int32 flag, int32 vl, int32 depth, uint32 mv);
	// 对最佳走法的处理
	inline void set_best_move(uint32 mv, int32 depth);
	// 静态(Quiescence)搜索过程
	int32 search_quiesc(int alpha, int beta);
	// 超出边界(Fail-Soft)的Alpha-Beta搜索过程
	int32 search_full(int32 alpha, int32 beta, int32 depth, bool no_null = false);
	// 根节点的Alpha-Beta搜索过程
	int32 search_root(int depth);
	// 迭代加深搜索过程, sec 思考的时间(秒)
	void search_main(float sec = 1);
	// 电脑走的棋
	uint32 _mv_result;
private:
	// 置换表
	HashItem _hash_table[HASH_SIZE];
	// 开局库
	BookItem _book_table[BOOK_SIZE];
	// 开局库大小
	uint32 _book_size;
public:
	// 杀手走法表
	uint32 _killers[LIMIT_DEPTH][2];
	// 历史表
	int32 _history_table[65536];
	// 局面实例
	Board _board;
};

inline void Engine::init()
{
	_board.init();
}

// 对最佳走法的处理
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
