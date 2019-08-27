
/* coding=UTF-8 */

/*
 * 作者：向量
 * 日期：2019-08-11
 * 摘要：包含 CvZ 所有接口
 * 命名格式：类的公有成员函数为小驼峰命名规则，普通函数和类为大驼峰命名规则，
 *		   其他标识符方式均为下划线
 */

#pragma once
#ifndef LIBPVZ_H
#define LIBPVZ_H

#include <cstdio>
#include <fstream>
#include <Windows.h>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <initializer_list>
#include <algorithm>

//用于储存位置信息
struct GRID
{
	int row;
	int col;
	//重载 == 运算符
	friend const bool operator==(const GRID &grid1, const GRID &grid2)
	{
		return grid1.row == grid2.row && grid1.col == grid2.col;
	}
};

//用于选卡函数(根据卡片对象序列选择)
struct SEED_INDEX
{
	int row;
	int col;
	bool imitator = false;
};

//用于选卡函数(根据卡片名称选择)
struct SEED_NAME
{
	std::string seed_name;
};

//用于铲除函数
struct SHOVEL
{
	int row;
	float col;
	bool pumpkin = false;
};

struct CARD
{
	int row;
	float col;
};

//用于用卡函数(根据对象序列用卡)
struct CARD_INDEX
{
	int x;
	int row;
	float col;
};

//用于选卡函数(根据名称用卡)
struct CARD_NAME
{
	std::string card_name;
	int row;
	float col;
};

extern HANDLE handle;
extern int wave;

//包含内存读取相关函数
//熟悉并掌握使用此命名空间的函数可以定制属于自己的“自动”函数
namespace pvz_memory
{
extern int mainobject;

//读取内存函数
template <typename T, typename... Args>
T ReadMemory(Args... args)
{
	std::initializer_list<uintptr_t> lst = {static_cast<uintptr_t>(args)...};
	uintptr_t buff = 0;
	T result = T();
	for (auto it = lst.begin(); it != lst.end(); it++)
		if (it != lst.end() - 1)
			ReadProcessMemory(handle, (const void *)(buff + *it), &buff, sizeof(buff), nullptr);
		else
			ReadProcessMemory(handle, (const void *)(buff + *it), &result, sizeof(result), nullptr);
	return result;
}

//改写内存函数
template <typename T, typename... Args>
void WriteMemory(T value, Args... args)
{
	std::initializer_list<uintptr_t> lst = {static_cast<uintptr_t>(args)...};
	uintptr_t buff = 0;
	for (auto it = lst.begin(); it != lst.end(); it++)
		if (it != lst.end() - 1)
			ReadProcessMemory(handle, (const void *)(buff + *it), &buff, sizeof(buff), nullptr);
		else
			WriteProcessMemory(handle, (void *)(buff + *it), &value, sizeof(value), nullptr);
}

//判断鼠标是否在游戏窗口内
bool MouseInGame();

//返回鼠标所在行
int MouseRow();

//返回鼠标所在列
float MouseCol();

/* time */

//判断游戏是否暂停
bool GamePaused();

//获取游戏当前游戏时钟
int GameClock();

//一个内部时钟, 可用于判断舞王/伴舞的舞蹈/前进
int DancerClock();

//返回刷新倒计时
int Countdown();

//返回大波刷新倒计时
int HugeWaveCountdown();

//返回已刷新波数
int NowWave();

//返回初始刷新倒计时
int InitialCountdown();

//抽象内存类
class AbstractMemory
{
protected:
	int offset; //地址偏移
	int index;  //对象序列/栈位/序号...

public:
	AbstractMemory() { offset = index = 0; }
	//设置对象序列
	void setIndex(int _index) { index = _index; }
	//重新得到地址偏移信息
	virtual void getOffset() = 0;
};

//卡片内存信息类
class SeedMemory : public AbstractMemory
{
public:
	SeedMemory(int _index);
	SeedMemory();

	virtual void getOffset();

	//返回卡槽数目
	int slotsCount()
	{
		return ReadMemory<int>(offset + 0x24);
	}

	//判断卡片是否可用
	bool isUsable()
	{
		return ReadMemory<bool>(offset + 0x70 + 0x50 * index);
	}

	//返回卡片冷却
	int CD()
	{
		return ReadMemory<int>(offset + 0x4C + index * 0x50);
	}

	//返回卡片总冷却时间
	int initialCD()
	{
		return ReadMemory<int>(offset + 0x50 + index * 0x50);
	}

	//返回模仿者卡片类型
	int imitatorType()
	{
		return ReadMemory<int>(offset + 0x60 + index * 0x50);
	}

	//返回卡片类型
	int type()
	{
		return ReadMemory<int>(offset + 0x5C + index * 0x50);
	}
};

class PlantMemory : public AbstractMemory
{
public:
	PlantMemory(int _index);
	PlantMemory();

