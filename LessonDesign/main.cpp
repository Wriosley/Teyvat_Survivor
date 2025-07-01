#include <graphics.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")  
#pragma comment(lib, "msimg32.lib")  // ���ӿ�
#pragma comment(lib,"MSIMG32.LIB")
#include <string>

using namespace std;


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;


int idx_current_anim = 0; //��ǰ������֡����
const int PLAYER_ANIM_NUM = 6; //��Ҷ���֡��
const int PLAYER_SPEED = 5; // ����ƶ��ٶ�

const int PLAYER_WIDTH = 80; // ��ҿ��
const int PLAYER_HEIGHT = 80; // ��Ҹ߶�
const int SHADOW_WIDTH = 32; // ��Ӱ���

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75; // ��ť�߶�

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;

bool is_game_started = false;
bool running = true;



// ��͸���ȵ�ͼƬ���ƺ���
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
			IMAGE* frame = new IMAGE(); // �����µ�IMAGE����
			loadimage(frame, path_file);
			frame_list.push_back(frame); // ��ÿһ֡��ͼƬ���ص��б���
		}
	}
	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i]; // �ͷ�ÿһ֡��ͼƬ��Դ
		}
	}
public:
	std::vector<IMAGE*> frame_list; // �洢����֡
};

Atlas* atlas_player_left;
Atlas* atlas_player_right; // ��������ƶ�������Դ
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right; // ���������ƶ�������Դ

class Button
{
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect; // ���ð�ť����

		loadimage(&img_idle, path_img_idle); // ���ؿ���״̬ͼƬ
		loadimage(&img_hovered, path_img_hovered); // ������ͣ״̬ͼƬ
		loadimage(&img_pushed, path_img_pushed); // ������ͣ״̬ͼƬ
	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y)) // ��������ͣ�ڰ�ť��
			{
				status = Status::Hovered; // �л�����ͣ״̬
			}
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y)) // �������뿪��ť����
			{
				status = Status::Idle; // �л��ؿ���״̬
			}
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y)) // ���������ڰ�ť����
			{
				status = Status::Pushed; // �л�������״̬
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed) // �������ڰ�ť�������ͷ�
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
		case Status::Idle: // ����״̬
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered: // ��ͣ״̬
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed: // ����״̬
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;// ��ť����¼�������

private:
	enum class Status
	{
		Idle = 0, // ����״̬
		Hovered,
		Pushed
	};

	

private:
	RECT region;
	IMAGE img_idle; // ����״̬ͼƬ
	IMAGE img_hovered; // ��ͣ״̬ͼƬ
	IMAGE img_pushed; // ����״̬ͼƬ
	Status status = Status::Idle; // ��ť��ǰ״̬

private:
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right &&
			y >= region.top && y <= region.bottom; // �������Ƿ��ڰ�ť������
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
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL); // ���ſ�ʼ��Ϸ��Ч
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
		anim_atlas = atlas; // ���ö�����Դ
		interval_ms = interval; // ����֡���ʱ��
		
	}

	~Animation() = default;

	void Play(int x, int y, int delta)
	{
		timer += delta + 3;
		if (timer >= interval_ms) // �����ʱ������֡���ʱ��
		{
			timer = 0; // ���ü�ʱ��
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size(); // �л�����һ֡
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]); // ���Ƶ�ǰ֡
	}

private:
	int interval_ms = 0; // ֡���ʱ��
	int idx_frame = 0; // ��ǰ֡����
	int timer = 0; // ��ʱ�������ڿ��ƶ��������ٶ�

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
		fillcircle(position.x, position.y, RADIUS); // �����ӵ�
	}
private:
	const int  RADIUS = 10; // �ӵ��뾶
};
class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("./img/shadow_player.png"),32,20); // ���ص�����ӰͼƬ
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
			case VK_LEFT: // ����л�����ද��
				idx_current_anim = 0;
				is_move_left = true; // �����ƶ�
				break;
			case VK_RIGHT: // �Ҽ��л����Ҳද��
				idx_current_anim = 0;
				is_move_right = true; // �����ƶ�
				break;
			case VK_UP: // �ϼ������ƶ�
				is_move_up = true;
				break;
			case VK_DOWN: // �¼������ƶ�
				is_move_down = true;
				break;
			}
		}
		if (msg.message == WM_KEYUP)
		{
			switch (msg.vkcode)
			{
			case VK_LEFT: // ����л�����ද��
				is_move_left = false; // �����ƶ�
				break;
			case VK_RIGHT: // �Ҽ��л����Ҳද��
				idx_current_anim = 0;
				is_move_right = false; // �����ƶ�
				break;
			case VK_UP: // �ϼ������ƶ�
				is_move_up = false;
				break;
			case VK_DOWN: // �¼������ƶ�
				is_move_down = false;
				break;
			}
		}
	}

	void Move()
	{
		int dir_x = is_move_right - is_move_left; // ����ˮƽ�ƶ�����
		int dir_y = is_move_down - is_move_up; // ���㴹ֱ�ƶ�����
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // ���㷽�������ĳ���
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // ��һ��ˮƽ����
			double normalized_y = dir_y / len_dir; // ��һ����ֱ����
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

		int pos_shadow_x = position.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2; // ��Ӱλ�ü���
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8; // ��Ӱλ�ü���
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // ������Ӱ


		static bool facing_left = false; // ����Ƿ��������
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
		{
			facing_left = true; // �����ƶ�
		}
		else if (dir_x > 0)
		{
			facing_left = false; // �����ƶ�
		}
		if (facing_left)
		{
			anim_left->Play(position.x, position.y, delta); // ���������ƶ�����
		}
		else
		{
			anim_right->Play(position.x, position.y, delta); // ���������ƶ�����
		}

		//putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // ������Ӱ
	}

