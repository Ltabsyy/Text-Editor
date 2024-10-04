#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <conio.h>
#include <windows.h>
//#include <commdlg.h>//链接参数-lcomdlg32

#define MaxFileName 128

char* fileName;
char** content;
int** type;
int numberOfRow;
int* numberOfColumn;
int cursorR = 0, cursorC = 0;

enum Type {
	Type_Default = 0,//编辑器默认
	Type_Gutter = 1,//行号
	Type_Gutter_AL = 2,//当前行号
	Type_Comment = 3,//注释
	Type_Symbol = 4,//符号
	Type_Bracket_L1 = 5,//1级括号
	Type_Bracket_L2 = 6,//2级括号
	Type_Bracket_L3 = 7,//3级括号
	Type_Bracket_L4 = 8,//4级括号
	Type_Number = 9,//数字
	Type_Preprocessor = 10,//预处理指令
	Type_Character = 11,//字符
	Type_String = 12,//字符串
	Type_EscapeSequences = 13,//转义序列
	Type_ReservedWord = 14,//关键字
	Type_ReservedWord_T = 15,//类型关键字
	Type_Function = 16,//函数
	Type_Variable = 17,//变量
	Type_Variable_G = 18,//全局变量/类
	Type_Variable_L = 19//局部变量
};

int Color[20] = {
	//MoLo Console Minus
	0x07,//编辑器默认
	0x08,//行号
	0x0b,//当前行号
	0x02,//注释
	0x0c,//符号
	0x0c,//1级括号
	0x0e,//2级括号
	0x0d,//3级括号
	0x09,//4级括号
	0x0f,//数字
	0x0d,//预处理指令
	0x0a,//字符
	0x06,//字符串
	0x0e,//转义序列0x06
	0x05,//关键字
	0x05,//类型关键字0x01
	0x0e,//函数
	0x0b,//变量
	0x0b,//全局变量/类0x01
	0x03//局部变量
	//MoLo Console Minus Minus
	//0x07, 0x07, 0x0f, 0x02, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x09, 0x0d, 0x09, 0x09, 0x09, 0x0d, 0x0d, 0x0e, 0x0b, 0x0b, 0x0b
	//MoLo Pencil Console
	//0xf0, 0xf0, 0xf1, 0xf8, 0xf4, 0xf1, 0xf4, 0xf1, 0xf4, 0xf1, 0xf0, 0xf1, 0xf1, 0xf4, 0xf5, 0xf5, 0xf0, 0xf0, 0xf0, 0xf0
};

void gotoxy(short int x, short int y)//光标定位
{
	COORD coord = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void clrscr()//清空屏幕
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);//获取标准输出设备的屏幕缓冲区属性
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y, num = 0;//定义双字节变量
	COORD pos = {0, 0};
	//把窗口缓冲区全部填充为空格并填充为默认颜色
	FillConsoleOutputCharacter(hdout, ' ', size, pos, &num);
	FillConsoleOutputAttribute(hdout, csbi.wAttributes, size, pos, &num);
	SetConsoleCursorPosition(hdout, pos);//光标定位到窗口左上角
}
void setbgcolor(int color)//设置背景颜色
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y, num = 0;
	COORD pos = {0, 0};
	FillConsoleOutputAttribute(hdout, color, size, pos, &num);
	SetConsoleTextAttribute(hdout, color);
}
void ColorChar(char c, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	putchar(c);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color[Type_Default]);
}
void ColorNumber(int number, int color)//输出彩色数字
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%d", number);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color[Type_Default]);
}
void SetConsoleMouseMode(int mode)//键鼠操作切换
{
	if(mode == 1)//切换到鼠标
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
	}
	else if(mode == 0)//切换到键盘
	{
		//SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
		//system("pause");//system指令使SetConsoleMode失效
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT
			| ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS
			| ENABLE_AUTO_POSITION);
	}
}