	virtual void getOffset();

	//返回当前最大植物数目
	int countMax()
	{
		return ReadMemory<int>(mainobject + 0xB0);
	}

	//返回植物所在行 数值范围：[0,5]
	int row()
	{
		return ReadMemory<int>(offset + 0x1C + 0x14C * index);
	}

	//返回植物所在列 数值范围：[0,8]
	int col()
	{
		return ReadMemory<int>(offset + 0x28 + 0x14C * index);
	}

	//判断植物是否消失
	bool isDisappeared()
	{
		return ReadMemory<bool>(offset + 0x141 + 0x14C * index);
	}

	//判断植物是否被压扁
	bool isCrushed()
	{
		return ReadMemory<bool>(offset + 0x142 + 0x14C * index);
	}

	//返回植物类型 与图鉴顺序一样，从0开始
	int type()
	{
		return ReadMemory<int>(offset + 0x24 + 0x14C * index);
	}

	//返回植物横坐标 数值范围：[0,800]
	int abscissa()
	{
		return ReadMemory<int>(offset + 0x8 + 0x14C * index);
	}

	//返回植物纵坐标
	int ordinate()
	{
		return ReadMemory<int>(offset + 0xC + 0x14C * index);
	}
	//返回植物血量
	int hp()
	{
		return ReadMemory<int>(offset + 0x40 + 0x14C * index);
	}

	//发射子弹倒计时，该倒计时不论是否有僵尸一直不断减小并重置
	int bulletCountdown()
	{
		return ReadMemory<int>(offset + 0x58 + 0x14C * index);
	}

	//子弹发射倒计时，该倒计时只在有僵尸时才一直不断减小并重置
	int countdownBullet()
	{
		return ReadMemory<int>(offset + 0x90 + 0x14C * index);
	}

	//返回植物的状态
	//35：空炮
	//36：正在装填
	//37：准备就绪
	//38：正在发射
	int status()
	{
		return ReadMemory<int>(offset + 0x3C + 0x14C * index);
	}

	//植物属性倒计时
	int statusCountdown()
	{
		return ReadMemory<int>(offset + 0x54 + 0x14C * index);
	}
};

class ZombieMemory : public AbstractMemory
{
public:
	ZombieMemory(int _index);
	ZombieMemory();

	virtual void getOffset();

	//返回僵尸最大数目
	int countMax()
	{
		return ReadMemory<int>(mainobject + 0x94);
	}

	//判断僵尸是否存在
	bool isExist()
	{
		return ReadMemory<short>(offset + 0x15A + 0x15C * index);
	}

	//返回僵尸所在行数 范围：[0,5]
	int row()
	{
		return ReadMemory<int>(offset + 0x1C + 0x15C * index);
	}

	//返回僵尸所在横坐标 范围：[0,800]
	float abscissa()
	{
		return ReadMemory<float>(offset + 0x2C + 0x15C * index);
	}

	//返回僵尸纵坐标
	float ordinate()
	{
		return ReadMemory<float>(offset + 0x30 + 0x15C * index);
	}

	//返回僵尸类型 与图鉴顺序一样，从0开始
	int type()
	{
		return ReadMemory<int>(offset + 0x24 + 0x15C * index);
	}

	//返回僵尸本体当前血量
	int hp()
	{
		return ReadMemory<int>(offset + 0xC8 + 0x15C * index);
	}

	//返回僵尸一类饰品当前血量
	int oneHp()
	{
		return ReadMemory<int>(offset + 0xD0 + 0x15C * index);
	}

	//返回僵尸二类饰品血量
	int twoHp()
	{
		return ReadMemory<int>(offset + 0xDC + 0x15C * index);
	}

	//判断僵尸是否啃食
	bool isEat()
	{
		return ReadMemory<bool>(offset + 0x51 + 0x15C * index);
	}

	//返回僵尸状态
	int status()
	{
		return ReadMemory<int>(offset + 0x28 + 0x15C * index);
	}

	//判断僵尸是否死亡
	bool isDead()
	{
		return status() == 1;
	}

	//判断巨人是否举锤
	bool isHammering()
	{
		return status() == 70;
	}

	//判断僵尸是否隐形
	bool isStealth()
	{
		return ReadMemory<bool>(offset + 0x18 + 0x15C * index);
	}

	//变化量(前进的舞王和减速的冰车等的前进速度)
	float speed()
	{
		return ReadMemory<float>(offset + 0x34 + 0x15C * index);
	}

	//僵尸已存在时间
	int existTime()
	{
		return ReadMemory<int>(offset + 0x60 + 0x15C * index);
	}

	//僵尸属性倒计时
	int statusCountdown()
	{
		return ReadMemory<int>(offset + 0x68 + 0x15C * index);
	}

	//判断僵尸是否消失
	bool isDisappeared()
	{
		return ReadMemory<bool>(offset + 0xEC + 0x15C * index);
	}

