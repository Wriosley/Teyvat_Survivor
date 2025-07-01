#include <graphics.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#pragma comment(lib, "msimg32.lib")  // ���ӿ�
#include <string>
using namespace std;

int idx_current_anim = 0; //��ǰ������֡����
const int PLAYER_ANIM_NUM = 6; //��Ҷ���֡��
const int PLAYER_SPEED = 5; // ����ƶ��ٶ�

const int PLAYER_WIDTH = 80; // ��ҿ��
const int PLAYER_HEIGHT = 80; // ��Ҹ߶�
const int SHADOW_WIDTH = 32; // ��Ӱ���

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_shadow;



POINT player_pos = { 500, 500 }; // ��ҳ�ʼλ��

// ��͸���ȵ�ͼƬ���ƺ���
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
		interval_ms = interval;// ֡���ʱ��

		TCHAR path_file[256];
		for (size_t i = 0;i < num;i++)
		{
			swprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE(); // �����µ�IMAGE����
			loadimage(frame, path_file);
			frame_list.push_back(frame); // ��ÿһ֡��ͼƬ���ص��б���
		}
	}

	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i]; // �ͷ�ÿһ֡��ͼƬ��Դ
		}
	}

	void Play(int x,int y,int delta)
	{
		timer += delta+3;
		//cout << "timer��" << timer << endl; // �����ʱ��ֵ
		if (timer >= interval_ms) // �����ʱ������֡���ʱ��
		{
			timer = 0; // ���ü�ʱ��
			idx_frame = (idx_frame + 1) % frame_list.size(); // �л�����һ֡
			//cout << "idx_frame: " << idx_frame << endl; // �����ǰ֡����
		}

		putimage_alpha(x, y, frame_list[idx_frame]); // ���Ƶ�ǰ֡
	}
	
private:
	int interval_ms = 0; // ֡���ʱ��
	int idx_frame = 0; // ��ǰ֡����
	int timer = 0; // ��ʱ�������ڿ��ƶ��������ٶ�
	vector<IMAGE*> frame_list; // �洢����֡>
};

class Bullet
{
public:

};


Animation anim_left_player(_T("./img/player_left_%d.png"), 6, 45); // ��������ƶ�����
Animation anim_right_player(_T("./img/player_right_%d.png"), 6, 45); // ��������ƶ�����


void DrawPlayer(int delta,int dir_x)
{
	int pos_shadow_x = player_pos.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2; // ��Ӱλ�ü���
	int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8; // ��Ӱλ�ü���

	static bool facing_left; // ����Ƿ��������
	if (dir_x<0)
	{
		facing_left = true; // �����ƶ�
	}
	else if (dir_x>0)
	{
		facing_left = false; // �����ƶ�
	}
	if (facing_left)
	{
		anim_left_player.Play(player_pos.x, player_pos.y, delta); // ���������ƶ�����
	}
	else
	{
		anim_right_player.Play(player_pos.x, player_pos.y, delta); // ���������ƶ�����
	}
	putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
}

int main()
{
	initgraph(1280, 720);
	
	bool is_move_left = false; // �Ƿ������ƶ�
	bool is_move_right = false; // �Ƿ������ƶ�
	bool is_move_up = false; // �Ƿ������ƶ�
	bool is_move_down = false; // �Ƿ������ƶ�

	bool running = true;

	ExMessage msg;
	IMAGE img_background;

	loadimage(&img_background, _T("./img/background.png"), 1280, 720);
	//LoadAnimation();
	loadimage(&img_shadow, _T("./img/shadow_player.png"), 32, 20);

	while (running)
	{
		DWORD start_time = GetTickCount();

		BeginBatchDraw();
		while (peekmessage(&msg))
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

		//if (is_move_up) player_pos.y -= PLAYER_SPEED;
		//if (is_move_down) player_pos.y += PLAYER_SPEED;
		//if (is_move_left) player_pos.x -= PLAYER_SPEED;
		//if (is_move_right) player_pos.x += PLAYER_SPEED;

		int dir_x = is_move_right - is_move_left; // ����ˮƽ�ƶ�����
		int dir_y = is_move_down - is_move_up; // ���㴹ֱ�ƶ�����
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y); // ���㷽�������ĳ���
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir; // ��һ��ˮƽ����
			double normalized_y = dir_y / len_dir; // ��һ����ֱ����
			player_pos.x += static_cast<int>(normalized_x * PLAYER_SPEED); // �������λ��
			player_pos.y += static_cast<int>(normalized_y * PLAYER_SPEED); // �������λ��
		}

		static int frame_count = 0; //֡������
		if (++frame_count % 5 == 0) idx_current_anim++;	//ÿ5֡�л�һ�ζ���֡

		//ѭ����������
		idx_current_anim %= PLAYER_ANIM_NUM; 

	
		DWORD end_time = GetTickCount();
		
		DWORD delta_time = end_time - start_time;

		cleardevice();


		putimage(0, 0, &img_background);
		//putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]);
		
		DrawPlayer(delta_time, dir_x);


		FlushBatchDraw();

		
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}

	return 0;
		
}