#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MaxFileName 128

char** content;
int** color;
int numberOfRow;
int* numberOfColumn;

enum Color {
	//高区分配色
	/*Color_Default = 0x07,
	Color_Comment = 0x02,
	Color_Symbol = 0x0c,
	Color_Bracket = 0x0c,//ced9
	Color_Number = 0x0f,
	Color_Preprocessor = 0x0d,
	Color_Character = 0x0a,
	Color_String = 0x06,
	Color_ReservedWord = 0x05,
	Color_Function = 0x0e,
	Color_Variable = 0x0b*/
	//低区分配色
	Color_Default = 0x07,
	Color_Comment = 0x02,
	Color_Symbol = 0x0c,
	Color_Bracket = 0x0c,
	Color_Number = 0x09,
	Color_Preprocessor = 0x0d,
	Color_Character = 0x09,
	Color_String = 0x09,
	Color_ReservedWord = 0x0d,
	Color_Function = 0x0e,
	Color_Variable = 0x0b
};

void ColorChar(char c, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	putchar(c);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color_Default);
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

void DrawColorInRange(int r1, int c1, int r2, int c2, int cl)
{
	int r, c;
	for(r=r1; r<=r2; r++)
	{
		if(r == r1) c = c1;
		else c = 0;
		while(c < numberOfColumn[r])
		{
			color[r][c] = cl;
			if(r == r2 && c == c2) break;
			c++;
		}
	}
}

int FindMatch(int r, int c1, char ch)
{
	int c;
	for(c=c1+1; c<numberOfColumn[r]; c++)
	{
		if(content[r][c] == ch && color[r][c] == Color_Default)
		{
			if(ch == '\'' && content[r][c-1] == '\\'
				&& c-2 >= 0 && content[r][c-2] != '\\')//判断'\''和'\\'
			{
				continue;
			}
			return c;
		}
	}
	return -1;
}

int FindReservedWord(int r1, int c1)
{
	static char* word[33] = {
		"auto", "break", "case", "char", "const",
		"continue", "default", "do", "double", "else",
		"enum", "extern", "float", "for", "goto",
		"if", "inline", "int", "long", "register",
		"return", "short", "signed", "sizeof", "static",
		"struct", "switch", "typedef", "union", "unsigned",
		"void", "volatile", "while"
	};
	int i, j;
	for(i=0; i<33; i++)//在所有关键字中比较
	{
		for(j=0; word[i][j]!=0; j++)
		{
			if(c1+j < numberOfColumn[r1] && content[r1][c1+j] == word[i][j]);
			else break;
		}
		if(word[i][j] == 0)
		{
			if(c1+j < numberOfColumn[r1] && ((content[r1][c1+j] >= '0' && content[r1][c1+j] <= '9')
				|| (content[r1][c1+j] >= 'A' && content[r1][c1+j] <= 'Z')
				|| (content[r1][c1+j] >= 'a' && content[r1][c1+j] <= 'z')
				|| content[r1][c1+j] == '_'))
			{
				continue;
			}
			return j;
		}
	}
	return 0;//返回匹配关键字字符数
}