	//僵尸中弹判定的横坐标
	int bulletAbscissa()
	{
		return ReadMemory<int>(offset + 0x8C + 0x15C * index);
	}

	//僵尸中弹判定的纵坐标
	int bulletOrdinate()
	{
		return ReadMemory<int>(offset + 0x90 + 0x15C * index);
	}

	//僵尸攻击判定的横坐标
	int attackAbscissa()
	{
		return ReadMemory<int>(offset + 0x9C + 0x15C * index);
	}

	//僵尸攻击判定的纵坐标
	int attackOrdinate()
	{
		return ReadMemory<int>(offset + 0xA0 + 0x15C * index);
	}

	//僵尸减速倒计时
	int slowCountdown()
	{
		return ReadMemory<int>(offset + 0xAC + 0x15C * index);
	}

	//僵尸黄油固定倒计时
	int fixationCountdown()
	{
		return ReadMemory<int>(offset + 0xB0 + 0x15C * index);
	}

	//僵尸冻结倒计时
	int freezeCountdown()
	{
		return ReadMemory<int>(offset + 0xB4 + 0x15C * index);
	}
};

class PlaceMemory : public AbstractMemory
{
public:
	PlaceMemory(int _index);
	PlaceMemory();

	virtual void getOffset();

	//返回场景物品最大数目
	//包括：弹坑 墓碑 罐子等
	int countMax()
	{
		return ReadMemory<int>(mainobject + 0x120);
	}

	//返回该格子物品的类型
	int type()
	{
		return ReadMemory<int>(offset + 0x8 + 0xEC * index);
	}

	//返回物品的行数 范围：[0,5]
	int row()
	{
		return ReadMemory<int>(offset + 0x14 + 0xEC * index);
	}

	//返回物品的列数 范围：[0,8]
	int col()
	{
		return ReadMemory<int>(offset + 0x10 + 0xEC * index);
	}

	//判断物品是否存在
	bool isExist()
	{
		return ReadMemory<short>(offset + 0xEA + 0xEC * index);
	}
};

//返回一行的冰道坐标 范围：[0,800]
//使用示例：
//ice_absci(0)------得到第一行的冰道坐标
int IceAbscissa(int i);

//获取游戏信息
//1: 主界面, 2: 选卡, 3: 正常游戏/战斗, 4: 僵尸进屋, 7: 模式选择.
int GameUi();

/* 一些常用的函数 */

//等待游戏开始
void WaitGameStart();

//获取指定类型植物的卡槽对象序列 植物类型与图鉴顺序相同，从0开始
//返回的卡片对象序列范围：[0,9]
//GetSeedIndex(16)------------获得荷叶的卡槽对象序列
//GetSeedIndex(16,true)-------获得模仿者荷叶的卡槽对象序列
int GetSeedIndex(int type, bool imitator = false);

//得到指定位置和类型的植物对象序列
//当参数type为默认值-1时该函数无视南瓜花盆荷叶
//使用示例：
//GetPlantIndex(3,4)------如果三行四列有除南瓜花盆荷叶之外的植物时，返回该植物的对象序列，否则返回-1
//GetPlantIndex(3,4,47)---如果三行四列有春哥，返回其对象序列，否则返回-1
//GetPlantIndex(3,4,33)---如果三行四列有花盆，返回其对象序列，否则返回-1
int GetPlantIndex(int row, int col, int type = -1);

//检查僵尸是否存在
//使用示例
//ExamineZombieExist()-------检查场上是否存在僵尸
//ExamineZombieExist(23)-------检查场上是否存在巨人僵尸
//ExamineZombieExist(-1,4)-----检查第四行是否有僵尸存在
//ExamineZombieExist(23,4)-----检查第四行是否有巨人僵尸存在
bool ExamineZombieExist(int type = -1, int row = -1);

//判断该格子是否能种植物
//该函数不适用于屋顶场景
//此函数只能判断此格子能不能种植物，但并不能判断此格子是否有植物
//使用示例：
//IsPlantable(3,3)-----3行3列如果能种植物，默认为非灰烬植物(即使此格子有非南瓜花盆荷叶的植物)，返回true,否则返回false
//IsPlantable(3,3,true)----3行3列如果能种植物，为灰烬植物，返回true,否则返回false
//如果判定的植物是灰烬植物则会无视冰车
bool IsPlantable(int row, int col, bool hui_jin = false);

//检测巨人是否对该格子植物锤击
//IsHammering(3,4)------------如果该格子有巨人不同时举锤返回true，否则返回false.
bool IsHammering(int row, int col, bool pumpkin = false);

//检测该格子是否会被即将爆炸的小丑炸到
//使用示例：IsExplode(3,4)-------如果（3，4）会被小丑炸到返回true 否则返回false.
bool IsExplode(int row, int col);

//得到僵尸出怪类型
//参数为 vector 数组
//使用示例：
//std::vector<int>zombies_type;
//GetZombieType(zombies_type);
//僵尸的出怪类型将会储存在数组 zombies_type 中
void GetZombieType(std::vector<int> &type_list);

//得到相应波数的出怪类型
//参数为 vector 数组
//使用示例：
//std::vector<int>zombies_type;
//GetWaveZombieType(zombies_type);-------得到当前波数出怪类型
//GetWaveZombieType(zombies_type, 1);-------得到第一波出怪类型
//僵尸的出怪类型将会储存在数组 zombies_type 中
void GetWaveZombieType(std::vector<int> &zombie_type, int _wave = wave);

} // namespace pvz_memory

