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


//���ָ����������Ƿ��ʤ
bool CheckWin(char c)
{//�����
	for (int i = 0; i < 3; i++)
	{
		if (board_data[i][0] == c && board_data[i][1] == c && board_data[i][2] == c)
			return true;
	}
	//�����
	for (int i = 0; i < 3; i++)
	{
		if (board_data[0][i] == c && board_data[1][i] == c && board_data[2][i] == c)
			return true;
	}
	//���Խ���
	if (board_data[0][0] == c && board_data[1][1] == c && board_data[2][2] == c)
		return true;
	if (board_data[0][2] == c && board_data[1][1] == c && board_data[2][0] == c)
		return true;
	return false;

}

//��⵱ǰ�Ƿ����ƽ��
bool CheckDraw()
{
	for (int i = 0;i < 3;i++)
		for (int j = 0;i < 3;i++)
		{
			if (board_data[i][j] == '-') return false;
		}
	return true;
}

//������������
void DrawBoard()
{
	setcolor(GREEN);
	for (int i = 0; i < 4; i++)
	{
		line(0, i * 200, 600, i * 200); //ˮƽ��
		line(i * 200, 0, i * 200, 600); //��ֱ��
	}
}

//��������
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

//������ʾ��Ϣ
void DrawTipText()
{
	static TCHAR tip_text[100];
	_stprintf_s(tip_text, _T("��ǰ��������: %c"), current_piece);
	settextcolor(RGB(225, 175, 45));
	outtextxy(0, 0, tip_text); //�ڴ������Ͻ���ʾ��ǰ��������
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
			//���������������Ϣ
			if (msg.message == WM_LBUTTONDOWN)
			{
				cout << "����������" << endl;
				//������λ��
				int x = msg.x;
				int y = msg.y;
				int index_x = x / 200;
				int index_y = y / 200;

				//��������
				if (board_data[index_y][index_x] == '-')
				{
					board_data[index_y][index_x] = current_piece;
					//�л���������
					if (current_piece == 'O')
						current_piece = 'X';
					else
						current_piece = 'O';

				}
			}

		}

		if (CheckWin('X'))
		{
			MessageBox(GetHWnd(), _T("X ��һ�ʤ"), _T("��Ϸ����"), MB_OK);
			running = false;
		}
		else if (CheckWin('O'))
		{
			MessageBox(GetHWnd(), _T("O ��һ�ʤ"), _T("��Ϸ����"), MB_OK);
			running = false;
		}
		else if (CheckDraw())
		{
			MessageBox(GetHWnd(), _T("ƽ�֣�"), _T("��Ϸ����"), MB_OK);
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