void AnalysisColor()
{
	int r, c, commentStart/*, characterStart, stringStart*/;
	int end;
	color =(int**) calloc(numberOfRow, sizeof(int*));
	//符号着色
	for(r=0; r<numberOfRow; r++)
	{
		color[r] =(int*) calloc(numberOfColumn[r], sizeof(int));
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
			color[r][c] = Color_Default;
			if(content[r][c] == '!'
				|| content[r][c] == '%'
				|| content[r][c] == '&'
				|| content[r][c] == '*'
				|| content[r][c] == '+'
				|| content[r][c] == ','
				|| content[r][c] == '-'
				|| content[r][c] == '.'
				|| content[r][c] == '/'
				|| content[r][c] == ':'
				|| content[r][c] == ';'
				|| content[r][c] == '<'
				|| content[r][c] == '='
				|| content[r][c] == '>'
				|| content[r][c] == '?'
				|| content[r][c] == '^'
				|| content[r][c] == '|'
				|| content[r][c] == '~')
			{
				color[r][c] = Color_Symbol;
			}
			else if(content[r][c] == '(' || content[r][c] == ')'
				|| content[r][c] == '[' || content[r][c] == ']'
				|| content[r][c] == '{' || content[r][c] == '}')
			{
				color[r][c] = Color_Bracket;
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
					|| content[r][c-1] == '_'));
				else//数字前面不是字母或下划线
				{
					color[r][c] = Color_Number;
				}
			}
			if(content[r][c] == '.' && c > 0 && c+1 < numberOfColumn[r])//小数点
			{
				if(content[r][c-1] >= '0' && content[r][c-1] <= '9'
					&& content[r][c+1] >= '0' && content[r][c+1] <= '9')
				{
					color[r][c] = Color_Number;
				}
			}
			if(content[r][c] == '0' && c+1 < numberOfColumn[r] && content[r][c+1] == 'x')//十六进制数
			{
				if(c+2 < numberOfColumn[r])
				{
					if((content[r][c+2] >= '0' && content[r][c+2] <= '9')
						|| (content[r][c+2] >= 'A' && content[r][c+2] <= 'F')
						|| (content[r][c+2] >= 'a' && content[r][c+2] <= 'f'))
					{
						color[r][c+1] = Color_Number;
						for(c+=2; c<numberOfColumn[r]; c++)
						{
							if((content[r][c] >= '0' && content[r][c] <= '9')
								|| (content[r][c] >= 'A' && content[r][c] <= 'F')
								|| (content[r][c] >= 'a' && content[r][c] <= 'f'))
							{
								color[r][c] = Color_Number;
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
				if(content[r][c] == '<') break;
				color[r][c] = Color_Preprocessor;
			}
		}//行首为#且在<前的内容
	}
	//注释着色
	commentStart = 0;
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]-1; c++)
		{
			if(content[r][c] == '/' && content[r][c+1] == '*')
			{
				commentStart = 1;
			}
			if(commentStart == 1)
			{
				color[r][c] = Color_Comment;
			}
			if(content[r][c] == '*' && content[r][c+1] == '/')
			{
				commentStart = 0;
				color[r][c+1] = Color_Comment;
			}
		}
		for(c=0; c<numberOfColumn[r]-1; c++)
		{
			if(content[r][c] == '/' && content[r][c+1] == '/')
			{
				for(; c<numberOfColumn[r]; c++)
				{
					color[r][c] = Color_Comment;
				}
			}
		}
	}
	//字符和字符串着色
	for(r=0; r<numberOfRow; r++)
	{
		/*characterStart = 0;
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '"' && characterStart == 1) characterStart = 0;
			if(characterStart == 1)
			{
				color[r][c] = Color_Character;
			}
			if(content[r][c] == '\'')
			{
				characterStart = 1-characterStart;
				color[r][c] = Color_Character;
			}
		}//字符为’后内容，遇到'或"则停止
		stringStart = 0;
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(stringStart == 1)
			{
				color[r][c] = Color_String;
			}
			if(content[r][c] == '"')
			{
				stringStart = 1-stringStart;
				color[r][c] = Color_String;
			}
		}//字符串为"后的内容，再次遇到"停止，字符串覆盖字符*/
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '\'')
			{
				end = FindMatch(r, c, '\'');
				if(end != -1)
				{
					DrawColorInRange(r, c, r, end, Color_Character);
					c = end;
				}
			}
		}
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '"')
			{
				end = FindMatch(r, c, '"');
				if(end != -1)
				{
					DrawColorInRange(r, c, r, end, Color_String);
					c = end;
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
			else if(color[r][c] == Color_Default)//仅搜索未着色部分
			{
				end = FindReservedWord(r, c);
				if(end > 0)
				{
					DrawColorInRange(r, c, r, c+end-1, Color_ReservedWord);
				}
			}
		}
	}
	//函数着色
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]-1; c++)
		{
			if(color[r][c] == Color_Default && content[r][c+1] == '(')
			{
				for(end=c; c>=0; c--)//(前为函数
				{
					if((content[r][c] >= '0' && content[r][c] <= '9')
						|| (content[r][c] >= 'A' && content[r][c] <= 'Z')
						|| (content[r][c] >= 'a' && content[r][c] <= 'z')
						|| content[r][c] == '_')
					{
						color[r][c] = Color_Function;
					}
					else
					{
						break;
					}
				}
				c = end;//防止死循环
			}
		}
	}
	//剩余内容均视为变量
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(color[r][c] == Color_Default)
			{
				color[r][c] = Color_Variable;
			}
		}
	}
}

void PrintContent()
{
	int r, c;
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			if(content[r][c] == '\t')
			{
				//printf("    ");
				ColorChar(':', Color_Bracket);
				printf("   ");
			}
			/*else if((r+1)%4 == 0 && content[r][c] == ' ')
			{
				printf("|");
			}*/
			else
			{
				//putchar(content[r][c]);
				ColorChar(content[r][c], color[r][c]);
			}
		}
	}
}

void ReadContent(char* fileName)
{
	FILE* file;
	char ch;
	int r, c;
	//int cn = 0;
	if(file = fopen(fileName, "r"))
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
		//numberOfRow = 1;//EOF也有一行
		numberOfRow = 0;
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
		//PrintContent();
	}
	else
	{
		printf("[Error]Can't find file %s\n", fileName);
	}
	fclose(file);
}

void WriteContent(char* fileName)
{
	FILE* file = fopen(fileName, "w");
	int r, c;
	for(r=0; r<numberOfRow; r++)
	{
		for(c=0; c<numberOfColumn[r]; c++)
		{
			fputc(content[r][c], file);
		}
	}
	fclose(file);
}

int main()
{
	char* fileName = InputFileName();
	//char* fileName = "Text Editer.c";
	int r;
	ReadContent(fileName);
	system("chcp 65001");//以UTF-8编码显示
	while(1)
	{
		AnalysisColor();
		system("cls");
		PrintContent();
		getchar();
		for(r=0; r<numberOfRow; r++)
		{
			free(color[r]);
		}
		free(color);
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
--------------------------------*/