char* InputFileName()
{
	char temp[MaxFileName];
	int i, j;
	char* fileName =(char*) calloc(MaxFileName, sizeof(char));
	printf("[File]>");
	fgets(temp, MaxFileName, stdin);//哼，回车
	j = 0;
	for(i=0; i<MaxFileName; i++)
	{
		if(temp[i] == '\n') break;
		else if(temp[i] == '"');//文件名不能包含\/:*?"<>|
		else
		{
			fileName[j] = temp[i];
			j++;
		}
	}
	return fileName;
}
/*
char* OpenFileDialog()//文件选择器，需要加链接参数-lcomdlg32
{
	OPENFILENAMEA ofn;
	char* fileName =(char*) calloc(MaxFileName, sizeof(char));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = NULL;
	ofn.lpstrFile       = fileName;
	ofn.nMaxFile        = MaxFileName*sizeof(char);
	//ofn.lpstrFilter     = "所有.c*文件\0*.c*\0文本文件\0*.txt\0";
	ofn.lpstrFilter     = "所有文件\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	SetProcessDPIAware();//避免Windows缩放造成模糊
	if(GetOpenFileNameA(&ofn))
	{
		//printf("选择打开的文件是：%s\n", ofn.lpstrFile);
		//return ofn.lpstrFile;
		return fileName;
	}
	else
	{
		free(fileName);
		return NULL;
	}
}
*/
void AnalysisColor()
{
	int r, c, start, end;
	int bLevel = 0, w;
	static char* word[58] = {
		// C语言33关键字
		"auto", "break", "case", "char", "const",
		"continue", "default", "do", "double", "else",
		"enum", "extern", "float", "for", "goto",
		"if", "inline", "int", "long", "register",
		"return", "short", "signed", "sizeof", "static",
		"struct", "switch", "typedef", "union", "unsigned",
		"void", "volatile", "while",
		// 常见C++关键字增补18
		"and", "bool", "class", "delete", "false",
		"friend", "namespace", "new", "not", "operator",
		"or", "private", "public","template", "this",
		"true", "typename", "using",
		// 常见Python关键字增补7
		"False", "True", "def", "elif", "from",
		"import", "in"
	};
	static int isTypeWord[58] = {
		1, 0, 0, 1, 0,
		0, 0, 0, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 1, 0,
		0, 1, 1, 0, 0,
		0, 0, 0, 0, 1,
		1, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0
	};//是否为类型关键字
	type =(int**) calloc(numberOfRow, sizeof(int*));
	//符号着色
	for(r=0; r<numberOfRow; r++)
	{
		type[r] =(int*) calloc(numberOfColumn[r], sizeof(int));
		for(c=0; c<numberOfColumn[r]; c++)
		{
			/*----------------
			ASCII:
			0-8:    控制码
			9:      制表符\t
			10-13:  空白符\n \v \f \r
			14-31:  控制码
			32:     空格
			33-47:  !"#$%&'()*+,-./
			48-57:  0123456789
			58-64:  :;<=>?@
			65-70:  ABCDEF
			71-90:  GHIJKLMNOPQRSTUVWXYZ
			91-96:  [\]^_`
			97-102: abcdef
			103-122:ghijklmnopqrstuvwxyz
			123-126:{|}~
			127:    退格符
			----------------*/
			//33-47除"#$'，58-64除@，91-96除\_`，123-126
			if((content[r][c] >= 'a' && content[r][c] <= 'z')
				|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
				|| content[r][c] <= ' ')//对于大多数字母，不进行后续比较
			{
				type[r][c] = Type_Default;
			}
			/*else if(content[r][c] >= '0' && content[r][c] <= '9')
			{
				type[r][c] = Type_Number;
			}*/
			else if((content[r][c] >= '(' && content[r][c] <= '/')
				|| (content[r][c] >= ':' && content[r][c] <= '?')
				|| (content[r][c] >= '{' && content[r][c] <= '~')
				|| content[r][c] == '!' || content[r][c] == '%' || content[r][c] == '&'
				|| content[r][c] == '[' || content[r][c] == ']'
				|| content[r][c] == '^')//优先比较常见字符以加速
			{
				type[r][c] = Type_Symbol;
			}
			else
			{
				type[r][c] = Type_Default;
			}
		}
	}
	//数字着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] >= '0' && content[r][c] <= '9')
			{
				if(c > 0 && ((content[r][c-1] >= 'A' && content[r][c-1] <= 'Z')
					|| (content[r][c-1] >= 'a' && content[r][c-1] <= 'z')
					|| content[r][c-1] == '_'))
				{
					while(content[r][c+1] >= '0' && content[r][c+1] <= '9')
					{
						c++;//连续跳过多个数字
					}
				}
				else//数字前面不是字母或下划线
				{
					type[r][c] = Type_Number;
				}
			}
			if(content[r][c] == '.' && c > 0 && c+1 < numberOfColumn[r])//小数点
			{
				if(content[r][c-1] >= '0' && content[r][c-1] <= '9'
					&& content[r][c+1] >= '0' && content[r][c+1] <= '9')
				{
					type[r][c] = Type_Number;
				}
			}
			if(content[r][c] == '0' && type[r][c] == Type_Number && c+1 < numberOfColumn[r] && content[r][c+1] == 'x')//十六进制数
			{
				if(c+2 < numberOfColumn[r])
				{
					if((content[r][c+2] >= '0' && content[r][c+2] <= '9')
						|| (content[r][c+2] >= 'A' && content[r][c+2] <= 'F')
						|| (content[r][c+2] >= 'a' && content[r][c+2] <= 'f'))
					{
						type[r][c+1] = Type_Number;
						for(c+=2; c<numberOfColumn[r]; c++)
						{
							if((content[r][c] >= '0' && content[r][c] <= '9')
								|| (content[r][c] >= 'A' && content[r][c] <= 'F')
								|| (content[r][c] >= 'a' && content[r][c] <= 'f'))
							{
								type[r][c] = Type_Number;
							}
							else
							{
								break;
							}
						}
					}
				}
			}
		}
	}
	//预处理指令着色
	for(r=0; r<numberOfRow; r++)
	{
		if(numberOfColumn[r] > 0 && content[r][0] == '#')
		{
			for(c=0; c<numberOfColumn[r]; c++)
			{
				if(content[r][c] == '<')//将<>内以字符串着色
				{
					start = c;
					end = -1;
					for(c++; c<numberOfColumn[r]; c++)
					{
						if(content[r][c] == '>')
						{
							end = c;
							break;
						}
					}
					c = start;
					if(end != -1)
					{
						for(c++; c<end; c++)
						{
							if(type[r][c] == Type_Default)
							{
								type[r][c] = Type_String;
							}
						}
					}
					break;
				}
				else if(content[r][c] == '"')
				{
					break;
				}
				else
				{
					type[r][c] = Type_Preprocessor;
				}
			}
		}//行首为#且在<前的内容
	}
	//注释着色
	start = 0;
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '/' && c+1 < numberOfColumn[r])
			{
				if(content[r][c+1] == '*') start = 1;
				else if(content[r][c+1] == '/' && start == 0)
				{
					for(; c<numberOfColumn[r]; c++)
					{
						type[r][c] = Type_Comment;
					}
					continue;
				}
			}
			if(start == 1) type[r][c] = Type_Comment;
			if(content[r][c] == '*' && c+1 < numberOfColumn[r] && content[r][c+1] == '/')
			{
				start = 0;
				type[r][c+1] = Type_Comment;
				c++;
			}
		}
	}
	//字符和字符串着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '\'' && type[r][c] == Type_Default)
			{
				start = c;
				end = -1;
				for(c++; c<numberOfColumn[r]; c++)
				{
					/*if(content[r][c] == '\\' && type[r][c] == Type_Default && c+1 < numberOfColumn[r])
					{
						c++;//判断'\''和'\\'
					}
					else */if(content[r][c] == '\'' && type[r][c] == Type_Default)
					{
						if(content[r][c-1] == '\\' && c-2 >= 0 && content[r][c-2] != '\\')
						{
							continue;//判断'\''和'\\'
						}
						end = c;
						break;
					}
				}
				c = start;
				if(end != -1)
				{
					for(; c<=end; c++)
					{
						type[r][c] = Type_Character;
					}
					c--;
				}
			}
		}
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '"' && type[r][c] == Type_Default)
			{
				start = c;
				end = -1;
				for(c++; c<numberOfColumn[r]; c++)
				{
					if(content[r][c] == '"' && type[r][c] == Type_Default)
					{
						if(content[r][c-1] == '\\' && c-2 >= 0 && content[r][c-2] != '\\')
						{
							continue;//判断"\""和"\\"
						}
						end = c;
						break;
					}
				}
				c = start;
				if(end != -1)
				{
					start = c;
					for(; c<=end; c++)
					{
						type[r][c] = Type_String;
					}
					c = start;
					//转义序列着色
					for(c++; c<end; c++)
					{
						if(content[r][c] == '\\')
						{
							if(content[r][c+1] == '\''
								|| content[r][c+1] == '"'
								|| content[r][c+1] == '?'
								|| content[r][c+1] == '\\'
								|| content[r][c+1] == 'a'
								|| content[r][c+1] == 'b'
								|| content[r][c+1] == 'f'
								|| content[r][c+1] == 'n'
								|| content[r][c+1] == 'r'
								|| content[r][c+1] == 't'
								|| content[r][c+1] == 'v')
							{
								type[r][c] = Type_EscapeSequences;
								type[r][c+1] = Type_EscapeSequences;
								c++;
							}
							else if(content[r][c+1] >= '0' && content[r][c+1] <= '7' && c+3 < end
								&& content[r][c+2] >= '0' && content[r][c+2] <= '7'
								&& content[r][c+3] >= '0' && content[r][c+3] <= '7')
							{
								type[r][c] = Type_EscapeSequences;
								type[r][c+1] = Type_EscapeSequences;
								type[r][c+2] = Type_EscapeSequences;
								type[r][c+3] = Type_EscapeSequences;
								c += 3;
							}
							else if(content[r][c+1] == '0')
							{
								type[r][c] = Type_EscapeSequences;
								type[r][c+1] = Type_EscapeSequences;
								c++;
							}
							else if(content[r][c+1] == 'x')
							{
								type[r][c] = Type_EscapeSequences;
								type[r][c+1] = Type_EscapeSequences;
								for(c++; c+1 < numberOfColumn[r]; c++)
								{
									if((content[r][c+1] >= '0' && content[r][c+1] <= '9')
										|| (content[r][c+1] >= 'A' && content[r][c+1] <= 'F')
										|| (content[r][c+1] >= 'a' && content[r][c+1] <= 'f'))
									{
										type[r][c+1] = Type_EscapeSequences;
									}
									else
									{
										break;
									}
								}
							}
						}
					}
					//c = end;
				}
			}
		}
	}
	//彩虹括号着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(type[r][c] == Type_Symbol)
			{
				if(content[r][c] == '(' || content[r][c] == ')'
					|| content[r][c] == '[' || content[r][c] == ']'
					|| content[r][c] == '{' || content[r][c] == '}')
				{
					if(content[r][c] == '(' || content[r][c] == '[' || content[r][c] == '{')
					{
						bLevel++;
					}
					if(bLevel % 4 == 0) type[r][c] = Type_Bracket_L1;
					else if(bLevel % 4 == 1) type[r][c] = Type_Bracket_L2;//保持底层为L2
					else if(bLevel % 4 == 2) type[r][c] = Type_Bracket_L3;
					else if(bLevel % 4 == 3) type[r][c] = Type_Bracket_L4;
					if(content[r][c] == ')' || content[r][c] == ']' || content[r][c] == '}')
					{
						bLevel--;
					}
				}
			}
		}
	}
	//关键字着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(c > 0 && ((content[r][c-1] >= '0' && content[r][c-1] <= '9')
				|| (content[r][c-1] >= 'A' && content[r][c-1] <= 'Z')
				|| (content[r][c-1] >= 'a' && content[r][c-1] <= 'z')
				|| content[r][c-1] == '_'));
			else if(type[r][c] == Type_Default)//仅搜索未着色部分
			{
				start = c;
				end = start;
				for(w=0; w<58; w++)
				{
					for(; word[w][end-start]!=0; end++)
					{
						if(end < numberOfColumn[r] && content[r][end] == word[w][end-start]);
						else break;
					}
					if(word[w][end-start] == 0)
					{
						if(end < numberOfColumn[r] && ((content[r][end] >= '0' && content[r][end] <= '9')
							|| (content[r][end] >= 'A' && content[r][end] <= 'Z')
							|| (content[r][end] >= 'a' && content[r][end] <= 'z')
							|| content[r][end] == '_'))
						{
							end = start;
							continue;
						}
						else
						{
							end--;
							break;
						}
					}
					end = start;
				}
				c = start;
				if(end > start)
				{
					for(start=c; c<=end; c++)
					{
						if(isTypeWord[w] == 1) type[r][c] = Type_ReservedWord_T;
						else type[r][c] = Type_ReservedWord;
					}
					c--;
				}
			}
		}
	}
	//标识符着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c+1 < numberOfColumn[r]; c++)
		{
			if(type[r][c] == Type_Default && content[r][c+1] == '(')
			{
				for(end=c; c>=0; c--)//(前为函数
				{
					if((content[r][c] >= '0' && content[r][c] <= '9')
						|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
						|| (content[r][c] >= 'a' && content[r][c] <= 'z')
						|| content[r][c] == '_')
					{
						type[r][c] = Type_Function;
					}
					else
					{
						break;
					}
				}
				c = end;//防止死循环
			}
		}
		for(c=0; c+1 < numberOfColumn[r]; c++)//.后为变量
		{
			if(content[r][c] == '.' && type[r][c+1] == Type_Default)
			{
				for(c++; c<numberOfColumn[r]; c++)
				{
					if((content[r][c] >= '0' && content[r][c] <= '9')
						|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
						|| (content[r][c] >= 'a' && content[r][c] <= 'z')
						|| content[r][c] == '_')
					{
						type[r][c] = Type_Variable;
					}
					else
					{
						break;
					}
				}
				c--;//连续.处理
			}
		}
		for(c=0; c+2 < numberOfColumn[r]; c++)//->后为变量
		{
			if(content[r][c] == '-' && content[r][c+1] == '>' && type[r][c+2] == Type_Default)
			{
				for(c+=2; c<numberOfColumn[r]; c++)
				{
					if((content[r][c] >= '0' && content[r][c] <= '9')
						|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
						|| (content[r][c] >= 'a' && content[r][c] <= 'z')
						|| content[r][c] == '_')
					{
						type[r][c] = Type_Variable;
					}
					else
					{
						break;
					}
				}
				c--;//连续->处理
			}
		}
		for(c=0; c+2 < numberOfColumn[r]; c++)//::前为字符串(命名空间)
		{
			if(type[r][c] == Type_Default && content[r][c+1] == ':' && content[r][c+2] == ':')
			{
				for(end=c; c>=0; c--)
				{
					if((content[r][c] >= '0' && content[r][c] <= '9')
						|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
						|| (content[r][c] >= 'a' && content[r][c] <= 'z')
						|| content[r][c] == '_')
					{
						type[r][c] = Type_String;
					}
					else
					{
						break;
					}
				}
				c = end;
			}
		}
		for(c=0; c<numberOfColumn[r]; c++)//开头为大写字母或下划线视为全局变量
		{
			if(type[r][c] == Type_Default && content[r][c] != ' ' && content[r][c] != '\t')
			{
				if((content[r][c] >= 'A' && content[r][c] <= 'Z')
					|| content[r][c] == '_')
				{
					for(; c<numberOfColumn[r]; c++)
					{
						if(type[r][c] == Type_Default && content[r][c] != ' ' && content[r][c] != '\t')
						{
							type[r][c] = Type_Variable_G;
						}
						else
						{
							break;
						}
					}
				}
				else//剩余内容均视为局部变量
				{
					for(; c<numberOfColumn[r]; c++)
					{
						if(type[r][c] == Type_Default && content[r][c] != ' ' && content[r][c] != '\t')
						{
							type[r][c] = Type_Variable_L;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}
}

int TabBefore(int r, int c1)
{
	int c, num = 0;
	for(c=0; c<c1; c++)
	{
		if(content[r][c] == '\t')
		{
			num++;
		}
	}
	return num;
}

int NegBefore(int r, int c1)
{
	int c, num = 0;
	for(c=0; c<c1; c++)
	{
		if(content[r][c] < 0)
		{
			num++;
		}
	}
	return num;
}

int Place(int n)//计算某数所占位数
{
	int i = 0;
	if(n == 0) return 1;
	while(n > 0)
	{
		i++;
		n /= 10;
	}
	return i;
}

void PrintGutter(int r, int al)
{
	int c;
	gotoxy(0, r);
	for(c=0; c<Place(numberOfRow)-Place(r+1); c++)
	{
		ColorChar(' ', Color[Type_Gutter]);
	}
	if(al == 1) ColorNumber(r+1, Color[Type_Gutter_AL]);
	else ColorNumber(r+1, Color[Type_Gutter]);
	ColorChar(' ', Color[Type_Gutter]);
}

void PrintContentR(int r)
{
	int c, ishead = 1;
	if(r == cursorR) PrintGutter(r, 1);
	else PrintGutter(r, 0);
	for(c=0; c<numberOfColumn[r]; c++)
	{
		if(c == numberOfColumn[r]-1)
		{
			printf(" \b");//删除时抹去尾部
		}
		if(ishead == 1 && content[r][c] != '\t' && content[r][c] != ' ') ishead = 0;
		if(ishead == 1)
		{
			if(content[r][c] == '\t')
			{
				//printf("    ");
				if(c % 4 == 0) ColorChar(':', Color[Type_Bracket_L2]);
				else if(c % 4 == 1) ColorChar(':', Color[Type_Bracket_L3]);
				else if(c % 4 == 2) ColorChar(':', Color[Type_Bracket_L4]);
				else if(c % 4 == 3) ColorChar(':', Color[Type_Bracket_L1]);
				//ColorChar(':', Color_Default/16*16 + 0x08);
				printf("   ");
			}
			else if(content[r][c] == ' ')
			{
				if(c % 4 == 0)
				{
					if(c/4 % 4 == 0) ColorChar(':', Color[Type_Bracket_L2]);
					else if(c/4 % 4 == 1) ColorChar(':', Color[Type_Bracket_L3]);
					else if(c/4 % 4 == 2) ColorChar(':', Color[Type_Bracket_L4]);
					else if(c/4 % 4 == 3) ColorChar(':', Color[Type_Bracket_L1]);
				}
				else
				{
					printf(" ");
				}
			}
		}
		else
		{
			//putchar(content[r][c]);
			if(content[r][c] == '\t')
			{
				printf("    ");
			}
			else
			{
				ColorChar(content[r][c], Color[type[r][c]]);
			}
		}
	}
}

void PrintContent()
{
	int r;
	clrscr();
	for(r=0; r<numberOfRow; r++)
	{
		PrintContentR(r);
	}
}

void ReadContent(char* fileName)
{
	FILE* file;
	char ch;
	int r, c;
	//int cn = 0;
	numberOfRow = 0;
	if((file = fopen(fileName, "r")))
	{
		//printf("[Opened]File %s:\n", fileName);
		/*while(1)//调试
		{
			c = fgetc(file);
			if(c == EOF)//-1
			{
				printf("[文件尾]");
				break;
			}
			else if(c == '\n') printf("[换行]");//文件以'\n'为换行
			else if(c == '\r') printf("[回车]");
			else putchar(c);
		}
		rewind(file);*/
		//分析行数
		numberOfRow = 1;//EOF也有一行
		while(1)
		{
			ch = fgetc(file);
			if(ch == '\n') numberOfRow++;
			if(ch == EOF) break;
		}
		//printf("行数：%d\n", numberOfRow);
		content =(char**) calloc(numberOfRow, sizeof(char*));
		numberOfColumn =(int*) calloc(numberOfRow, sizeof(int));
		//分析列数
		rewind(file);//返回到文件首
		ch = 0;
		for(r=0; ch!=EOF; r++)
		{
			ch = 0;
			for(c=0; ch!='\n' && ch!=EOF; c++)
			{
				ch = fgetc(file);//将'\n'和EOF计入
			}
			content[r] =(char*) calloc(c, sizeof(char));
			numberOfColumn[r] = c;
			//printf("行%d列数%d\n", r, c);
		}
		//存入缓存
		rewind(file);
		r = 0;
		c = 0;
		while(1)
		{
			ch = fgetc(file);
			content[r][c] = ch;
			c++;
			if(ch == '\n')
			{
				r++;
				c = 0;
			}
			if(ch == EOF) break;
		}
		//输出缓存
		/*printf("\n");
		system("chcp 65001");
		for(r=0; r<numberOfRow; r++)
		{
			printf("%2d(%2d) ", r, numberOfColumn[r]);
			for(c=0; c<numberOfColumn[r]; c++)
			{
				if(content[r][c] == EOF) printf("[EOF]");
				else if(content[r][c] == '\n') printf("[\\n]");
				else if(content[r][c] == '\r') printf("[\\r]");
				else if(content[r][c] == '\t') printf("[\\t]");
				else if(content[r][c] >= 32 && content[r][c] <= 126) putchar(content[r][c]);
				else
				{
					putchar(content[r][c]);
					putchar(content[r][c+1]);
					putchar(content[r][c+2]);
					printf("[%d %d %d]", content[r][c], content[r][c+1], content[r][c+2]);
					c+=2;
				}
			}
			printf("\n");
		}
		getchar();*/
	}
	else
	{
		printf("[Error]Can't find file %s\n", fileName);
		file = fopen(fileName, "w");//新建该文件
	}
	if(numberOfRow == 0)
	{
		numberOfRow = 1;
		content =(char**) calloc(numberOfRow, sizeof(char*));
		numberOfColumn =(int*) calloc(numberOfRow, sizeof(int));
		content[0] =(char*) calloc(1, sizeof(char));
		content[0][0] = EOF;
		numberOfColumn[0] = 1;
	}
	fclose(file);
}

void WriteContent(char* fileName)
{
	FILE* file = fopen(fileName, "w");
	int r, c;
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r] && content[r][c] != EOF; c++)
		{
			fputc(content[r][c], file);
		}
	}
	fclose(file);
}

