#include <graphics.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")  
#pragma comment(lib, "msimg32.lib")  // 链接库
#pragma comment(lib,"MSIMG32.LIB")
#include <string>

using namespace std;


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;


int idx_current_anim = 0; //当前动画的帧索引
const int PLAYER_ANIM_NUM = 6; //玩家动画帧数
const int PLAYER_SPEED = 5; // 玩家移动速度

const int PLAYER_WIDTH = 80; // 玩家宽度
const int PLAYER_HEIGHT = 80; // 玩家高度
const int SHADOW_WIDTH = 32; // 阴影宽度

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75; // 按钮高度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;

bool is_game_started = false;
bool running = true;



// 带透明度的图片绘制函数
inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}



class Atlas
{
public:
	Atlas(LPCTSTR path, int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			swprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE(); // 创建新的IMAGE对象
			loadimage(frame, path_file);
			frame_list.push_back(frame); // 将每一帧的图片加载到列表中
		}
	}
	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i]; // 释放每一帧的图片资源
		}
	}
public:
	std::vector<IMAGE*> frame_list; // 存储动画帧
};

Atlas* atlas_player_left;
Atlas* atlas_player_right; // 玩家向右移动动画资源
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right; // 敌人向右移动动画资源

class Button
{
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect; // 设置按钮区域

		loadimage(&img_idle, path_img_idle); // 加载空闲状态图片
		loadimage(&img_hovered, path_img_hovered); // 加载悬停状态图片
		loadimage(&img_pushed, path_img_pushed); // 加载悬停状态图片
	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y)) // 如果鼠标悬停在按钮上
			{
				status = Status::Hovered; // 切换到悬停状态
			}
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y)) // 如果鼠标离开按钮区域
			{
				status = Status::Idle; // 切换回空闲状态
			}
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y)) // 如果鼠标点击在按钮区域
			{
				status = Status::Pushed; // 切换到按下状态
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed) // 如果鼠标在按钮区域内释放
				OnClick();
			break;
		default:
			break;	
		}
	}

	void Draw()
	{
		switch (status)
		{
		case Status::Idle: // 空闲状态
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered: // 悬停状态
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed: // 按下状态
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;// 按钮点击事件处理函数

private:
	enum class Status
	{
		Idle = 0, // 空闲状态
		Hovered,
		Pushed
	};

	

private:
	RECT region;
	IMAGE img_idle; // 空闲状态图片
	IMAGE img_hovered; // 悬停状态图片
	IMAGE img_pushed; // 按下状态图片
	Status status = Status::Idle; // 按钮当前状态

private:
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right &&
			y >= region.top && y <= region.bottom; // 检查鼠标是否在按钮区域内
	}
};

class StartGameButton :public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		: Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{
	}
	~StartGameButton() = default;

protected:
	void OnClick() override
	{
		is_game_started = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL); // 播放开始游戏音效
	}
};

class QuitGameButton :public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		: Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{
	}
	~QuitGameButton() = default;

protected:
	void OnClick() override
	{
		running = false;
	}
};


class Animation
{
public:
	Animation(Atlas* atlas, int interval)
	{
		anim_atlas = atlas; // 设置动画资源
		interval_ms = interval; // 设置帧间隔时间
		
	}

	~Animation() = default;

	void Play(int x, int y, int delta)
	{
		timer += delta + 3;
		if (timer >= interval_ms) // 如果计时器超过帧间隔时间
		{
			timer = 0; // 重置计时器
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size(); // 切换到下一帧
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]); // 绘制当前帧
	}

private:
	int interval_ms = 0; // 帧间隔时间
	int idx_frame = 0; // 当前帧索引
	int timer = 0; // 计时器，用于控制动画播放速度

private:
	Atlas* anim_atlas;
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
		fillcircle(position.x, position.y, RADIUS); // 绘制子弹
	}
private:
	const int  RADIUS = 10; // 子弹半径
};
class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("./img/shadow_player.png"),32,20); // 加载敌人阴影图片
		anim_left = new Animation(atlas_player_left,45);
		anim_right = new Animation(atlas_player_right, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;

	}

	POINT position = { 500,500 };

	void ProcessEvent(const ExMessage& msg)
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

	void Move()
	{
		int dir_x = is_move_right - is_move_left; // 计算水平移动方向
		int dir_y = is_move_down - is_move_up; // 计算垂直移动方向
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // 计算方向向量的长度
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // 归一化水平方向
			double normalized_y = dir_y / len_dir; // 归一化垂直方向
			double update_x = position.x + static_cast<int>(normalized_x * PLAYER_SPEED);
			double update_y = position.y + static_cast<int>(normalized_y * PLAYER_SPEED);
			if (update_x > 0 && update_x < WINDOW_WIDTH - PLAYER_WIDTH)
				position.x = update_x;
			if (update_y > 0 && update_y < WINDOW_HEIGHT - PLAYER_HEIGHT)
				position.y = update_y;
		}

	}

	void Draw(int delta)
	{

		int pos_shadow_x = position.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2; // 阴影位置计算
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8; // 阴影位置计算
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // 绘制阴影


		static bool facing_left = false; // 玩家是否面向左侧
		int dir_x = is_move_right - is_move_left;
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

		//putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // 绘制阴影
	}

