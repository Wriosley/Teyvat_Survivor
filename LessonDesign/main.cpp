#include <graphics.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#pragma comment(lib, "msimg32.lib")  // 链接库
#include <string>
using namespace std;

const int WINDOW_WIDTH = 1280; // 窗口宽度
const int WINDOW_HEIGHT = 720; // 窗口高度

int idx_current_anim = 0; //当前动画的帧索引
const int PLAYER_ANIM_NUM = 6; //玩家动画帧数
const int PLAYER_SPEED = 5; // 玩家移动速度


const int PLAYER_WIDTH = 80; // 玩家宽度
const int PLAYER_HEIGHT = 80; // 玩家高度
const int SHADOW_WIDTH = 32; // 阴影宽度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;



POINT player_pos = { 500, 500 }; // 玩家初始位置

// 带透明度的图片绘制函数
inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Animation
{
public:
	Animation(LPCTSTR path, int num , int interval)
	{
		interval_ms = interval;// 帧间隔时间

		TCHAR path_file[256];
		for (size_t i = 0;i < num;i++)
		{
			swprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE(); // 创建新的IMAGE对象
			loadimage(frame, path_file);
			frame_list.push_back(frame); // 将每一帧的图片加载到列表中
		}
	}

	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i]; // 释放每一帧的图片资源
		}
	}

	void Play(int x,int y,int delta)
	{
		timer += delta+3;
		//cout << "timer：" << timer << endl; // 输出计时器值
		if (timer >= interval_ms) // 如果计时器超过帧间隔时间
		{
			timer = 0; // 重置计时器
			idx_frame = (idx_frame + 1) % frame_list.size(); // 切换到下一帧
			//cout << "idx_frame: " << idx_frame << endl; // 输出当前帧索引
		}

		putimage_alpha(x, y, frame_list[idx_frame]); // 绘制当前帧
	}
	
private:
	int interval_ms = 0; // 帧间隔时间
	int idx_frame = 0; // 当前帧索引
	int timer = 0; // 计时器，用于控制动画播放速度
	vector<IMAGE*> frame_list; // 存储动画帧>
};


class Bullet
{
public:
	POINT position = { 0,0 };

public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y,RADIUS); // 绘制子弹
	}
private:
	const int  RADIUS = 10; // 子弹半径
};

class Player
{

};

Player player;

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("./img/shadow_enemy.png")); // 加载敌人阴影图片
		anim_left = new Animation(_T("./img/enemy_left_%d.png"), 6, 45); // 敌人向左移动动画
		anim_right = new Animation(_T("./img/enemy_right_%d.png"), 6, 45); // 敌人向右移动动画

		enum class SpawnEdge
		{
			Up = 0, Down, Left, Right
		};

		SpawnEdge edge = static_cast<SpawnEdge>(rand() % 4); // 随机生成敌人出生边缘
		switch (edge)
		{
		case SpawnEdge::Up: // 上边缘
			position.x = rand() % WINDOW_WIDTH; // 随机生成x坐标
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH; // 随机生成x坐标
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left: // 左边缘
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT; // 随机生成y坐标
			break;
		case SpawnEdge::Right: // 右边缘
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT; // 随机生成y坐标
			break;
		default:
			break;
		}
		


	}

	~Enemy()
	{
		delete anim_left; // 释放向左移动动画资源
		delete anim_right; // 释放向右移动动画资源
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		return false;
	}

	bool CheckBulletCollision(const Player& player)
	{
		return false;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player_pos; // 获取玩家位置
		dir_x = player_position.x - position.x; // 计算水平方向
		dir_y = player_position.y - position.y; // 计算垂直方向
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // 计算方向向量的长度
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // 归一化水平方向
			double normalized_y = dir_y / len_dir; // 归一化垂直方向
			position.x += static_cast<int>(normalized_x * SPEED); // 更新玩家位置
			position.y += static_cast<int>(normalized_y * SPEED); // 更新玩家位置
		}

	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH - SHADOW_WIDTH) / 2-5; // 阴影位置计算
		int pos_shadow_y = position.y + FRAME_HEIGHT -25; // 阴影位置计算
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // 绘制阴影

		static bool facing_left; // 玩家是否面向左侧
		if (dir_x < 0)
		{
			facing_left = true; // 向左移动
		}
		else if (dir_x > 0)
		{
			facing_left = false; // 向右移动
		}
		if (facing_left)
		{
			anim_left->Play(position.x, position.y, delta); // 播放向左移动动画
		}
		else
		{
			anim_right->Play(position.x, position.y, delta); // 播放向右移动动画
		}
		
	}