int Operate(char operation)
{
	int r, c;
	char* newLine;
	char** newContent;
	int* newNumberOfColumn;
	if(operation == '\b')
	{
		if(cursorC != 0)//删除前一个字符
		{
			newLine =(char*) calloc(numberOfColumn[cursorR]-1, sizeof(char));
			for(c=0; c<cursorC-1; c++)
			{
				newLine[c] = content[cursorR][c];
			}
			for(c=cursorC-1; c<numberOfColumn[cursorR]-1; c++)
			{
				newLine[c] = content[cursorR][c+1];
			}
			free(content[cursorR]);
			content[cursorR] = newLine;
			cursorC--;
			numberOfColumn[cursorR]--;
		}
		else if(cursorR != 0)//删除换行，合并两行
		{
			//合并cursorR-1和cursorR行
			newLine =(char*) calloc(numberOfColumn[cursorR-1]-1+numberOfColumn[cursorR], sizeof(char));
			for(c=0; c<numberOfColumn[cursorR-1]-1; c++)//去除换行符
			{
				newLine[c] = content[cursorR-1][c];
			}
			for(c=0; c<numberOfColumn[cursorR]; c++)
			{
				newLine[numberOfColumn[cursorR-1]-1+c] = content[cursorR][c];
			}
			free(content[cursorR-1]);
			content[cursorR-1] = newLine;
			cursorC = numberOfColumn[cursorR-1]-1;//光标列定位
			numberOfColumn[cursorR-1] = numberOfColumn[cursorR-1]-1+numberOfColumn[cursorR];
			//删除cursorR行
			free(content[cursorR]);
			//cursorR+1行及之后前移
			for(r=cursorR; r<numberOfRow-1; r++)
			{
				content[r] = content[r+1];
				numberOfColumn[r] = numberOfColumn[r+1];
			}
			cursorR--;//光标行定位
			//删除尾行空间
			free(type[numberOfRow-1]);//color在main中释放
			numberOfRow--;
			newContent =(char**) calloc(numberOfRow, sizeof(char*));
			newNumberOfColumn =(int*) calloc(numberOfRow, sizeof(int));
			for(r=0; r<numberOfRow; r++)
			{
				newContent[r] = content[r];
				newNumberOfColumn[r] = numberOfColumn[r];
			}
			free(content);
			free(numberOfColumn);
			content = newContent;
			numberOfColumn = newNumberOfColumn;
			return 1;
		}
		return 0;
	}
	else if(operation == '\r')//插入行
	{
		newContent =(char**) calloc(numberOfRow+1, sizeof(char*));
		newNumberOfColumn =(int*) calloc(numberOfRow+1, sizeof(int));
		//cursorR+1行及之后后移
		for(r=numberOfRow; r>cursorR+1; r--)
		{
			newContent[r] = content[r-1];
			newNumberOfColumn[r] = numberOfColumn[r-1];
		}
		//复制cursorR-1行及之前行
		for(r=0; r<cursorR; r++)
		{
			newContent[r] = content[r];
			newNumberOfColumn[r] = numberOfColumn[r];
		}
		//处理cursorR和cursorR+1行
		if(cursorC == numberOfColumn[cursorR]-1)
		{
			newLine =(char*) calloc(1, sizeof(char));
			newLine[0] = '\n';
			newContent[cursorR+1] = newLine;
			newNumberOfColumn[cursorR+1] = 1;
			newContent[cursorR] = content[cursorR];
			if(newContent[cursorR][cursorC] != '\n') newContent[cursorR][cursorC] = '\n';
			newNumberOfColumn[cursorR] = numberOfColumn[cursorR];
		}
		else
		{
			newLine =(char*) calloc(numberOfColumn[cursorR]-cursorC, sizeof(char));
			for(c=cursorC; c<numberOfColumn[cursorR]; c++)
			{
				newLine[c-cursorC] = content[cursorR][c];
			}
			newContent[cursorR+1] = newLine;
			newNumberOfColumn[cursorR+1] = numberOfColumn[cursorR]-cursorC;
			//cursorC位置为换行符
			newContent[cursorR] =(char*) calloc(cursorC+1, sizeof(char));
			for(c=0; c<cursorC; c++)
			{
				newContent[cursorR][c] = content[cursorR][c];
			}
			free(content[cursorR]);
			newContent[cursorR][cursorC] = '\n';
			newNumberOfColumn[cursorR] = cursorC+1;
		}
		cursorR++;
		cursorC = 0;
		numberOfRow++;
		type =(int**) realloc(type, numberOfRow * sizeof(int*));
		type[numberOfRow-1] =(int*) calloc(numberOfColumn[r], sizeof(int));//防止main中释放时闪退
		//CheckGlobalPointer();
		free(content);
		free(numberOfColumn);
		content = newContent;
		numberOfColumn = newNumberOfColumn;
		return 1;
	}
	else//插入字符
	{
		newLine =(char*) calloc(numberOfColumn[cursorR]+1, sizeof(char));
		for(c=numberOfColumn[cursorR]; c>cursorC; c--)
		{
			newLine[c] = content[cursorR][c-1];
		}
		newLine[cursorC] = operation;
		for(c=cursorC-1; c>=0; c--)
		{
			newLine[c] = content[cursorR][c];
		}
		free(content[cursorR]);
		content[cursorR] = newLine;
		cursorC++;
		numberOfColumn[cursorR]++;
		return 0;
	}
}