//包含时间操作函数
namespace pvz_time
{
//预判函数
//获取僵尸刷新时间戳
//除 wave 1 10 20 最小预判时间分别为 -550 -700 -700 之外，其他波次最小预判时间均为 -200
//使用示例：
//Prejudge(-95,wave)-----僵尸刷新前95cs进行接下来的操作
//Prejudge(100,wave)-----僵尸刷新后100cs进行接下来的操作
void Prejudge(int t, int wave);

//时间延迟函数（以游戏内部时钟为基准）
//使用示例：
//Delay(100)----延迟100cs
void Delay(int time);

//等待直至当前时间戳与本波刷新时间点的差值达到指定值
//使用该函数之前必须使用Prejudge函数
//使用示例：
//Until(-95)-----僵尸刷新前95cs进行接下来的操作
//Until(200)-----僵尸刷新后200cs进行接下来的操作
//注意：此函数使用顺序必须符合时间先后顺序！例如：
//Until(-95);
//Until(-100);
//这种用法是错误的！
void Until(int time);

} // namespace pvz_time

//包含模拟鼠标键盘操作函数
namespace pvz_simulate
{
//模拟鼠标左键操作
//使用示例：
//LeftClick(400,300)-----点击pvz窗口中央
//LeftClick(55+85,80*6)------点击泳池(1,6)
void LeftClick(int x, int y);

//模拟鼠标右键点击操作
//主要用于安全点击
void RightClick(int x, int y);

//模拟敲击空格键
void PressSpace();

//点击函数
//此函数自动识别场景，只用于战斗界面
//0: 白天, 1 : 黑夜, 2 : 泳池, 3 : 浓雾, 4 : 屋顶, 5 : 月夜, 6 : 蘑菇园, 7 : 禅境花园, 8 : 水族馆, 9 : 智慧树.
//使用示例：
//SceneClick(3, 4)------点击场地上3行4列
//SceneClick(3, 4, 10)-----点击场地上3行4列再向下偏移10px
void SceneClick(float row, float col, int offset = 0);

//铲除植物函数
//使用示例：
//Shovel(4,6)--------铲除4行6列的植物,如果植物有南瓜保护默认铲除被保护植物
//Shovel(4,6,true)---铲除4行6列的植物,如果植物有南瓜保护铲除南瓜
void Shovel(int row, float col, bool pumpkin = false);

//铲除植物函数
//使用示例：
//Shovel(4,6)--------铲除4行6列的植物,如果植物有南瓜保护默认铲除被保护植物
//Shovel(4,6,true)---铲除4行6列的植物,如果植物有南瓜保护铲除南瓜
void Shovel(std::initializer_list<SHOVEL> lst);

//检测某键是否按下
//KeyDown('Q')--------若Q键按下，返回true，否则返回false
//while(1)
//{Sleep(1); if(KeyDown('Q')) Card("ytzd",2,9);}--------不断检测Q键状态，一旦Q键按下，则在二行九列使用樱桃炸弹
//注意此函数参数书写形式为单引号
inline bool KeyDown(int key)
{
	if (key > 96 && key < 123)
		key -= 32;
	return GetAsyncKeyState(key) & 0x8001 == 0x8001;
}

} // namespace pvz_simulate

