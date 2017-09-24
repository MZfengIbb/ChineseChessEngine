#include <memory.h>
#include <cstdlib>
#include <search.h>
#include <algorithm>
#include <time.h>
#include "engine.h"

// 帅(将)的步长,马腿等同于帅,车炮兵的步长都等同于帅的倍数
static const int8 __king_step[4] = { -16, -1, 1, 16 };
// 仕(士)的步长,象的步长两倍于仕,同时象眼等同于仕
static const int8 __guard_step[4] = { -17, -15, 15, 17 };
// 马的步长
static const int8 __knight_step[4][2] =
{
	{ -33, -31 },
	{ -18, 14 },
	{ -14, 18 },
	{ 31, 33 } };
// 马被将军的步长，以仕(士)的步长作为马腿
static const int8 __knight_check_step[4][2] =
{
	{ -33, -18 },
	{ -31, -14 },
	{ 14, 31 },
	{ 18, 33 } };

// 判断棋子是否在棋盘中的掩码
static const uint8 __inboard[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 判断棋子是否在九宫的数组
static const uint8 __infort[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 棋盘初始设置
static const uint8 __board_init[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0, 20, 19, 18, 17, 16, 17, 18, 19, 20,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0, 21,  0,  0,  0,  0,  0, 21,  0,  0,  0,  0,  0,
	0,  0,  0, 22,  0, 22,  0, 22,  0, 22,  0, 22,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0, 14,  0, 14,  0, 14,  0, 14,  0, 14,  0,  0,  0,  0,
	0,  0,  0,  0, 13,  0,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0, 12, 11, 10,  9,  8,  9, 10, 11, 12,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 判断步长是否符合特定走法的数组，1=帅(将)，2=仕(士)，3=相(象)
static const int8 __legal_span[512] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  3,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  2,  1,  2,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  2,  1,  2,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  3,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0
};

// 根据步长判断马是否蹩腿的数组
static const int8 __knight_pin[512] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0
};



// 子力位置价值表
static const uint8 __pos_value[7][256] = {
	{ // 帅(将)
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0, 11, 15, 11,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 仕(士)
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 相(象)
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0, 18,  0,  0,  0, 23,  0,  0,  0, 18,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 马
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0, 90, 90, 90, 96, 90, 96, 90, 90, 90,  0,  0,  0,  0,
		0,  0,  0, 90, 96,103, 97, 94, 97,103, 96, 90,  0,  0,  0,  0,
		0,  0,  0, 92, 98, 99,103, 99,103, 99, 98, 92,  0,  0,  0,  0,
		0,  0,  0, 93,108,100,107,100,107,100,108, 93,  0,  0,  0,  0,
		0,  0,  0, 90,100, 99,103,104,103, 99,100, 90,  0,  0,  0,  0,
		0,  0,  0, 90, 98,101,102,103,102,101, 98, 90,  0,  0,  0,  0,
		0,  0,  0, 92, 94, 98, 95, 98, 95, 98, 94, 92,  0,  0,  0,  0,
		0,  0,  0, 93, 92, 94, 95, 92, 95, 94, 92, 93,  0,  0,  0,  0,
		0,  0,  0, 85, 90, 92, 93, 78, 93, 92, 90, 85,  0,  0,  0,  0,
		0,  0,  0, 88, 85, 90, 88, 90, 88, 90, 85, 88,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 车
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,206,208,207,213,214,213,207,208,206,  0,  0,  0,  0,
		0,  0,  0,206,212,209,216,233,216,209,212,206,  0,  0,  0,  0,
		0,  0,  0,206,208,207,214,216,214,207,208,206,  0,  0,  0,  0,
		0,  0,  0,206,213,213,216,216,216,213,213,206,  0,  0,  0,  0,
		0,  0,  0,208,211,211,214,215,214,211,211,208,  0,  0,  0,  0,
		0,  0,  0,208,212,212,214,215,214,212,212,208,  0,  0,  0,  0,
		0,  0,  0,204,209,204,212,214,212,204,209,204,  0,  0,  0,  0,
		0,  0,  0,198,208,204,212,212,212,204,208,198,  0,  0,  0,  0,
		0,  0,  0,200,208,206,212,200,212,206,208,200,  0,  0,  0,  0,
		0,  0,  0,194,206,204,212,200,212,204,206,194,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 炮
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,100,100, 96, 91, 90, 91, 96,100,100,  0,  0,  0,  0,
		0,  0,  0, 98, 98, 96, 92, 89, 92, 96, 98, 98,  0,  0,  0,  0,
		0,  0,  0, 97, 97, 96, 91, 92, 91, 96, 97, 97,  0,  0,  0,  0,
		0,  0,  0, 96, 99, 99, 98,100, 98, 99, 99, 96,  0,  0,  0,  0,
		0,  0,  0, 96, 96, 96, 96,100, 96, 96, 96, 96,  0,  0,  0,  0,
		0,  0,  0, 95, 96, 99, 96,100, 96, 99, 96, 95,  0,  0,  0,  0,
		0,  0,  0, 96, 96, 96, 96, 96, 96, 96, 96, 96,  0,  0,  0,  0,
		0,  0,  0, 97, 96,100, 99,101, 99,100, 96, 97,  0,  0,  0,  0,
		0,  0,  0, 96, 97, 98, 98, 98, 98, 98, 97, 96,  0,  0,  0,  0,
		0,  0,  0, 96, 96, 97, 99, 99, 99, 97, 96, 96,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},{ // 兵(卒)
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  9,  9,  9, 11, 13, 11,  9,  9,  9,  0,  0,  0,  0,
		0,  0,  0, 19, 24, 34, 42, 44, 42, 34, 24, 19,  0,  0,  0,  0,
		0,  0,  0, 19, 24, 32, 37, 37, 37, 32, 24, 19,  0,  0,  0,  0,
		0,  0,  0, 19, 23, 27, 29, 30, 29, 27, 23, 19,  0,  0,  0,  0,
		0,  0,  0, 14, 18, 20, 27, 29, 27, 20, 18, 14,  0,  0,  0,  0,
		0,  0,  0,  7,  0, 13,  0, 16,  0, 13,  0,  7,  0,  0,  0,  0,
		0,  0,  0,  7,  0,  7,  0, 15,  0,  7,  0,  7,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	} };

// 索引是否在棋盘有效范围内
inline bool in_board(uint32 idx)
{
	return __inboard[idx];
}

// 索引是否在九宫格范围内
inline bool in_fort(uint32 idx)
{
	return __infort[idx];
}

// 走法是否符合帅(将)的步长
inline bool king_span(uint32 from_idx, uint32 to_idx)
{
	return __legal_span[to_idx - from_idx + 256] == 1;
}

// 走法是否符合仕(士)的步长
inline bool guard_span(uint32 from_idx, uint32 to_idx)
{
	return __legal_span[to_idx - from_idx + 256] == 2;
}

// 走法是否符合相(象)的步长
inline bool bishop_span(uint32 from_idx, uint32 to_idx)
{
	return __legal_span[to_idx - from_idx + 256] == 3;
}

// 相(象)眼的位置
inline uint32 bishop_pin(uint32 from_idx, uint32 to_idx)
{
	return (from_idx + to_idx) >> 1;
}

// 马腿的位置
inline uint32 knight_pin(uint32 from_idx, uint32 to_idx)
{
	return from_idx + __knight_pin[to_idx - from_idx + 256];
}


static ZobristStruct __zobrist;

////////////////////////////////////////////////////////////////////////
// 以下为类方法

// 初始化棋盘
void Board::init(void)
{
	int idx, val;
	clear_board();
	for (idx = 0; idx < 256; idx++)
	{
		val = __board_init[idx];
		if (val != 0)
		{
			add_piece(idx, val);
		}
	}
	clear_moves();
	_records.clear();
}

inline void Board::change_side(void)
{
	_player = 1 - _player;
	_zobr.Xor(__zobrist.player);
}

// 在棋盘上放一枚棋子
inline void Board::add_piece(uint32 idx, uint8 piece)
{
	_board[idx] = piece;
	// 红方加分，黑方(注意"__pos_value"取值要颠倒)减分
	if (piece < 16)
	{
		_red_val += __pos_value[piece - 8][idx];
		_zobr.Xor(__zobrist.table[piece - 8][idx]);
	}
	else
	{
		_black_val += __pos_value[piece - 16][rotate(idx)];
		_zobr.Xor(__zobrist.table[piece - 9][idx]);
	}
}

// 从棋盘上拿走一枚棋子
inline void Board::del_piece(uint32 idx, uint8 piece)
{
	_board[idx] = 0;
	// 红方减分，黑方(注意"__pos_value"取值要颠倒)加分
	if (piece < 16)
	{
		_red_val -= __pos_value[piece - 8][idx];
		_zobr.Xor(__zobrist.table[piece - 8][idx]);
	}
	else
	{
		_black_val -= __pos_value[piece - 16][rotate(idx)];
		_zobr.Xor(__zobrist.table[piece - 9][idx]);
	}
}

// 局面评价函数
inline int32 Board::evaluate(void) const
{
	return (_player == 0 ? _red_val - _black_val : _black_val - _red_val)
		+ ADVANCED_VALUE;
}

// 移动一个棋子
inline uint8 Board::move_piece(uint32 mv)
{
	uint32 from_idx, to_idx;
	uint8 from_val, captured;
	from_idx = from(mv);
	to_idx = to(mv);
	captured = _board[to_idx];
	if (captured != 0)
		del_piece(to_idx, captured);
	from_val = _board[from_idx];
	del_piece(from_idx, from_val);
	add_piece(to_idx, from_val);
	return captured;
}

// 撤消移动棋子
inline void Board::undo_move_piece(uint32 mv, uint8 captured)
{
	uint32 from_idx, to_idx;
	uint8 val;
	from_idx = from(mv);
	to_idx = to(mv);
	val = _board[to_idx];
	del_piece(to_idx, val);
	add_piece(from_idx, val);
	if (captured != 0)
		add_piece(to_idx, captured);
}

// 走一步棋
inline bool Board::make_move(uint32 mv)
{
	uint8 captured;
	uint32 key;
	key = _zobr.key;
	captured = move_piece(mv);
	if (checked())
	{
		undo_move_piece(mv, captured);
		return false;
	}
	change_side();
	_move_list[_move_count].set(mv, captured, checked(), key);
	_move_count++;
	_distance++;
	return true;
}

// 撤消走一步棋
inline void Board::undo_make_move(void)
{
	_distance--;
	_move_count--;
	change_side();
	undo_move_piece(_move_list[_move_count].mv, _move_list[_move_count].captured);
}

bool Board::do_move(uint32 mv)
{
	bool ret = make_move(mv);
	if (ret)
	{
		_records.push_back(_move_list[_move_count - 1]);
	}
	return ret;
}

// 悔棋
void Board::undo_move(void)
{
	if (_move_count > 1)
	{
		undo_make_move();
		_records.pop_back();
	}
	else if (_records.size() > 0)
	{
		MoveInfo& mvinfo = _records.back();
		change_side();
		undo_move_piece(mvinfo.mv, mvinfo.captured);
		_records.pop_back();
	}
}

// 走一步空步
inline void Board::make_null_move(void)
{
	uint32 key;
	key = _zobr.key;
	change_side();
	_move_list[_move_count].set(0, 0, false, key);
	_move_count++;
	_distance++;
}

// 撤消走一步空步
inline void Board::undo_make_null_move(void)
{
	_distance--;
	_move_count--;
	change_side();
}

// 扫描棋盘判断是否被将军
bool Board::checked(void) const
{
	uint32 from_idx, to_idx;
	uint8 to_val;
	int32 step;
	uint8 self_base = base(_player);
	uint8 mate_base = base(!_player);

	//for (from_idx = 0; from_idx < 256; from_idx++)
	for (from_idx = 0x33; from_idx <= 0xcb; from_idx++)
	{
		// 找到棋盘上的帅(将)
		if (_board[from_idx] != self_base + KING)
			continue;

		// 1. 判断是否被对方的兵(卒)将军
		if (_board[forward(from_idx, _player)] == mate_base + PAWN)
			return true;
		for (step = -1; step <= 1; step += 2)
		{
			if (_board[from_idx + step] == mate_base + PAWN)
				return true;
		}

		// 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
		for (uint32 i = 0; i < 4; i++)
		{
			if (_board[from_idx + __guard_step[i]] != 0)
				continue;
			for (uint32 j = 0; j < 2; j++)
			{
				to_val = _board[from_idx + __knight_check_step[i][j]];
				if (to_val == mate_base + KNIGHT)
					return true;
			}
		}

		// 3. 判断是否被对方的车或炮将军(包括将帅对脸)
		for (uint32 i = 0; i < 4; i++)
		{
			step = __king_step[i];
			to_idx = from_idx + step;
			while (in_board(to_idx))
			{
				to_val = _board[to_idx];
				if (to_val != 0)
				{
					if (to_val == mate_base + ROOK || to_val == mate_base
						+ KING)
						return true;
					break;
				}
				to_idx += step;
			}
			to_idx += step;
			while (in_board(to_idx))
			{
				to_val = _board[to_idx];
				if (to_val != 0)
				{
					if (to_val == mate_base + CANNON)
						return true;
					break;
				}
				to_idx += step;
			}
		}
		return false;
	}
	return false;
}

// 判断是否被杀
bool Board::is_mate(void)
{
	uint8 captured;
	uint32 mvs[MAX_GEN_MOVES];
	uint32 gen_count = gen_moves(mvs);
	for (uint32 i = 0; i < gen_count; i++)
	{
		captured = move_piece(mvs[i]);
		if (!checked())
		{
			undo_move_piece(mvs[i], captured);
			return false;
		}
		else
		{
			undo_move_piece(mvs[i], captured);
		}
	}
	return true;
}

// 判断走法是否合理
bool Board::legal_move(uint32 mv) const
{
	uint32 from_idx, to_idx, pin_idx, step;
	uint8 self_base, from_val, to_val;
	// 判断走法是否合法，需要经过以下的判断过程：

	// 1. 判断起始格是否有自己的棋子
	from_idx = from(mv);
	from_val = _board[from_idx];
	self_base = base(_player);
	if ((from_val & self_base) == 0)
		return false;

	// 2. 判断目标格是否有自己的棋子
	to_idx = to(mv);
	to_val = _board[to_idx];
	if ((to_val & self_base) != 0)
		return false;

	// 3. 根据棋子的类型检查走法是否合理
	switch (from_val - self_base)
	{
	case KING:
		return in_fort(to_idx) && king_span(from_idx, to_idx);
	case GUARD:
		return in_fort(to_idx) && guard_span(from_idx, to_idx);
	case BISHOP:
		return same_side(from_idx, to_idx) && bishop_span(from_idx, to_idx)
			&& _board[bishop_pin(from_idx, to_idx)] == 0;
	case KNIGHT:
		pin_idx = knight_pin(from_idx, to_idx);
		return pin_idx != from_idx && _board[pin_idx] == 0;
	case ROOK:
	case CANNON:
		if (same_rank(from_idx, to_idx))
		{
			step = (to_idx < from_idx ? -1 : 1);
		}
		else if (same_file(from_idx, to_idx))
		{
			step = (to_idx < from_idx ? -16 : 16);
		}
		else
		{
			return false;
		}
		pin_idx = from_idx + step;
		while (pin_idx != to_idx && _board[pin_idx] == 0)
		{
			pin_idx += step;
		}
		if (pin_idx == to_idx)
		{
			return to_val == 0 || from_val - self_base == ROOK;
		}
		else if (to_val != 0 && from_val - self_base == CANNON)
		{
			pin_idx += step;
			while (pin_idx != to_idx && _board[pin_idx] == 0)
			{
				pin_idx += step;
			}
			return pin_idx == to_idx;
		}
		else
		{
			return false;
		}
	case PAWN:
		if (away_side(to_idx, _player) && (to_idx == from_idx - 1 || to_idx == from_idx
			+ 1))
		{
			return true;
		}
		return to_idx == forward(from_idx, _player);
	default:
		return false;
	}
}

// 产生所有走法
uint32 Board::gen_moves(uint32 * moves, bool capture)
{
	uint32 gen_count = 0;
	uint8 self_base = base(_player);
	uint8 mate_base = base(!_player);
	int32 from_idx, to_idx;
	uint8 from_val, to_val;
	int32 step;
	//for (from_idx = 0; from_idx < 256; ++from_idx)
	for (from_idx = 0x33; from_idx <= 0xcb; ++from_idx)
	{
		from_val = _board[from_idx];
		// 判断是不是本方棋子
		if ((from_val & self_base) == 0)
			continue;

		// 根据棋子的类型生成走法
		switch (from_val - self_base)
		{
		case KING: // 帅
			for (uint32 i = 0; i < 4; i++)
			{
				to_idx = from_idx + __king_step[i];
				if (!in_fort(to_idx))
					continue;
				to_val = _board[to_idx];
				if (capture ? (to_val & mate_base) != 0
					: (to_val & self_base) == 0)
					moves[gen_count++] = move(from_idx, to_idx);
			}
			break;
		case GUARD: // 仕
			for (uint32 i = 0; i < 4; i++)
			{
				to_idx = from_idx + __guard_step[i];
				if (!in_fort(to_idx))
					continue;
				to_val = _board[to_idx];
				if (capture ? (to_val & mate_base) != 0
					: (to_val & self_base) == 0)
					moves[gen_count++] = move(from_idx, to_idx);
			}
			break;
		case BISHOP: // 象
			for (uint32 i = 0; i < 4; i++)
			{
				to_idx = from_idx + __guard_step[i];
				if (!(in_board(to_idx) && home_side(to_idx, _player)
					&& _board[to_idx] == 0))
					continue;

				to_idx += __guard_step[i];
				to_val = _board[to_idx];
				if (capture ? (to_val & mate_base) != 0
					: (to_val & self_base) == 0)
					moves[gen_count++] = move(from_idx, to_idx);
			}
			break;
		case KNIGHT: // 马
			for (uint32 i = 0; i < 4; i++)
			{
				to_idx = from_idx + __king_step[i];
				if (_board[to_idx] != 0)
					continue;
				for (uint32 j = 0; j < 2; j++)
				{
					to_idx = from_idx + __knight_step[i][j];
					if (!in_board(to_idx))
						continue;
					to_val = _board[to_idx];
					if (capture ? (to_val & mate_base) != 0 : (to_val
						& self_base) == 0)
						moves[gen_count++] = move(from_idx, to_idx);
				}
			}
			break;
		case ROOK: // 车
			for (uint32 i = 0; i < 4; i++)
			{
				step = __king_step[i];
				to_idx = from_idx + step;
				while (in_board(to_idx))
				{
					to_val = _board[to_idx];
					if (to_val == 0)
					{
						if (!capture)
							moves[gen_count++] = move(from_idx, to_idx);
					}
					else
					{
						if ((to_val & mate_base) != 0)
							moves[gen_count++] = move(from_idx, to_idx);
						break;
					}
					to_idx += step;
				}
			}
			break;
		case CANNON: // 炮
			for (uint32 i = 0; i < 4; i++)
			{
				step = __king_step[i];
				to_idx = from_idx + step;
				while (in_board(to_idx))
				{
					to_val = _board[to_idx];
					if (to_val == 0)
					{
						if (!capture)
							moves[gen_count++] = move(from_idx, to_idx);
					}
					else
					{
						break;
					}
					to_idx += step;
				}
				to_idx += step;
				while (in_board(to_idx))
				{
					to_val = _board[to_idx];
					if (to_val != 0)
					{
						if ((to_val & mate_base) != 0)
							moves[gen_count++] = move(from_idx, to_idx);
						break;
					}
					to_idx += step;
				}
			}
			break;
		case PAWN: // 兵
			to_idx = forward(from_idx, _player);
			if (in_board(to_idx))
			{
				to_val = _board[to_idx];
				if (capture ? (to_val & mate_base) != 0 : (to_val
					& self_base) == 0)
					moves[gen_count++] = move(from_idx, to_idx);
			}
			if (away_side(from_idx, _player))
			{
				for (step = -1; step <= 1; step += 2)
				{
					to_idx = from_idx + step;
					if (in_board(to_idx))
					{
						to_val = _board[to_idx];
						if (capture ? (to_val & mate_base) != 0 : (to_val
							& self_base) == 0)
							moves[gen_count++] = move(from_idx, to_idx);
					}
				}
			}
			break;
		}
	}
	return gen_count;
}

// 检测重复局面
int Board::rep_status(int32 recur) const
{
	bool self_side, perp_check, opp_perp_check;
	const MoveInfo *mvs;

	self_side = false;
	perp_check = opp_perp_check = true;
	mvs = _move_list + _move_count - 1;
	while (mvs->mv != 0 && mvs->captured == 0)
	{
		if (self_side)
		{
			perp_check = perp_check && mvs->check;
			if (mvs->key == _zobr.key)
			{
				recur--;
				if (recur == 0)
				{
					return 1 + (perp_check ? 2 : 0) + (opp_perp_check ? 4 : 0);
				}
			}
		}
		else
		{
			opp_perp_check = opp_perp_check && mvs->check;
		}
		self_side = !self_side;
		mvs--;
	}
	return 0;
}






// 装入开局库
void Engine::load_book(void)
{
	std::ifstream infs;
	try
	{
		infs.open("book.dat", std::ios::in | std::ios::binary);
		infs.read((char *)_book_table, BOOK_SIZE * sizeof(BookItem));
		_book_size = infs.gcount() / sizeof(BookItem);
		infs.close();
	}
	catch (...)
	{
		_book_size = 0;
	}
}

static int compare_book(const void *lpbk1, const void *lpbk2)
{
	return ((BookItem *)lpbk1)->lock - ((BookItem *)lpbk2)->lock;
}

// 搜索开局库
int32 Engine::search_book(void)
{
	int32 i, vl, moves;
	uint32 mv;
	uint32 mvs[MAX_GEN_MOVES];
	int32 vls[MAX_GEN_MOVES];
	bool is_mirror;
	BookItem item, *pitem;
	Board mirror_board;
	// 搜索开局库的过程有以下几个步骤

	// 1. 如果没有开局库，则立即返回
	if (_book_size == 0)
		return 0;

	// 2. 搜索当前局面
	is_mirror = false;
	item.lock = _board._zobr.lock1;
	pitem = (BookItem *)bsearch(&item, _book_table,
		_book_size, sizeof(BookItem), compare_book);
	// 3. 如果没有找到，那么搜索当前局面的镜像局面
	if (pitem == NULL)
	{
		is_mirror = true;
		_board.mirror(mirror_board);
		item.lock = mirror_board._zobr.lock1;
		pitem = (BookItem *)bsearch(&item, _book_table,
			_book_size, sizeof(BookItem), compare_book);
	}
	// 4. 如果镜像局面也没找到，则立即返回
	if (pitem == NULL)
	{
		return 0;
	}
	// 5. 如果找到，则向前查第一个开局库项
	while (pitem >= _book_table && pitem->lock == item.lock)
	{
		pitem--;
	}
	pitem++;
	// 6. 把走法和分值写入到"mvs"和"vls"数组中
	vl = moves = 0;
	while (pitem < _book_table + _book_size && pitem->lock == item.lock)
	{
		mv = (is_mirror ? mirror_move(pitem->mv) : pitem->mv);
		if (_board.legal_move(mv))
		{
			mvs[moves] = mv;
			vls[moves] = pitem->vl;
			vl += vls[moves];
			moves++;
			if (moves == MAX_GEN_MOVES)
			{
				break; // 防止"BOOK.DAT"中含有异常数据
			}
		}
		pitem++;
	}
	if (vl == 0)
	{
		return 0; // 防止"BOOK.DAT"中含有异常数据
	}
	// 7. 根据权重随机选择一个走法
	vl = rand() % vl;
	for (i = 0; i < moves; i++)
	{
		vl -= vls[i];
		if (vl < 0)
		{
			break;
		}
	}
	return mvs[i];
}

// 提取置换表项
int32 Engine::probe_hash(int32 alpha, int32 beta, int32 depth, uint32 &mv)
{
	bool mate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件
	HashItem hsh;

	hsh = _hash_table[_board._zobr.key & (HASH_SIZE - 1)];
	if (hsh.lock0 != _board._zobr.lock0 || hsh.lock1 != _board._zobr.lock1)
	{
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.mv;
	mate = false;
	if (hsh.vl > WIN_VALUE)
	{
		if (hsh.vl < BAN_VALUE)
		{
			// 可能导致搜索的不稳定性，立刻退出，但最佳着法可能拿到
			return -MATE_VALUE;
		}
		hsh.vl -= _board._distance;
		mate = true;
	}
	else if (hsh.vl < -WIN_VALUE)
	{
		if (hsh.vl > -BAN_VALUE)
		{
			// 同上
			return -MATE_VALUE;
		}
		hsh.vl += _board._distance;
		mate = true;
	}
	if (hsh.depth >= depth || mate)
	{
		if (hsh.flag == HASH_BETA)
		{
			return (hsh.vl >= beta ? hsh.vl : -MATE_VALUE);
		}
		else if (hsh.flag == HASH_ALPHA)
		{
			return (hsh.vl <= alpha ? hsh.vl : -MATE_VALUE);
		}
		return hsh.vl;
	}
	return -MATE_VALUE;
}

// 保存置换表项
void Engine::record_hash(int32 flag, int32 vl, int32 depth, uint32 mv)
{
	HashItem hsh;
	hsh = _hash_table[_board._zobr.key & (HASH_SIZE - 1)];
	if (hsh.depth > depth)
	{
		return;
	}
	hsh.flag = flag;
	hsh.depth = depth;
	if (vl > WIN_VALUE)
	{
		if (mv == 0 && vl <= BAN_VALUE)
		{
			return; // 可能导致搜索的不稳定性，并且没有最佳着法，立刻退出
		}
		hsh.vl = vl + _board._distance;
	}
	else if (vl < -WIN_VALUE)
	{
		if (mv == 0 && vl >= -BAN_VALUE)
		{
			return; // 同上
		}
		hsh.vl = vl - _board._distance;
	}
	else
	{
		hsh.vl = vl;
	}
	hsh.mv = mv;
	hsh.lock0 = _board._zobr.lock0;
	hsh.lock1 = _board._zobr.lock1;
	_hash_table[_board._zobr.key & (HASH_SIZE - 1)] = hsh;
}


// MVV/LVA每种子力的价值
extern const uint8 __mvv_lva[24] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 1, 3, 4, 3, 2, 0, 5, 1, 1, 3, 4, 3, 2, 0 };


// 按MVV/LVA值排序的比较函数
class CompareMvvLva
{
public:
	inline CompareMvvLva(Board& board);
	inline bool operator()(uint32 mv1, uint32 mv2);
private:
	Board& _board;
};
CompareMvvLva::CompareMvvLva(Board& board)
	: _board(board)
{
}
inline bool CompareMvvLva::operator()(uint32 mv1, uint32 mv2)
{
	return _board.mvv_lva(mv1) > _board.mvv_lva(mv2);
}


// 按历史表排序的比较函数
class CompareHistory
{
public:
	inline CompareHistory(int32* history_table);
	inline bool operator()(uint32 mv1, uint32 mv2);
private:
	int32* _history_table;
};
inline CompareHistory::CompareHistory(int32* history_table)
	: _history_table(history_table)
{
}
inline bool CompareHistory::operator()(uint32 mv1, uint32 mv2)
{
	return _history_table[mv1] > _history_table[mv2];
}


// 走法排序阶段
#define PHASE_HASH			0
#define PHASE_KILLER_1		1
#define PHASE_KILLER_2		2
#define PHASE_GEN_MOVES		3
#define PHASE_REST			4

// 走法排序结构
struct SortStruct
{
	inline SortStruct(Engine& engine);
	uint32 mv_hash, mv_killer1, mv_killer2; // 置换表走法和两个杀手走法
	int32 phase, index, moves; // 当前阶段，当前采用第几个走法，总共有几个走法
	uint32 mvs[MAX_GEN_MOVES]; // 所有的走法

	inline void init(uint32 mv_hash_);
	// 得到下一个走法
	int next(void);
	Engine& _engine;
};

inline SortStruct::SortStruct(Engine& engine) : _engine(engine)
{
}

inline void SortStruct::init(uint32 mv_hash_)
{ // 初始化，设定置换表走法和两个杀手走法
	mv_hash = mv_hash_;
	mv_killer1 = _engine._killers[_engine._board._distance][0];
	mv_killer2 = _engine._killers[_engine._board._distance][1];
	phase = PHASE_HASH;
}

// 得到下一个走法
int SortStruct::next(void)
{
	uint32 mv;
	CompareHistory comp_his(_engine._history_table);
	switch (phase)
	{
		// "phase"表示着法启发的若干阶段，依次为：

		// 0. 置换表着法启发，完成后立即进入下一阶段；
	case PHASE_HASH:
		phase = PHASE_KILLER_1;
		if (mv_hash != 0)
		{
			return mv_hash;
		}
		// 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同

		// 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
	case PHASE_KILLER_1:
		phase = PHASE_KILLER_2;
		if (mv_killer1 != mv_hash && mv_killer1 != 0
			&& _engine._board.legal_move(mv_killer1))
		{
			return mv_killer1;
		}

		// 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
	case PHASE_KILLER_2:
		phase = PHASE_GEN_MOVES;
		if (mv_killer2 != mv_hash && mv_killer2 != 0
			&& _engine._board.legal_move(mv_killer2))
		{
			return mv_killer2;
		}

		// 3. 生成所有着法，完成后立即进入下一阶段；
	case PHASE_GEN_MOVES:
		phase = PHASE_REST;
		moves = _engine._board.gen_moves(mvs);
		std::sort(mvs, mvs + moves, comp_his);
		index = 0;

		// 4. 对剩余着法做历史表启发；
	case PHASE_REST:
		while (index < moves)
		{
			mv = mvs[index];
			index++;
			if (mv != mv_hash && mv != mv_killer1 && mv != mv_killer2)
			{
				return mv;
			}
		}

		// 5. 没有着法了，返回零。
	default:
		return 0;
	}
}





Engine::Engine()
{
	load_book();
	init();
}

// 静态(Quiescence)搜索过程
int32 Engine::search_quiesc(int alpha, int beta)
{
	int32 i, moves;
	int32 vl, vl_best;
	uint32 mvs[MAX_GEN_MOVES];
	CompareHistory comp_his(_history_table);
	CompareMvvLva comp_mvvlva(_board);

	// 一个静态搜索分为以下几个阶段

	// 1. 检查重复局面
	vl = _board.rep_status();
	if (vl != 0)
	{
		return _board.rep_value(vl);
	}

	// 2. 到达极限深度就返回局面评价
	if (_board._distance == LIMIT_DEPTH)
	{
		return _board.evaluate();
	}

	// 3. 初始化最佳值
	vl_best = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)

	if (_board.in_check())
	{
		// 4. 如果被将军，则生成全部走法
		moves = _board.gen_moves(mvs);
		std::sort(mvs, mvs + moves, comp_his);
	}
	else
	{

		// 5. 如果不被将军，先做局面评价
		vl = _board.evaluate();
		if (vl > vl_best)
		{
			vl_best = vl;
			if (vl >= beta)
			{
				return vl;
			}
			if (vl > alpha)
			{
				alpha = vl;
			}
		}

		// 6. 如果局面评价没有截断，再生成吃子走法
		moves = _board.gen_moves(mvs, true);
		std::sort(mvs, mvs + moves, comp_mvvlva);
	}

	// 7. 逐一走这些走法，并进行递归
	for (i = 0; i < moves; i++)
	{
		if (_board.make_move(mvs[i]))
		{
			vl = -search_quiesc(-beta, -alpha);
			_board.undo_make_move();

			// 8. 进行Alpha-Beta大小判断和截断
			if (vl > vl_best)
			{ // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
				vl_best = vl; // "vl_best"就是目前要返回的最佳值，可能超出Alpha-Beta边界
				if (vl >= beta)
				{ // 找到一个Beta走法
					return vl; // Beta截断
				}
				if (vl > alpha)
				{ // 找到一个PV走法
					alpha = vl; // 缩小Alpha-Beta边界
				}
			}
		}
	}
	// 9. 所有走法都搜索完了，返回最佳值
	return vl_best == -MATE_VALUE ? _board._distance - MATE_VALUE : vl_best;
}