int EditContent()
{
	int operation, /*r,*/ c, tab;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	INPUT_RECORD rcd;
	DWORD rcdnum;
	gotoxy(cursorC+3*TabBefore(cursorR, cursorC)+Place(numberOfRow)+1-NegBefore(cursorR, cursorC)/3, cursorR);
	while(1)
	{
		ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
		if(rcd.EventType == MOUSE_EVENT)
		{
			if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				PrintGutter(cursorR, 0);
				mousePos = rcd.Event.MouseEvent.dwMousePosition;
				cursorR = mousePos.Y;
				if(cursorR > numberOfRow-1) cursorR = numberOfRow-1;
				cursorC = mousePos.X-3*TabBefore(cursorR, mousePos.X)-Place(numberOfRow)-1+NegBefore(cursorR, cursorC)/3;
				while(cursorC+3*TabBefore(cursorR, cursorC)+Place(numberOfRow)+1-NegBefore(cursorR, cursorC)/3 < mousePos.X) cursorC++;
				while(NegBefore(cursorR, cursorC)%3 != 0 && content[cursorR][cursorC] < 0) cursorC++;
				if(cursorC > numberOfColumn[cursorR]-1) cursorC = numberOfColumn[cursorR]-1;
				PrintGutter(cursorR, 1);
				gotoxy(cursorC+3*TabBefore(cursorR, cursorC)+Place(numberOfRow)+1-NegBefore(cursorR, cursorC)/3, cursorR);
			}
			if(rcd.Event.MouseEvent.dwEventFlags == MOUSE_WHEELED)
			{
				PrintGutter(cursorR, 0);
				//printf("%lld", rcd.Event.MouseEvent.dwButtonState);
				if(rcd.Event.MouseEvent.dwButtonState/0x10000 == 0xff88)//4287102976滚轮向下
				{
					if(cursorR+3 < numberOfRow) cursorR += 3;
					else if(cursorR < numberOfRow-1) cursorR = numberOfRow-1;
				}
				else if(rcd.Event.MouseEvent.dwButtonState/0x10000 == 0x78)//7864320滚轮向上
				{
					if(cursorR-3 >= 0) cursorR -= 3;
					else if(cursorR > 0) cursorR = 0;
				}
				while(NegBefore(cursorR, cursorC)%3 != 0 && content[cursorR][cursorC] < 0) cursorC++;
				if(cursorC > numberOfColumn[cursorR]-1) cursorC = numberOfColumn[cursorR]-1;
				PrintGutter(cursorR, 1);
				gotoxy(cursorC+3*TabBefore(cursorR, cursorC)+Place(numberOfRow)+1-NegBefore(cursorR, cursorC)/3, cursorR);
			}
		}
		else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
		{
			operation = rcd.Event.KeyEvent.wVirtualKeyCode;
			if(operation == VK_UP || operation == VK_LEFT || operation == VK_DOWN || operation == VK_RIGHT)
			{
				PrintGutter(cursorR, 0);
				if(operation == VK_UP)
				{
					if(cursorR > 0) cursorR--;
					if(cursorC > numberOfColumn[cursorR]-1) cursorC = numberOfColumn[cursorR]-1;
				}
				else if(operation == VK_LEFT)
				{
					if(cursorC > 0)
					{
						if(content[cursorR][cursorC-1] < 0 && cursorC > 2) cursorC -= 3;
						else cursorC--;
					}
					else if(cursorR > 0)//行首左移尝试上一行
					{
						cursorR--;
						cursorC = numberOfColumn[cursorR]-1;
					}
				}
				else if(operation == VK_DOWN)
				{
					if(cursorR < numberOfRow-1) cursorR++;
					if(cursorC > numberOfColumn[cursorR]-1) cursorC = numberOfColumn[cursorR]-1;
				}
				else if(operation == VK_RIGHT)
				{
					if(cursorC < numberOfColumn[cursorR]-1) cursorC++;//列最大为换行符位置
					else if(cursorR < numberOfRow-1)//行末右移尝试下一行
					{
						cursorR++;
						cursorC = 0;
					}
				}
				while(NegBefore(cursorR, cursorC)%3 != 0 && content[cursorR][cursorC] < 0) cursorC++;
				PrintGutter(cursorR, 1);
				gotoxy(cursorC+3*TabBefore(cursorR, cursorC)+Place(numberOfRow)+1-NegBefore(cursorR, cursorC)/3, cursorR);
				continue;
			}
			if(operation == VK_CAPITAL)
			{
				//printf("%x", rcd.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON != 0);//&优先级低
				if(rcd.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON)
				{
					SetConsoleMouseMode(0);
					while(rcd.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						Sleep(100);
					}
					SetConsoleMouseMode(1);
				}
			}
			if(GetKeyState(VK_CONTROL) < 0)
			{
				if(operation == 'S')
				{
					if(GetKeyState(VK_SHIFT) < 0)
					{
						//printf("Ctrl+Shift+S");
						SetConsoleMouseMode(0);
						WriteContent(InputFileName());
						SetConsoleMouseMode(1);
					}
					else
					{
						//printf("Ctrl+S");
						WriteContent(fileName);
					}
					printf("[Saved!]");
					continue;
				}
			}
			if(operation >= 'A' && operation <= 'Z') operation = operation-'A'+'a';
			else if(operation >= 'a' && operation <= 'i') operation = operation-'a'+'1';
			else if(operation == '`') operation = '0';
			else if(operation == 'n') operation = '.';
			else if(operation == 'k') operation = '+';
			else if(operation == 'm') operation = '-';
			else if(operation == 'j') operation = '*';
			else if(operation == 'o') operation = '/';
			else if(operation == VK_OEM_MINUS) operation = '-';
			else if(operation == VK_OEM_PLUS) operation = '=';
			else if(operation == VK_OEM_4) operation = '[';
			else if(operation == VK_OEM_6) operation = ']';
			else if(operation == VK_OEM_5) operation = '\\';
			else if(operation == VK_OEM_1) operation = ';';
			else if(operation == VK_OEM_7) operation = '\'';
			else if(operation == VK_OEM_COMMA) operation = ',';
			else if(operation == VK_OEM_PERIOD) operation = '.';
			else if(operation == VK_OEM_2) operation = '/';
			else if(operation == VK_OEM_3) operation = '`';
			if(GetKeyState(VK_SHIFT) < 0)//Shift键转换
			{
				if(operation >= 'a' && operation <= 'z')
				{
					operation = operation-'a'+'A';
				}
				else if(operation == '1') operation = '!';
				else if(operation == '2') operation = '@';
				else if(operation == '3') operation = '#';
				else if(operation == '4') operation = '$';
				else if(operation == '5') operation = '%';
				else if(operation == '6') operation = '^';
				else if(operation == '7') operation = '&';
				else if(operation == '8') operation = '*';
				else if(operation == '9') operation = '(';
				else if(operation == '0') operation = ')';
				else if(operation == '-') operation = '_';
				else if(operation == '=') operation = '+';
				else if(operation == '[') operation = '{';
				else if(operation == ']') operation = '}';
				else if(operation == '\\') operation = '|';
				else if(operation == ';') operation = ':';
				else if(operation == '\'') operation = '"';
				else if(operation == ',') operation = '<';
				else if(operation == '.') operation = '>';
				else if(operation == '/') operation = '?';
				else if(operation == '`') operation = '~';
			}
			if(operation == '\b' || operation == '\r' || operation == '\t'
				|| (operation >= 32 && operation < 128))
			{
				break;
			}
			else if(operation != VK_SHIFT)
			{
				//printf("[UnknownKey0x%x]", operation);
			}
		}
		Sleep(20);
	}
	if(operation == '\r')
	{
		if(cursorC > 0 && content[cursorR][cursorC-1] == '{')
		{
			tab = TabBefore(cursorR, cursorC);
			Operate('\r');
			if(content[cursorR][cursorC] == '}')
			{
				Operate('\r');
				for(c=0; c<tab; c++)
				{
					Operate('\t');
				}
				cursorR--;
				cursorC = 0;
			}
			for(c=0; c<tab+1; c++)
			{
				Operate('\t');
			}
			return 1;
		}
		if(content[cursorR][0] == '\t')
		{
			tab = TabBefore(cursorR, cursorC);
			Operate('\r');
			for(c=0; c<tab; c++)
			{
				Operate('\t');
			}
			return 1;
		}
	}
	if(operation == '(' && content[cursorR][cursorC] != ')')
	{
		Operate(')');
		cursorC--;
	}
	else if(operation == '[' && content[cursorR][cursorC] != ']')
	{
		Operate(']');
		cursorC--;
	}
	else if(operation == '{' && content[cursorR][cursorC] != '}')
	{
		Operate('}');
		cursorC--;
	}
	else if(operation == '\'' && content[cursorR][cursorC] != '\'')
	{
		Operate('\'');
		cursorC--;
	}
	else if(operation == '"' && content[cursorR][cursorC] != '"')
	{
		Operate('"');
		cursorC--;
	}
	else if(operation == '<' && content[cursorR][cursorC] != '>' && content[cursorR][0] == '#')
	{
		Operate('>');
		cursorC--;
	}
	else if(operation == '\b' && cursorC > 0)
	{
		if((content[cursorR][cursorC-1] == '(' && content[cursorR][cursorC] == ')')
			|| (content[cursorR][cursorC-1] == '[' && content[cursorR][cursorC] == ']')
			|| (content[cursorR][cursorC-1] == '{' && content[cursorR][cursorC] == '}')
			|| (content[cursorR][cursorC-1] == '\'' && content[cursorR][cursorC] == '\'')
			|| (content[cursorR][cursorC-1] == '"' && content[cursorR][cursorC] == '"')
			|| (content[cursorR][cursorC-1] == '<' && content[cursorR][cursorC] == '>' && content[cursorR][0] == '#'))
		{
			cursorC++;
			Operate('\b');
			PrintContentR(cursorR);//多执行一次尾部抹除
		}
		else if(cursorC > 2
			&& content[cursorR][cursorC-1] < 0
			&& content[cursorR][cursorC-2] < 0
			&& content[cursorR][cursorC-3] < 0)
		{
			Operate('\b');
			Operate('\b');
			Operate('\b');//UTF-8中文删除时连续删除3字符
			Operate(' ');//空格去残影
			PrintContentR(cursorR);
		}
	}
	return Operate(operation);
}