//包含用卡操作函数
namespace pvz_card
{

//选择卡片用来战斗
//使用示例：
//SelectCards({ {2,7},{2,7,true},{5,4},{2,8},{3,1},{3,2},{1,3},{4,4},{4,7},{2,1} });
//true为使用模仿者,默认不使用
//卡片名称为拼音首字母，具体参考pvz_data.h的命名
//选择 寒冰菇，模仿寒冰菇，咖啡豆，窝瓜，樱桃炸弹，南瓜头，小喷菇，阳光菇，胆小菇，向日葵 卡片
//SelectCards({{"hbg"}, {"Mhbg"}, {"kfd"}, {"wg"}, {"ytzd"}, {"ngt"}, {"xpg"}, {"yyg"}, {"dxg"}, {"xrk"}});
void SelectCards(const std::vector<SEED_INDEX> lst);

//选择卡片用来战斗
//使用示例：
//SelectCards({ {2,7},{2,7,true},{5,4},{2,8},{3,1},{3,2},{1,3},{4,4},{4,7},{2,1} });
//true为使用模仿者,默认不使用
//卡片名称为拼音首字母，具体参考pvz_data.h的命名
//选择 寒冰菇，模仿寒冰菇，咖啡豆，窝瓜，樱桃炸弹，南瓜头，小喷菇，阳光菇，胆小菇，向日葵 卡片
//SelectCards({{"hbg"}, {"Mhbg"}, {"kfd"}, {"wg"}, {"ytzd"}, {"ngt"}, {"xpg"}, {"yyg"}, {"dxg"}, {"xrk"}});
void SelectCards(std::initializer_list<SEED_NAME> lst);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(int x, int row, float col);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(std::initializer_list<CARD_INDEX> lst);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(int x, std::initializer_list<CARD> lst);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(const std::string &card_name, int row, float col);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(std::initializer_list<CARD_NAME> lst);

//用卡函数
//使用示例：
//Card(1,2,3)---------选取第1张卡片，放在2行,3列
//Card({{1,2,3},{2,3,4}})------选取第1张卡片，放在2行,3列，选取第2张卡片，放在3行,4列
//Card(1,{{2,3},{3,4}})--------选取第1张卡片，优先放在2行,3列，其次放在3行,4列
//以下用卡片名称使用Card,卡片名称为拼音首字母，具体参考pvz_data.h的命名
//Card({{"ytzd",2,3},{"Mhblj",3,4}})---------选取樱桃卡片，放在2行,3列，选取辣椒卡片，放在3行,4列
void Card(const std::string &card_name, std::initializer_list<CARD> lst);

} // namespace pvz_card

//包含用炮操作函数
namespace pvz_pao
{

//记录炮的信息
struct PAO_MESSAGE
{
	int row;				 //所在行
	int col;				 //所在列
	int recover_time;		 //恢复时间
	int index;				 //炮的对象序列
	bool is_in_list = false; //记录是否在炮列表内

	//重载 ==
	friend bool operator==(const PAO_MESSAGE &p1, const PAO_MESSAGE &p2)
	{
		return p1.row == p2.row && p1.col == p2.col;
	}

	PAO_MESSAGE() : row(0), col(0), recover_time(0), index(0) {}
	PAO_MESSAGE(int _row, int _col) : row(_row), col(_col), recover_time(0), index(0) {}
};

//炮操作类：使用炮操作类可以建立多个炮列表，使得复杂的操作能够更好的实现
//public 成员函数介绍：
//resetPaoList：手动重置炮列表，与UpdatePaolist用法相同
//skipPao：跳过一定数量的炮，用法与SkipPao相同
//rawPao：用户自定义位置发射炮，用法与RawPao相同
//pao：用炮函数，用法与Pao相同
//recoverPao：等待炮恢复立即用炮，用法与RecoverPao相同
//roofPao：屋顶修正时间发炮，用法与RoofPao相同
//rawRoofPao：屋顶用户自定义位置修正时发炮，用法与RawRoofPao相同
//fixPao：铲种炮，用法与FixPao相同
class PaoOperator
{
private:
	//PAO_MESSAGE 迭代器
	using pao_message_iterator = std::vector<PAO_MESSAGE>::iterator;

	//用于发炮函数
	struct PAO
	{
		int row;
		float col;
	};

	//用于RAWPAO函数
	struct RAWPAO
	{
		int pao_row;
		int pao_col;
		int drop_row;
		float drop_col;
	};

	//记录炮的位置和落点的位置及炮弹飞行时间
	struct RP
	{
		pao_message_iterator it;
		int drop_row;
		float drop_col;
		int fire_time;
	};
	//冲突解决方式
	static int conflict_resolution_type;
	//炮列表，记录炮的迭代器信息
	std::vector<pao_message_iterator> paolist;
	//记录当前即将发射的下一门炮
	int nowpao;
	//是否限制炮序
	bool limit_pao_sequence;
	//屋顶炮飞行时间辅助数据
	struct
	{
		int min_drop_x;   //记录该列炮最小飞行时间对应的最小的横坐标
		int min_fly_time; //记录最小的飞行时间
	} roof_pao_fly_time[8]{
		{515, 359}, {499, 362}, {515, 364}, {499, 367}, {515, 369}, {499, 372}, {511, 373}, {511, 373}}; //辅助排序函数