// 超出边界(Fail-Soft)的Alpha-Beta搜索过程
int32 Engine::search_full(int32 alpha, int32 beta, int32 depth, bool no_null)
{
	int32 hash_flag, vl, vl_best;
	uint32 mv, mv_best, mv_hash;
	int32 new_depth;
	SortStruct sort_data(*this);
	// 一个Alpha-Beta完全搜索分为以下几个阶段

	// 1. 到达水平线，则调用静态搜索(注意：由于空步裁剪，深度可能小于零)
	if (depth <= 0)
	{
		return search_quiesc(alpha, beta);
	}

	// 1-1. 检查重复局面(注意：不要在根节点检查，否则就没有走法了)
	vl = _board.rep_status();
	if (vl != 0)
	{
		return _board.rep_value(vl);
	}

	// 1-2. 到达极限深度就返回局面评价
	if (_board._distance == LIMIT_DEPTH)
	{
		return _board.evaluate();
	}

	// 1-3. 尝试置换表裁剪，并得到置换表走法
	vl = probe_hash(alpha, beta, depth, mv_hash);
	if (vl > -MATE_VALUE)
	{
		return vl;
	}

	// 1-4. 尝试空步裁剪(根节点的Beta值是"MATE_VALUE"，所以不可能发生空步裁剪)
	if (!no_null && !_board.in_check() && _board.null_okay())
	{
		_board.make_null_move();
		vl = -search_full(-beta, 1 - beta, depth - NULL_DEPTH - 1, true);
		_board.undo_make_null_move();
		if (vl >= beta)
		{
			return vl;
		}
	}

	// 2. 初始化最佳值和最佳走法
	hash_flag = HASH_ALPHA;
	vl_best = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)
	mv_best = 0; // 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

				 // 3. 初始化走法排序结构
	sort_data.init(mv_hash);

	// 4. 逐一走这些走法，并进行递归
	while ((mv = sort_data.next()) != 0)
	{
		if (_board.make_move(mv))
		{
			// 将军延伸
			new_depth = _board.in_check() ? depth : depth - 1;
			// PVS
			if (vl_best == -MATE_VALUE)
			{
				vl = -search_full(-beta, -alpha, new_depth);
			}
			else
			{
				vl = -search_full(-alpha - 1, -alpha, new_depth);
				if (vl > alpha && vl < beta)
				{
					vl = -search_full(-beta, -alpha, new_depth);
				}
			}
			_board.undo_make_move();

			// 5. 进行Alpha-Beta大小判断和截断
			if (vl > vl_best)
			{ // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
				vl_best = vl; // "vl_best"就是目前要返回的最佳值，可能超出Alpha-Beta边界
				if (vl >= beta)
				{ // 找到一个Beta走法
					hash_flag = HASH_BETA;
					mv_best = mv; // Beta走法要保存到历史表
					break; // Beta截断
				}
				if (vl > alpha)
				{ // 找到一个PV走法
					hash_flag = HASH_PV;
					mv_best = mv; // PV走法要保存到历史表
					alpha = vl; // 缩小Alpha-Beta边界
				}
			}
		}
	}

	// 5. 所有走法都搜索完了，把最佳走法(不能是Alpha走法)保存到历史表，返回最佳值
	if (vl_best == -MATE_VALUE)
	{
		// 如果是杀棋，就根据杀棋步数给出评价
		return _board._distance - MATE_VALUE;
	}
	// 记录到置换表
	record_hash(hash_flag, vl_best, depth, mv_best);
	if (mv_best != 0)
	{
		// 如果不是Alpha走法，就将最佳走法保存到历史表
		set_best_move(mv_best, depth);
	}
	return vl_best;
}