private:
	int dir_x;
	int dir_y;
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80; // 敌人动画帧高度
	const int SHADOW_WIDTH = 32; 

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right; // 敌人向右移动动画
	POINT position = { 0,0 }; // 敌人位置
	bool facing_left = false;
};


Animation anim_left_player(_T("./img/player_left_%d.png"), 6, 45); // 玩家向左移动动画
Animation anim_right_player(_T("./img/player_right_%d.png"), 6, 45); // 玩家向左移动动画


void DrawPlayer(int delta,int dir_x)
{
	int pos_shadow_x = player_pos.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2; // 阴影位置计算
	int pos_shadow_y = player_pos.y + PLAYER_HEIGHT-8; // 阴影位置计算
	putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

	static bool facing_left; // 玩家是否面向左侧
	if (dir_x<0)
	{
		facing_left = true; // 向左移动
	}
	else if (dir_x>0)
	{
		facing_left = false; // 向右移动
	}
	if (facing_left)
	{
		anim_left_player.Play(player_pos.x, player_pos.y, delta); // 播放向左移动动画
	}
	else
	{
		anim_right_player.Play(player_pos.x, player_pos.y, delta); // 播放向右移动动画
	}
	
}

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0) // 每隔一定帧数生成一个敌人
	{
		enemy_list.push_back(new Enemy()); // 将新敌人添加到列表中
	}
}

int main()
{
	initgraph(1280, 720);
	
	bool is_move_left = false; // 是否向左移动
	bool is_move_right = false; // 是否向左移动
	bool is_move_up = false; // 是否向左移动
	bool is_move_down = false; // 是否向左移动

	bool running = true;

	ExMessage msg;
	IMAGE img_background;
	std::vector<Enemy*> enemy_list;


	loadimage(&img_background, _T("./img/background.png"), 1280, 720);
	//LoadAnimation();
	loadimage(&img_shadow, _T("./img/shadow_player.png"), 32, 20);

	while (running)
	{
		DWORD start_time = GetTickCount();

		BeginBatchDraw();

		/********************

			消息处理部分

		*********************/


		while (peekmessage(&msg))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.vkcode)
				{
				case VK_LEFT: // 左键切换到左侧动画
					idx_current_anim = 0;
					is_move_left = true; // 向左移动
					break;
				case VK_RIGHT: // 右键切换到右侧动画
					idx_current_anim = 0;
					is_move_right = true; // 向右移动
					break;
				case VK_UP: // 上键向上移动
					is_move_up = true;
					break;
				case VK_DOWN: // 下键向下移动
					is_move_down = true;
					break;
				}
			}
			if (msg.message == WM_KEYUP)
			{
				switch (msg.vkcode)
				{
				case VK_LEFT: // 左键切换到左侧动画
					is_move_left = false; // 向左移动
					break;
				case VK_RIGHT: // 右键切换到右侧动画
					idx_current_anim = 0;
					is_move_right = false; // 向右移动
					break;
				case VK_UP: // 上键向上移动
					is_move_up = false;
					break;
				case VK_DOWN: // 下键向下移动
					is_move_down = false;
					break;
				}
			}
		} 

		/********************

			数据处理部分

		*********************/

		TryGenerateEnemy(enemy_list); // 尝试生成敌人
		for (Enemy* enemy : enemy_list) enemy->Move(player);

		//if (is_move_up) player_pos.y -= PLAYER_SPEED;
		//if (is_move_down) player_pos.y += PLAYER_SPEED;
		//if (is_move_left) player_pos.x -= PLAYER_SPEED;
		//if (is_move_right) player_pos.x += PLAYER_SPEED;

		int dir_x = is_move_right - is_move_left; // 计算水平移动方向
		int dir_y = is_move_down - is_move_up; // 计算垂直移动方向
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // 计算方向向量的长度
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // 归一化水平方向
			double normalized_y = dir_y / len_dir; // 归一化垂直方向
			player_pos.x += static_cast<int>(normalized_x * PLAYER_SPEED); // 更新玩家位置
			player_pos.y += static_cast<int>(normalized_y * PLAYER_SPEED); // 更新玩家位置
		}

		static int frame_count = 0; //帧计数器
		if (++frame_count % 5 == 0) idx_current_anim++;	//每5帧切换一次动画帧

		//循环动画播放
		idx_current_anim %= PLAYER_ANIM_NUM; 

	/********************
	
	      渲染画面部分
	
	*********************/
		

		DWORD end_time = GetTickCount();
		
		DWORD delta_time = end_time - start_time;

		cleardevice();


		putimage(0, 0, &img_background);
		//putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]);
		
		DrawPlayer(delta_time, dir_x);

		for (Enemy* enemy : enemy_list) enemy->Draw(1000 / 144); // 绘制所有敌人

		FlushBatchDraw();

		
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}

	return 0;
		
}