	//对炮进行一些检查
	void pao_examine(pao_message_iterator it, int now_time, int drop_row, float drop_col);
	//检查落点
	bool is_drop_conflict(int pao_row, int pao_col, int drop_row, float drop_col);
	//基础发炮函数
	void base_fire_pao(pao_message_iterator it, int now_time, int drop_row, float drop_col);
	//获取屋顶炮飞行时间
	int roof_fly_time(int pao_col, float drop_col);
	//基础屋顶修正时间发炮
	void base_fire_roof_pao(pao_message_iterator it, int fire_time, int drop_row, float drop_col);
	//用户自定义炮位置，屋顶修正时间发炮：多发
	void base_fire_roof_paos(std::vector<RP> lst);
	//铲种炮
	void shovel_and_plant_pao(int row, int col, int move_col, pao_message_iterator it, int delay_time);
	//改变炮的信息
	static void change_pao_message(pao_message_iterator it, int now_row, int now_col);

public:
	//冲突参数
	enum
	{
		DROP_POINT,
		COLLECTION
	};

	//构造函数，完成变量初始化
	PaoOperator(std::initializer_list<GRID> lst);
	PaoOperator();
	~PaoOperator();

	//设置炮序限制 参数为 false 则解除炮序限制，true 则增加炮序限制
	//解除此限制后 fixPao 可铲种炮列表内的炮，tryPao 系列可使用， Pao 系列不可使用
	//增加此限制后 fixPao 不可铲种炮列表内的炮，tryPao 系列不可使用， Pao 系列可使用
	void setLimitPaoSequence(bool limit) { limit_pao_sequence = limit; }

	//设定解决冲突模式
	//使用示例：
	//setResolveConflictType(PaoOperator::DROP_POINT)---只解决落点冲突，不解决收集物点炮冲突
	//setResolveConflictType(PaoOperator::COLLECTION)---只解决收集物点炮冲突，不解决落点冲突
	static void setResolveConflictType(int type);

	//改变炮的信息
	//请在手动或使用基础函数例如 Card 改变炮的信息后立即使用此函数
	//使用示例：
	//changePaoMessage(2,3,2,3)--------在手动铲种(2,3)位置的炮后，更改相关炮的信息
	//changePaoMessage(2,3,2,4)--------手动位移铲种(2,3)位置的炮后，更改相关炮的信息
	//changePaoMessage(0,0,2,3)--------手动增加(2,3)位置的炮后，更改相关炮的信息
	static void changePaoMessage(int origin_row, int origin_col, int now_row, int now_col);

	//设置即将发射的下一门炮
	//此函数只有在限制炮序的时候才可调用
	void setNextPao(int next_pao) { nowpao = next_pao; }

	//重置炮列表
	void resetPaoList(const std::vector<GRID> lst);
	void resetPaoList();

	//跳过一定数量的炮
	void skipPao(int x);

	//用户自定义炮位置发炮：单发
	void rawPao(int pao_row, int pao_col, int drop_row, float drop_col);

	//用户自定义炮位置发炮：多发
	void rawPao(const std::vector<RAWPAO> &lst);

	//发炮函数：单发
	void pao(int row, float col);

	//发炮函数：多发
	void pao(std::initializer_list<PAO> lst);

	//尝试发炮：单发
	bool tryPao(int row, float col);

	//尝试发炮：多发
	bool tryPao(const std::vector<PAO> lst);

	//等待炮恢复立即用炮：单发
	void recoverPao(int row, float col);

	//等待炮恢复立即用炮：多发
	void recoverPao(std::initializer_list<PAO> lst);

	//屋顶修正时间发炮：单发
	void roofPao(int row, float col);

	//屋顶修正时间发炮：多发
	void roofPao(std::initializer_list<PAO> lst);

	//屋顶修正时间发炮：单发
	bool tryRoofPao(int row, float col);

	//屋顶修正时间发炮：多发
	bool tryRoofPao(std::initializer_list<PAO> lst);

	//屋顶修正时间发炮，单发
	void rawRoofPao(int pao_row, int pao_col, int drop_row, float drop_col);

	//屋顶修正时间发炮 多发
	void rawRoofPao(const std::vector<RAWPAO> &lst);

	//铲种炮
	void fixPao(int row, int col, int move_col = 0, int delay_time = 0);
};

//CvZ自定义炮类对象
extern PaoOperator pao_cvz;

} // namespace pvz_pao

//包含自动操作函数
namespace pvz_auto
{
//自动操作线程基类的基类 =_=
class BaseBaseAutoThread
{
protected:
	bool pause_;		   //暂停，线程仍在运行
	bool stop_;			   //停止，线程不再运行
	void error_messages(); //错误信息

public:
	BaseBaseAutoThread()
	{
		pause_ = false;
		stop_ = true;
	}
	virtual ~BaseBaseAutoThread() {}
	//调用此函数使得线程停止运行
	void stop() { stop_ = true; }
	//调用此函数使得线程暂停运行
	void pause() { pause_ = true; }
	//调用此函数使得线程继续运行
	void goOn() { pause_ = false; }
};

//自动操作函数基类
class BaseAutoThread : public BaseBaseAutoThread
{
protected:
	int leaf_seed_index;
	std::vector<GRID> grid_lst; //记录位置信息
	//重置填充列表
	void base_reset_list(std::initializer_list<GRID> lst);

public:
	BaseAutoThread();
	virtual ~BaseAutoThread() {}
};

//自动存用冰类
//public成员函数介绍：
//start：开始存冰，使用此函数会开辟一个线程用于存冰，使用方法与StartAutoFillIceThread相同
//stop：停止存冰，使用此函数会使得存冰线程停止运行
//pause：存冰暂停，使用此函数会暂停存冰任务，但是线程在不断运行
//goOn：继续存冰，使用此函数使得存冰线程继续工作
//resetList：重置存冰列表，使用方法与StartAutoFillIceThread相同
//coffee：使用咖啡豆，激活存冰，使用方法与Coffee相同
class FillIce : public BaseAutoThread
{
public:
	FillIce();
	~FillIce() {}