private:
	const int SPEED = 5; // ����ƶ��ٶ�

	const int FRAME_WIDTH = 80; // ��ҿ��
	const int FRAME_HEIGHT = 80; // ��Ҹ߶�
	const int SHADOW_WIDTH = 32; // ��Ӱ���

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
		loadimage(&img_shadow, _T("./img/shadow_enemy.png")); // ���ص�����ӰͼƬ
		anim_left = new Animation(atlas_enemy_left, 45); // ���������ƶ�����
		anim_right = new Animation(atlas_enemy_right, 45); // ���������ƶ�����

		enum class SpawnEdge
		{
			Up = 0, Down, Left, Right
		};

		SpawnEdge edge = static_cast<SpawnEdge>(rand() % 4); // ������ɵ��˳�����Ե
		switch (edge)
		{
		case SpawnEdge::Up: // �ϱ�Ե
			position.x = rand() % WINDOW_WIDTH; // �������x����
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH; // �������x����
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left: // ���Ե
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT; // �������y����
			break;
		case SpawnEdge::Right: // �ұ�Ե
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT; // �������y����
			break;
		default:
			break;
		}



	}

	~Enemy()
	{
		delete anim_left; // �ͷ������ƶ�������Դ
		delete anim_right; // �ͷ������ƶ�������Դ
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
		const POINT& player_position = player.position; // ��ȡ���λ��
		dir_x = player_position.x - position.x; // ����ˮƽ����
		dir_y = player_position.y - position.y; // ���㴹ֱ����
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // ���㷽�������ĳ���
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // ��һ��ˮƽ����
			double normalized_y = dir_y / len_dir; // ��һ����ֱ����
			position.x += static_cast<int>(normalized_x * SPEED); // �������λ��
			position.y += static_cast<int>(normalized_y * SPEED); // �������λ��
		}

	}

	void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH - SHADOW_WIDTH) / 2 - 5; // ��Ӱλ�ü���
		int pos_shadow_y = position.y + FRAME_HEIGHT - 25; // ��Ӱλ�ü���
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow); // ������Ӱ

		static bool facing_left; // ����Ƿ��������
		if (dir_x < 0)
		{
			facing_left = true; // �����ƶ�
		}
		else if (dir_x > 0)
		{
			facing_left = false; // �����ƶ�
		}
		if (facing_left)
		{
			anim_left->Play(position.x, position.y, delta); // ���������ƶ�����
		}
		else
		{
			anim_right->Play(position.x, position.y, delta); // ���������ƶ�����
		}

	}

private:
	int dir_x;
	int dir_y;
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80; // ���˶���֡�߶�
	const int SHADOW_WIDTH = 32;

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right; // ���������ƶ�����
	POINT position = { 0,0 }; // ����λ��
	bool facing_left = false;
};

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0) // ÿ��һ��֡������һ������
	{
		enemy_list.push_back(new Enemy()); // ���µ�����ӵ��б���
	}
}

int main()
{

	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	atlas_enemy_left = new Atlas(_T("./img/enemy_left_%d.png"), 6); // ���ص��������ƶ�������Դ
	atlas_enemy_right = new Atlas(_T("./img/enemy_right_%d.png"), 6); // ���ص��������ƶ�������Դ
	atlas_player_left = new Atlas(_T("./img/player_left_%d.png"), PLAYER_ANIM_NUM); // ������������ƶ�������Դ
	atlas_player_right = new Atlas(_T("./img/player_right_%d.png"), PLAYER_ANIM_NUM); // ������������ƶ�������Դ

	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL); // �򿪱�������
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL); // �򿪱�������

	

	Player player;
	ExMessage msg;
	IMAGE img_background;
	IMAGE img_menu; // �˵�ͼƬ
	std::vector<Enemy*> enemy_list;

	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2; // ��ʼ��Ϸ��ť����
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430; // ��ʼ��Ϸ��ť����
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;
	
	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2; // ��ʼ��Ϸ��ť����
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550; // ��ʼ��Ϸ��ť����
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

		�¼�����

	*************************************************************************/
		while (peekmessage(&msg))
		{
			if(is_game_started)
				player.ProcessEvent(msg); // ��������¼�
			else {
				btn_start_game.ProcessEvent(msg); // ����ʼ��Ϸ��ť�¼�
				btn_quit_game.ProcessEvent(msg); // �����˳���Ϸ��ť�¼�
			}

		}
	/************************************************************************

		���ݴ���

	*************************************************************************/
		if(is_game_started)
		{
			TryGenerateEnemy(enemy_list); // �������ɵ���
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

		��Ⱦ���沿��
		-�����Ļ�����Ʊ��������
		-�������е���
		-ˢ����Ļ
		-���֡�ʹ��ͣ���ȴ�һ��ʱ���Ա����ȶ���֡��

	*************************************************************************/

		cleardevice();

		if(is_game_started)
		{
			putimage(0, 0, &img_background);

			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list) enemy->Draw(1000 / 144); // �������е���
		}
		else
		{
			putimage(0, 0, &img_menu); // ���Ʋ˵�����
			btn_start_game.Draw(); // ���ƿ�ʼ��Ϸ��ť
			btn_quit_game.Draw(); // �����˳���Ϸ��ť
		}



		FlushBatchDraw();


	}

	delete atlas_enemy_left;
	delete atlas_enemy_right; // �ͷŵ��������ƶ�������Դ
	delete atlas_player_left; // �ͷ���������ƶ�������Դ
	delete atlas_player_right; // �ͷ���������ƶ�������Դ


	EndBatchDraw();
	return 0;

}