// 根节点的Alpha-Beta搜索过程
int32 Engine::search_root(int depth)
{
	int32 vl, vl_best, new_depth;
	uint32 mv;
	SortStruct sort_data(*this);

	vl_best = -MATE_VALUE;
	sort_data.init(_mv_result);
	while ((mv = sort_data.next()) != 0)
	{
		if (_board.make_move(mv))
		{
			new_depth = _board.in_check() ? depth : depth - 1;
			if (vl_best == -MATE_VALUE)
			{
				vl = -search_full(-MATE_VALUE, MATE_VALUE, new_depth, true);
			}
			else
			{
				vl = -search_full(-vl_best - 1, -vl_best, new_depth);
				if (vl > vl_best)
				{
					vl = -search_full(-MATE_VALUE, -vl_best, new_depth, true);
				}
			}
			_board.undo_make_move();
			if (vl > vl_best)
			{
				vl_best = vl;
				_mv_result = mv;
				if (vl_best > -WIN_VALUE && vl_best < WIN_VALUE)
				{
					vl_best += (rand() & RANDOM_MASK) - (rand() & RANDOM_MASK);
				}
			}
		}
	}
	record_hash(HASH_PV, vl_best, depth, _mv_result);
	set_best_move(_mv_result, depth);
	return vl_best;
}