	//重置存冰位置
	//使用示例：
	//resetIceList({{3,4},{5,6}})-----将存冰位置重置为{3，4}，{5，6}
	void resetIceList(std::initializer_list<GRID> lst);

	//线程开始工作
	//使用示例：
	//start({{3,4},{5,6}})-----在{3，4}，{5，6}位置存冰
	void start(std::initializer_list<GRID> lst);

	//使用咖啡豆
	//使用此函数前必须使用start
	void coffee();

private:
	void use_ice();
	int coffee_index;
};

//自动存用冰类对象：使用此对象可以使您更愉快的管理存冰
//public成员函数介绍：
//start：开始存冰，使用此函数会开辟一个线程用于存冰，使用方法与StartAutoFillIceThread相同
//stop：停止存冰，使用此函数会使得存冰线程停止运行
//pause：存冰暂停，使用此函数会暂停存冰任务，但是线程在不断运行
//goOn：继续存冰，使用此函数使得存冰线程继续工作
//resetList：重置存冰列表，使用方法与StartAutoFillIceThread相同
//coffee：使用咖啡豆，激活存冰，使用方法与Coffee相同
extern FillIce ice_filler;

//自动修补坚果类
//public成员函数介绍：
//start：开始修补坚果，使用此函数将会开启一个线程用于修补坚果，用法与StartAutoFixNutThread相同
//stop：停止修补坚果，使用此函数会使得修补坚果线程停止运行
//pause：修补坚果暂停，使用此函数会暂停修补坚果任务，但是线程在不断运行
//goOn：继续修补坚果，使用此函数使得修补坚果线程继续工作
//resetList：重置存冰列表，使用方法与StartAutoFixNutThread相同
//resetFixHp：重置修补血量，使用示例：reset_fix_hp(300)-----当坚果的血量下降至300时进行修补
class FixNut : public BaseAutoThread
{
public:
	FixNut();
	~FixNut() {}

	//重置坚果修补位置
	//使用示例：
	//resetNutList({{2,3},{3,4}})-------位置被重置为{2，3}，{3，4}
	void resetNutList(std::initializer_list<GRID> lst);

	//线程开始工作，此函数开销较大，不建议多次调用
	//第一个参数为坚果类型：3--坚果，23--高坚果，30--南瓜头
	//第二个参数不填默认全场
	//第三个参数不填默认刚一开始损坏就修补
	//使用示例：
	//start(23)-------修补全场的高坚果
	//start(30,{{1,3},{2,3}})-----修补位置为{1，3}，{2，3}位置的南瓜头
	//start(3,{{1,3},{2,3}},300)------修补位置为{1，3}，{2，3}位置的坚果，血量降至300开始修补
	void start(int nut_type, std::initializer_list<GRID> lst = {}, int fix_hp = 8000);

	//重置修补血量
	//使用示例：
	//resetFixHp(200)------将修补触发血量改为200
	void resetFixHp(int fix_hp);

private:
	int nut_type_;
	int nut_seed_index;
	int fix_hp_;
	void update_nut();
};

//自动修补坚果类对象
//public成员函数介绍：
//start：开始修补坚果，使用此函数将会开启一个线程用于修补坚果，用法与StartAutoFixNutThread相同
//stop：停止修补坚果，使用此函数会使得修补坚果线程停止运行
//pause：修补坚果暂停，使用此函数会暂停修补坚果任务，但是线程在不断运行
//goOn：继续修补坚果，使用此函数使得修补坚果线程继续工作
//resetList：重置存冰列表，使用方法与StartAutoFixNutThread相同
//resetFixHp：重置修补血量，使用示例：reset_fix_hp(300)-----当坚果的血量下降至300时进行修补
extern FixNut nut_fixer;

//女仆秘籍类
//public成员函数介绍
//start：开始女仆秘籍，
//stop：停止女仆秘籍，
//pause：暂停女仆秘籍，
//goOn：继续女仆秘籍，
class NvPuMiJi : public BaseBaseAutoThread
{
public:
	NvPuMiJi();
	~NvPuMiJi() {}