void AdaptScreenBuffer()//自适应屏幕缓冲区
{
	int r, maxNumberOfColumn = 0;
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);
	//printf("%d %d\n", csbi.dwSize.X, csbi.dwSize.Y);
	for(r=0; r<numberOfRow; r++)
	{
		if(numberOfColumn[r] > maxNumberOfColumn)
		{
			maxNumberOfColumn = numberOfColumn[r];
		}
	}
	maxNumberOfColumn += Place(numberOfRow)+1;
	if(csbi.dwSize.X < maxNumberOfColumn || csbi.dwSize.Y < numberOfRow)
	{
		if(csbi.dwSize.X < maxNumberOfColumn) csbi.dwSize.X = maxNumberOfColumn;
		if(csbi.dwSize.Y < numberOfRow) csbi.dwSize.Y = numberOfRow;
		SetConsoleScreenBufferSize(hdout, csbi.dwSize);
	}
}

int main(int argc, char** argv)
{
	int r, f;
	setbgcolor(Color[Type_Default]);
	printf("Text Editor使用说明：\n");
	printf("输入名称或拖入文件后回车打开文件，若不存在则自动创建该文件。\n");
	printf("也可拖动文件至程序图标，或设置程序为文件默认打开方式以打开文件。\n");
	printf("此程序以UTF-8编码显示文件内容，自动显示高亮和缩进提示线，无视文件后缀。\n");
	printf("使用鼠标左键或方向键定位光标，使用滚轮时光标移动3行每格。\n");
	printf("使用键盘输入小写英文字母、数字和符号，按住Shift即可输入大写英文字母和符号。\n");
	printf("按下CapsLk时，若CapsLk亮起，则临时禁用编辑功能，直到CapsLk熄灭。\n");
	printf("Ctrl+S保存，Ctrl+Shift+S另存为。保存后仍为当前文件编辑状态。\n");
	/*while(1)
	{
		//fileName = InputFileName();
		fileName = OpenFileDialog();
		printf("[File]>%s\n", fileName);
		ReadContent(fileName);
		int clock0 = clock();
		AnalysisColor();
		int clock1 = clock();
		printf("行数：%d\n色彩分析耗时：%dms\n", numberOfRow, clock1-clock0);
	}*/
	if(argc == 2)
	{
		fileName = argv[1];
	}
	else
	{
		fileName = InputFileName();
		//fileName = OpenFileDialog();
		//printf("[File]>%s\n", fileName);
		//fileName = "Text Editor.c";
	}
	ReadContent(fileName);
	system("chcp 65001");//以UTF-8编码显示
	AnalysisColor();
	AdaptScreenBuffer();
	PrintContent();
	SetConsoleMouseMode(1);
	while(1)
	{
		//getchar();
		f = EditContent();
		for(r=0; r<numberOfRow; r++)
		{
			free(type[r]);
		}
		free(type);
		AnalysisColor();
		//AdaptScreenBuffer();
		if(f == 0) PrintContentR(cursorR);//仅重绘当前行
		else PrintContent();
	}
	WriteContent(fileName);
	return 0;
}
/*--------------------------------
Text Editer 0.2
——新增 数字小数点、十六进制数着色
——优化 使用低区分配色
——优化 UTF-8编码显示
——优化 标识符内数字判断
——优化 字符和字符串在'和"混杂场景着色
——优化 不再将剩余内容均视为变量
——修复 color内存分配错误
——修复 多分配一行内存
Text Editer 0.3
——新增 关键字着色
——新增 缩进提示线显示
——新增 初步区分函数和变量
——优化 更准确的判断'\''和'\\'
Text Editor 0.4
——新增 简单的WASD光标移动
——新增 插入和删除字符
——新增 插入和删除换行
——优化 刷新时仅重绘当前行
——修复 更改程序名称为Text Editor
Text Editor 0.5
——新增 新建不存在的文件
——新增 读取到0行文件时，新建1行
——优化 前有\t的光标位置显示
——优化 插入和删除换行时重绘全部
——修复 插入和删除换行时可能闪退
Text Editor 0.6
——新增 鼠标定位光标
——新增 插入换行时自动保持\t数与当前行一致
——新增 在{}间插入换行时自动保持}前\t数与{前一致
——新增 在{}间插入换行时自动多插入\t数为{前\t数+1的行并定位光标
——新增 插入([{'"时，如果插入位置不为匹配符号，则自动插入并左移光标
——新增 插入<时，如果插入位置不为>且行首为#，则自动插入>并左移光标
——新增 删除([{'"<时，自动删除其右侧的匹配符号
——新增 Ctrl+S保存文件
——新增 从0开始的行号
——优化 WASD光标移动改为方向键
——修复 新行退格不会抹去尾部
Text Editor 0.7
——新增 初步区分字符串内转义序列
——新增 4级彩虹括号和彩虹缩进
——新增 4空格显示彩虹缩进
——新增 行号着色
——优化 增补3个常见关键字(bool,false,true)
——优化 方向键行首左移或行末右移时移动光标
——优化 Color_Default可改变背景色
——优化 使用高区分配色
Text Editor 0.8
——新增 当前行号
——新增 滚轮拨动时光标移动3行
——新增 大写锁定时禁用编辑功能
——新增 使用说明
——优化 行号从1开始
——优化 按键检测周期由100ms缩短至20ms
——优化 区分Ctrl+S保存和Ctrl+Shift+S另存为
Text Editor 0.9
——新增 在{后插入换行时，如果插入位置不为}，新行自动\t数+1
——新增 一键删除一个中文字符(连续3个负值字符)
——优化 在文件头部和尾部的滚轮光标移动
——优化 增补12个常见C++/Python关键字
——优化 带有中文字符时的光标位置显示
——优化 光标位置不再定位到中文字符中间
——优化 不再将EOF写入文件
——优化 更准确的判断"\""和"\\"
——修复 连续删除右侧有匹配符号的([{'"<时可能闪退
——修复 新建文件再次打开的行数偏差1
Text Editor 0.10
——新增 识别转义序列\0
——新增 十六进制数转义序列
——修复 不能识别八进制数转义序列
Text Editor 0.11
——优化 着色效率
Text Editor 0.12
——新增 将结构体内变量视为变量
——新增 自适应屏幕缓冲区
——新增 命名空间识别为字符串
——优化 部分边界检查
——优化 include双引号视为字符串
——优化 include尖括号以字符串着色
——优化 现在剩余内容均视为局部变量而非变量
Text Editor 0.13
——新增 区分关键字和类型关键字
Text Editor 0.14
——优化 块注释的中间行尾着色
——优化 更准确的判断注释的/和*混用
——优化 不再识别与块注释互套的字符和字符串
——优化 标识符尾部连续数字识别
——修复 in开头的3字符识别为关键字
——修复 新建文件的首次换行再次打开消失
Text Editor 0.15
——新增 将大写字母或下划线开头的单词视为全局变量
——优化 自适应屏幕缓冲区考虑行号
——修复 块注释的终止会被行注释取消
Text Editor 0.16
——新增 拖动文件到程序图标打开
——优化 增补10个常见C++/Python关键字
//——新增 插入中文字符
//——新增 文件选择器
--------------------------------*/