private:
	const int SPEED = 5; // 玩家移动速度

	const int FRAME_WIDTH = 80; // 玩家宽度
	const int FRAME_HEIGHT = 80; // 玩家高度
	const int SHADOW_WIDTH = 32; // 阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;


};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("./img/shadow_enemy.png")); // 加载敌人阴影图片
		anim_left = new Animation(atlas_enemy_left, 45); // 敌人向左移动动画
		anim_right = new Animation(atlas_enemy_right, 45); // 敌人向右移动动画

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
		const POINT& player_position = player.position; // 获取玩家位置
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
		int pos_shadow_x = position.x + (FRAME_WIDTH - SHADOW_WIDTH) / 2 - 5; // 阴影位置计算
		int pos_shadow_y = position.y + FRAME_HEIGHT - 25; // 阴影位置计算
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

	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	atlas_enemy_left = new Atlas(_T("./img/enemy_left_%d.png"), 6); // 加载敌人向左移动动画资源
	atlas_enemy_right = new Atlas(_T("./img/enemy_right_%d.png"), 6); // 加载敌人向右移动动画资源
	atlas_player_left = new Atlas(_T("./img/player_left_%d.png"), PLAYER_ANIM_NUM); // 加载玩家向左移动动画资源
	atlas_player_right = new Atlas(_T("./img/player_right_%d.png"), PLAYER_ANIM_NUM); // 加载玩家向右移动动画资源

	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL); // 打开背景音乐
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL); // 打开背景音乐

	

	Player player;
	ExMessage msg;
	IMAGE img_background;
	IMAGE img_menu; // 菜单图片
	std::vector<Enemy*> enemy_list;

	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2; // 开始游戏按钮区域
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430; // 开始游戏按钮区域
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;
	
	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2; // 开始游戏按钮区域
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550; // 开始游戏按钮区域
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game(region_btn_start_game, _T("./img/ui_start_idle.png"), _T("./img/ui_start_hovered.png"), _T("./img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game(region_btn_quit_game, _T("./img/ui_quit_idle.png"), _T("./img/ui_quit_hovered.png"), _T("./img/ui_quit_pushed.png"));

	loadimage(&img_menu, _T("img/menu.png"));
	loadimage(&img_background, _T("./img/background.png"), 1280, 720);

	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();
	/************************************************************************

		事件处理

	*************************************************************************/
		while (peekmessage(&msg))
		{
			if(is_game_started)
				player.ProcessEvent(msg); // 处理玩家事件
			else {
				btn_start_game.ProcessEvent(msg); // 处理开始游戏按钮事件
				btn_quit_game.ProcessEvent(msg); // 处理退出游戏按钮事件
			}

		}
	/************************************************************************

		数据处理

	*************************************************************************/
		if(is_game_started)
		{
			TryGenerateEnemy(enemy_list); // 尝试生成敌人
			for (Enemy* enemy : enemy_list) enemy->Move(player);

			player.Move();

			DWORD end_time = GetTickCount();

			DWORD delta_time = end_time - start_time;

			if (delta_time < 1000 / 144)
			{
				Sleep(1000 / 144 - delta_time);
			}
		}

	/************************************************************************

		渲染画面部分
		-清除屏幕，绘制背景和玩家
		-绘制所有敌人
		-刷新屏幕
		-如果帧率过低，则等待一段时间以保持稳定的帧率

	*************************************************************************/

		cleardevice();

		if(is_game_started)
		{
			putimage(0, 0, &img_background);

			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list) enemy->Draw(1000 / 144); // 绘制所有敌人
		}
		else
		{
			putimage(0, 0, &img_menu); // 绘制菜单背景
			btn_start_game.Draw(); // 绘制开始游戏按钮
			btn_quit_game.Draw(); // 绘制退出游戏按钮
		}



		FlushBatchDraw();


	}

	delete atlas_enemy_left;
	delete atlas_enemy_right; // 释放敌人向右移动动画资源
	delete atlas_player_left; // 释放玩家向左移动动画资源
	delete atlas_player_right; // 释放玩家向右移动动画资源


	EndBatchDraw();
	return 0;

}