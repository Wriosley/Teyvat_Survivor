#include <graphics.h>
#include <iostream>
#include <Windows.h>
using namespace std;

char board_data[3][3] =
{
	{'-','-','-'},
	{'-','-','-'},
	{'-','-','-'}
};
char current_piece = 'O';


//检测指定棋子玩家是否获胜
bool CheckWin(char c)
{//检查行
	for (int i = 0; i < 3; i++)
	{
		if (board_data[i][0] == c && board_data[i][1] == c && board_data[i][2] == c)
			return true;
	}
	//检查列
	for (int i = 0; i < 3; i++)
	{
		if (board_data[0][i] == c && board_data[1][i] == c && board_data[2][i] == c)
			return true;
	}
	//检查对角线
	if (board_data[0][0] == c && board_data[1][1] == c && board_data[2][2] == c)
		return true;
	if (board_data[0][2] == c && board_data[1][1] == c && board_data[2][0] == c)
		return true;
	return false;

}

//检测当前是否出现平局
bool CheckDraw()
{
	for (int i = 0;i < 3;i++)
		for (int j = 0;i < 3;i++)
		{
			if (board_data[i][j] == '-') return false;
		}
	return true;
}

//定义棋盘网格
void DrawBoard()
{
	setcolor(GREEN);
	for (int i = 0; i < 4; i++)
	{
		line(0, i * 200, 600, i * 200); //水平线
		line(i * 200, 0, i * 200, 600); //垂直线
	}
}

//绘制棋子
void DrawPiece()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (board_data[i][j] == 'X')
			{
				setcolor(WHITE);
				line(j * 200 + 20, i * 200 + 20, j * 200 + 180, i * 200 + 180);
				line(j * 200 + 180, i * 200 + 20, j * 200 + 20, i * 200 + 180);
			}
			else if (board_data[i][j] == 'O')
			{
				setcolor(WHITE);
				circle(j * 200 + 100, i * 200 + 100, 80);
			}
		}
	}
}

//绘制提示信息
void DrawTipText()
{
	static TCHAR tip_text[100];
	_stprintf_s(tip_text, _T("当前棋子类型: %c"), current_piece);
	settextcolor(RGB(225, 175, 45));
	outtextxy(0, 0, tip_text); //在窗口左上角显示当前棋子类型
}

int BoardGame()
{
	initgraph(600, 600);

	bool running = true;
	ExMessage msg;

	BeginBatchDraw();

	while (running)
	{

		while (peekmessage(&msg))
		{
			//检查鼠标左键按下消息
			if (msg.message == WM_LBUTTONDOWN)
			{
				cout << "鼠标左键按下" << endl;
				//计算点击位置
				int x = msg.x;
				int y = msg.y;
				int index_x = x / 200;
				int index_y = y / 200;

				//尝试落子
				if (board_data[index_y][index_x] == '-')
				{
					board_data[index_y][index_x] = current_piece;
					//切换棋子类型
					if (current_piece == 'O')
						current_piece = 'X';
					else
						current_piece = 'O';

				}
			}

		}

		if (CheckWin('X'))
		{
			MessageBox(GetHWnd(), _T("X 玩家获胜"), _T("游戏结束"), MB_OK);
			running = false;
		}
		else if (CheckWin('O'))
		{
			MessageBox(GetHWnd(), _T("O 玩家获胜"), _T("游戏结束"), MB_OK);
			running = false;
		}
		else if (CheckDraw())
		{
			MessageBox(GetHWnd(), _T("平局！"), _T("游戏结束"), MB_OK);
			running = false;
		}




		cleardevice();

		DrawBoard();
		DrawPiece();



		FlushBatchDraw();
		DrawTipText();
	}

	return 0;
}