	//开启女仆秘籍
	void start();

private:
	//停止舞伴前进
	void stop_dancer_advance();
};

//女仆秘籍类对象
//public成员函数介绍
//start：开始女仆秘籍，
//stop：停止女仆秘籍，
//pause：暂停女仆秘籍，
//goOn：继续女仆秘籍，
extern NvPuMiJi nv_pu_mi_ji;

//自动放置垫材类
//start：开始放置垫材，使用此函数会开辟一个线程用于放置垫材，使用方法与StartAutoSetDianCaiThread相同
//stop：停止放置垫材，使用此函数会使得放置垫材线程停止运行
//pause：放置垫材暂停，使用此函数会暂停放置垫材任务，但是线程在不断运行
//goOn：继续放置垫材，使用此函数使得放置垫材线程继续工作
//resetSetDianCaiList：重置垫材优先放置列表
//resetProtectedPlantList：重置保护植物位置列表
//resetPlantList：重置被用来当作垫材的植物
//resetZombieList：重置需要垫的僵尸
class PlaceDianCai : public BaseAutoThread
{
private:
	//记录一些横坐标信息
	struct ABSCI
	{
		int plant;	//记录植物最大的横坐标值
		float zombie; //记录巨人僵尸最小的横坐标值
	};

	std::vector<ABSCI> absci;
	std::vector<int> lst_plant_;
	std::vector<int> lst_zombie_;
	pvz_memory::PlantMemory plant;
	pvz_memory::ZombieMemory zombie;
	void get_min_absci_zombie();
	GRID need_diancai_grid();
	void auto_plant_diancai();

public:
	PlaceDianCai();
	~PlaceDianCai(){};

	//开启放置垫材线程
	//此函数开销较大，不建议多次调用
	//使用示例：
	//start({1,2,3,4})------将第 1 2 3 4 张卡片设为垫材，默认全场都从第九列开始垫，默认只垫 红眼 白眼
	//start({1,2,3,4},{{2,6},{4,5}})-----将第 1 2 3 4 张卡片设为垫材，只垫第 2 4 行，而且第二行从第六列开始垫，第四行从第五列开始垫
	//start({1,2,3,4},{{2,6},{4,5}},{23,32,7})----将第 1 2 3 4 张卡片设为垫材，只垫第 2 4 行，而且第二行从第六列开始垫，第四行从第五列开始垫,垫红白眼和橄榄
	void start(std::initializer_list<int> lst_plant,
			   std::initializer_list<GRID> lst_grid = {},
			   std::initializer_list<int> lst_zombie = {23, 32});

	//重置保护植物列数
	//使用示例：
	//reset_potected_plant_list()-----默认将要保护的植物的位置重置为每行的最靠前位置
	//reset_potected_plant_list({{2,3},{3,4}})------将要保护的植物的位置重置为{2,3},{3,4}
	//注意：此函数必须配合要垫的行数，如果出现重置的行数不在要垫的范围内，就会报错
	void resetProtectedPlantList(std::initializer_list<GRID> lst = {});

	//重置被当作垫材的植物
	//使用示例：
	//resetPlantList({1,2,3})----将第1 2 3张卡片设置为垫材
	void resetPlantList(std::initializer_list<int> lst) { lst_plant_ = lst; }

	//重置要垫的僵尸类型
	//使用示例：
	//resetZombieList({23,32,0})-----将要垫的僵尸改为 白眼 红眼 普僵
	void resetZombieList(std::initializer_list<int> lst) { lst_zombie_ = lst; }

	//重置垫材放置位置列表
	//使用示例：
	//resetSetDianCaiList({{2,8},{3,9}})-----将开始种垫材的位置重置为{2，8}，{3，9}
	void resetSetDianCaiList(std::initializer_list<GRID> lst);
};

//自动放置垫材类对象
//start：开始放置垫材，使用此函数会开辟一个线程用于放置垫材，使用方法与StartAutoSetDianCaiThread相同
//stop：停止放置垫材，使用此函数会使得放置垫材线程停止运行
//pause：放置垫材暂停，使用此函数会暂停放置垫材任务，但是线程在不断运行
//goOn：继续放置垫材，使用此函数使得放置垫材线程继续工作
//resetSetDianCaiList：重置垫材优先放置列表
//resetProtectedPlantList：重置保护植物位置列表
//resetPlantList：重置被用来当作垫材的植物
//resetZombieList：重置需要垫的僵尸
extern PlaceDianCai dian_cai_placer;

//开始自动收集
void StartAutoCollectThread();

//停止自动收集
void StopAutoCollectThread();

} // namespace pvz_auto

//包含cvz设置函数
namespace pvz_script
{
//开关函数：开启高精度
void OpenHighPrecision();

//开关函数：开启合理性检查（一些比较复杂的检查由此函数来控制）
void OpenExamine();

//取消自动退出机制
void CancelAutoExit();

} // namespace pvz_script

#endif //!LIBPVZ_H