// 迭代加深搜索过程
void Engine::search_main(float sec)
{
	int32 i, t, vl, moves;
	uint32 mvs[MAX_GEN_MOVES];

	// 初始化
	memset(_history_table, 0, 65536 * sizeof(int32)); // 清空历史表
	memset(_killers, 0, LIMIT_DEPTH * 2 * sizeof(uint32)); // 清空杀手走法表
	memset(_hash_table, 0, HASH_SIZE * sizeof(HashItem)); // 清空置换表
	t = clock(); // 初始化定时器
	_board._distance = 0; // 初始步数

						  // 搜索开局库
	_mv_result = search_book();
	if (_mv_result != 0)
	{
		_board.make_move(_mv_result);
		if (_board.rep_status(3) == 0)
		{
			_board.undo_make_move();
			return;
		}
		_board.undo_make_move();
	}

	// 检查是否只有唯一走法
	vl = 0;
	moves = _board.gen_moves(mvs);
	for (i = 0; i < moves; i++)
	{
		if (_board.make_move(mvs[i]))
		{
			_board.undo_make_move();
			_mv_result = mvs[i];
			vl++;
		}
	}
	if (vl == 1)
	{
		return;
	}

	// 迭代加深过程
	for (i = 1; i <= LIMIT_DEPTH; i++)
	{
		vl = search_root(i);
		// 搜索到杀棋，就终止搜索
		if (vl > WIN_VALUE || vl < -WIN_VALUE)
		{
			break;
		}
		// 超过一秒，就终止搜索
		if (clock() - t > CLOCKS_PER_SEC * sec)
		{
			break;
		}
	}
}



