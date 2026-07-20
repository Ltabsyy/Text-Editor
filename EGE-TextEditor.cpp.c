#include <stdio.h>
#include <graphics.h>
#include <ege/sys_edit.h>
#include <commdlg.h>

#define MaxFileSize 1048576
int height = 32;

void DrawButton(int x, int y, int w, int h, int highlight, const char* text)
{
	ege_point polyPoints1[3] =
	{
		{(float)x, (float)y}, {(float)x+w, (float)y}, {(float)x, (float)y+h}
	};
	ege_point polyPoints2[3] =
	{
		{(float)x+w, (float)y}, {(float)x, (float)y+h}, {(float)x+w, (float)y+h}
	};
	setfillcolor(WHITE);
	ege_fillpoly(3, polyPoints1);
	setfillcolor(GRAY);
	ege_fillpoly(3, polyPoints2);
	setfillcolor(highlight ? LIGHTBLUE : LIGHTGRAY);
	ege_fillrect(x+w*2.0/32, y+h*2.0/32, w*28/32, h*28/32);
	setcolor(BLACK);
	setfont(height/2, 0, "黑体");
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	outtextrect(x+w*2.0/32, y+h*2.0/32, w*28/32, h*28/32, text);
	settextjustify(LEFT_TEXT, TOP_TEXT);
}

int main()
{
	int choice, suspend = -1;
	mouse_msg mouseMsg;
	setcaption("untitled - Text Editor");
	SetProcessDPIAware();//避免Windows缩放造成模糊
	initgraph(1280, 720, INIT_RENDERMANUAL);
	setbkcolor(WHITE);
	setbkmode(TRANSPARENT);//默认设置为无背景字体
	sys_edit editBox;
	editBox.create(1);//多行文本框
	editBox.move(0, height);//位置
	editBox.size(1280, 720-height);//大小
	editBox.setfont(height*3/4, 0, "Consolas");
	editBox.setmaxlen(MaxFileSize);//最大输入长度
	editBox.visible(1);//默认不可见，设为可见
	while(1)
	{
		DrawButton(height*0, 0, height*2, height, suspend == 0, "新建");
		DrawButton(height*2, 0, height*2, height, suspend == 1, "打开");
		DrawButton(height*4, 0, height*2, height, suspend == 2, "保存");
		DrawButton(height*6, 0, height*2, height, suspend == 3, "关闭");
		choice = -1;
		while(mousemsg())
		{
			mouseMsg = getmouse();
			if(mouseMsg.y < height && mouseMsg.x < height*8)
			{
				suspend = mouseMsg.x / (height*2);
				if(mouseMsg.is_up()) choice = suspend;
			}
			else
			{
				suspend = -1;
			}
		}
		if(choice == 0)//新建
		{
			editBox.settext("");
			setcaption("untitled - Text Editor");
		}
		else if(choice == 1 || choice == 2)//打开或保存
		{
			OPENFILENAME ofn = {0};
			char fileName[260] = {0};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL; //父窗口句柄
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = sizeof(fileName);
			ofn.lpstrFilter = "All Files (*.*)\0*.*\0";//设置文件过滤器
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; //标志位
			if(choice == 1)//打开
			{
				if(GetOpenFileName(&ofn))//文件打开对话框
				{
					//开始读取file路径文件
					FILE* file = fopen(fileName, "r");
					char content[MaxFileSize] = {0};
					char ch;
					int i = 0;
					while(1)
					{
						ch = fgetc(file);
						if(ch == EOF) break;
						if(ch == '\n')//将\n转为\r\n适配sys_edit
						{
							content[i] = '\r';
							i++;
						}
						content[i] = ch;
						i++;
						if(i == MaxFileSize)
						{
							content[i-1] = 0;
							break;
						}
					}
					fclose(file);
					editBox.settext(content);
					//设置程序标题
					char title[300] = {0};
					sprintf(title, "%s - Text Editor", fileName);
					setcaption(title);
				}
				//editBox.settext("content\r\n123456\n7\r89");
			}
			else if(choice == 2)//保存
			{
				if(GetSaveFileName(&ofn))//文件另存为对话框
				{
					//开始写入file路径文件
					FILE* file = fopen(fileName, "w");
					char content[MaxFileSize] = {0};
					editBox.gettext(MaxFileSize, content);
					fputs(content, file);
					fclose(file);
				}
			}
		}
		else if(choice == 3)//关闭
		{
			break;
		}
		delay_ms(100);
	}
	closegraph();
	return 0;
}
