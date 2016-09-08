#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 外层变量访问：SearchTable 增加向外层搜索
// 符号表记录 本层的function，procedure及他们的变量， 注意弹栈问题

#define SORLEN 20    // 源程序名长度
#define TOKLEN 11    // 标识符、数字最大10 位
#define KEYNUM 39     // keyword 数量
#define CHNUM 37    // 英文字母加一位数字的数量，trie树用
#define STACKSIZE 100    // 堆栈大小，---------记得调成适当的---------
#define STATENUM 993    // 状态数---------记得调成适当的---------
#define NONTERNUM 105    // 非终结符号数--------记得调成适当的---------
#define CODELEN 100    // 一条中间代码的长度--------记得调成适当的---------
#define TEMINPLAC 3    // 临时变量池大小的 位数 （比如大小 100，则位数为3）
#define BASDIMEN 2    // 数组类型默认维数，一般不超过2维，超过再分配
#define LISTLEN 10
#define KINDLEN 5
#define ERRLEN 50

#define AND 1
#define ARRAY 2
#define BEGIN 3
#define CASE 4
#define CONST 5
#define DIV 6
#define DO 7
#define DOWNTO 8
#define ELSE 9
#define END 10
#define PFILE 11
#define FOR 12
#define FUNC 13
#define GOTO 14
#define IF 15
#define IN 16
#define LABEL 17
#define MOD 18
#define NIL 19
#define NOT 20
#define OF 21
#define OR 22
#define PACKED 23
#define PROC 24
#define PROG 25
#define READ 26    // 语法分析阶段新加入
#define READLINE 27    // 语法分析阶段新加入
#define RECORD 28
#define REPEAT 29
#define SET 30
#define THEN 31
#define TO 32
#define TYPE 33
#define UNTIL 34
#define VAR 35
#define WHILE 36
#define WITH 37
#define WRITE 38    // 语法分析阶段新加入
#define WRITELN 39    // 语法分析阶段新加入
#define ID 40
#define INT 41
#define REAL 42
#define STRING 43
#define PLUS 44
#define MINUS 45
#define MULTI 46
#define RDIV 47
#define EQ 48
#define LT 49
#define GT 50
#define LE 51
#define GE 52
#define NE 53
#define LR_BRAC 54
#define RR_BRAC 55
#define COMMA 56
// #define P_MARK 53 删去
#define F_STOP 57
#define RANGE 58
#define COLON 59
#define ASSIGN 60
#define SEMIC 61
#define CAP 62
#define EXP 63
#define LS_BRAC 64
#define RS_BRAC 65
#define Q_MARK 66    // 最大值还代表终结符号数
#define EREAL 67    // 科学记数法 暂不实现 ------------------ 测试程序中别出现别出现！！！----------------------------//
#define ENDMARK 67    // 对应分析表中的$

typedef struct TrieNode    // trie树节点：构建ID的trie树用
{
    char letter;    // 节点字符，打印检查用
    struct TrieNode *next[CHNUM];    // 节点指针
    int position;    // 符号表中位置
}TrieNode;

typedef struct ExtNode    // 符号表结点的扩展属性：数组型,function,procedure使用
{
    int dimen;    // 数组:维数; func/proc:参数个数
    int *dlen;    // 数组:各维长度; func/proc:他们的符号表在外层符号表的存储位置
}ExtNode;

typedef struct IDNode    // 符号表结点
{
    char id[TOKLEN];    // 标识符
    char kind[KINDLEN];    // 符号种类[常数：const，变量：var，过程：proc，函数：func，参数：para]
    char value[TOKLEN];    // 值
    char type[TOKLEN];    // 类型
    int offset;    // 偏移
    ExtNode *extend;    // 扩展域
}IDNode;

typedef struct TableHead
{
    char name[TOKLEN];
    TrieNode *trihead;    // 该表对应的trie树（一表对应一个trie树）
    int offset, insize, nextin;    // insize:insize:内层符号表数量,可动态扩展;nextin:为下一个在inner中的下标
    struct TableHead *outer;
    struct TableHead **inner;    // 配合insize动态扩充，初始为10
}TableHead;

typedef struct ParStack    //语法分析栈
{
    int stack[STACKSIZE];
    int top;    // 栈顶
}ParStack;

typedef struct SemStack    // 语义栈
{
    int offset[STACKSIZE];
    TableHead *tblhead[STACKSIZE];
    int top;    // 栈顶
}SemStack;

typedef struct StrStack    // token 栈，语义分析用
{
    char stack[STACKSIZE][TOKLEN];
    int top;    // 栈顶
}StrStack;

typedef struct TypNode    // 基本类型表的表项
{
    char name[TOKLEN];    // 名字
    int width;    // 偏移
}TypNode;

typedef struct TMP    // 临时变量
{
    int index;    // index和addr两个配合相当于书上的addr，当index=-1，未申请临时变量，id型归约成非终结符
    char addr[TOKLEN];
}TMP;

typedef struct TmpStack    // 临时变量栈
{
    TMP stack[STACKSIZE];
    int top;
}TmpStack;

typedef struct SETVar   // 数组型变量
{
    char base[TOKLEN];    // 书中array, 数组型变量的名称
    int fixepart, width, ndim;    // 基偏移， 宽度 和 维数
    int *dlen;    // 各维长度
}SETVar;

typedef struct TupleF    // 中间代码四元组    //--------------- 各个域长度记得修改----------------------------//
{
    char oprnt[TOKLEN], opr1[3 * TOKLEN], opr2[3 * TOKLEN], result[3 * TOKLEN];
}TupleF;

typedef struct ListStack
{
    int stack[STACKSIZE][LISTLEN];
    int top;
}ListStack;

int Scanner(char source[], IDNode idtable[]);    // 根据单词首字母调用函数获取单词
int GetToken(FILE *fs, char cho, char token[], FILE *fd, FILE *fe);    // 根据已扫描的首字母继续分类扫描
int GetDigit(FILE *fs, char cho, char token[], FILE *fd);    // 识别数字
int GetString(FILE *fs, char cho, char token[], FILE *fd);    // 识别字符串
int GetID(FILE *fs, char cho, char token[], FILE *fd);    // 识别标识符或者关键字
int IsKeyW(char ch[]);    // 判断是否为保留字
void InitTblH(TableHead *outer, TableHead *newtbl, char name[]);    // 初始化符号表，trie树
int UpdateTable(char token[], TrieNode *trihead, IDNode *idtable);    // 更新符号表
int SearchTable(char setname[], TableHead *tbhead);    // 查符号表
void InitStack(ParStack *cstack, ParStack *sstack, SemStack *tnfstack);    // 初始化栈
void PushParStack(ParStack *cstack, ParStack *sstack, int tokenum, int state);    // 压栈，状态栈和符号栈
void PopParStack(ParStack *cstack, ParStack *sstack);    // 弹栈，状态栈和符号栈
void PushSemstack(SemStack *tnfstack, TableHead *tblhead);    // 压栈,符号表和偏移栈
//int UpdateTyp(TypNode typtbl[], char token[], int amount);    // 更新类型表
int GetType(TypNode typtbl[], char token[]);    // 获取类型在类型表中的位置
void PopTokenstack(StrStack *tokenstack);    // 弹栈，id栈，辅助语义分析用，压栈简单未设函数
void GenCode(char result[], char oprnt[], char opr1[], char opr2[]);    // 生成中间代码
int NewTemp();    // 获取新的临时变量
void PushTmpstack(int index, char addr[TOKLEN]);    // 压栈， 临时变量栈
void PopTmpstack();    // 弹栈， 临时变量栈
void PushListstack(ListStack *onelist,int *list);    // 压栈， list栈，弹栈简单，不需要设函数
void Backpach(int list[], int mq);    // 回填
void Merge(int *onelist, int *twolist, int relist[]);    // 合并
void Parser(int tokenum, char token[], IDNode idtable[], TypNode typtbl[],
           ParStack *sstack, ParStack *cstack, SemStack *tnfstack,
           int gototable[][NONTERNUM], int action[][ENDMARK], FILE *fe);    // 语法分析
void PrintToken(TrieNode *trihead, int index, char oneword[]);    // 打印单词，辅助字符表打印
void PrintTable(TrieNode *trihead);    // 打印某个小符号表
void printTables(TableHead *tmp);    // 打印所有小符号表，主要用来显示嵌套关系
void Output(int code, char token[], int position, FILE *fd);    // 输出到文件和屏幕
void Error(FILE *fe, int row, char message[]);    // 打印错误消息

int row = 1, tablei = 0, typti = 0;    // row：行数，记录符号表使用的最大下标，tablei:记录符号表下一个空闲位置，typti:记录类型表下一个空闲位置, countidlis:记录id栈中
int countidlis = 0;    // 记录idlist长度，主要用于变量声明，一系列同类型变量
int idtabsize = 500, typtblsize = 50;    // idtabsize：符号表大小（动态变化）, typtblsize: 类型表大小（动态变化）
int codenum = 200, nextcode = 0;    // 分别为中间代码数量和下一条代码位置
int tnum = 50, *temps, maxt = -1;    // 临时变量池及其大小, 目前分配出的最大池号
int porfh, fun_or_proc = 0;    // porfh:标记procedure 或者 function 在外层头部的位置
int deffunc = 0, defproc = 0;    // 过程或者函数 在定义还是在调用的标志。0:定义;1:调用;
int countpar = 0, arryflag = 0, casenext = -1;    // countpar:记录输入/输出语句参数数量;arryflag:标记一个数组的规约
TableHead *entridtbl = NULL;    // 符号表入口，指向主程序符号表
//char **mcodes;    // 存储中间代码
TupleF *mcodes;    // 存储中间代码
ExtNode *settype = NULL;    // 声明时的数组类型
SETVar setypevar;    // 数组型临时变量
char keywords[][10] = {"and","array","begin","case","const","div","do","downto","else","end",
                        "file","for","function","goto","if","in","label","mod","nil","not","of",
                        "or","packed","procedure","program","read","readln","record","repeat",
                        "set","then","to","type","until","var","while","with","write","writeln"};
StrStack idstack, constack, oprstack, tmpoprstack;    // token 栈，语义分析用(ID, constant, operator),tmpoprstack: 临时存储运算符，优先级原因
TmpStack tmstack;    // 临时变量栈，语义分析用
ListStack truelist, falselist, nextlist;    // 链表，语义分析用
ParStack Mquad, Magain, for_down, caselist;    // for_down: 区分for中的to 和 downto
TMP casecore;    // case 比较的核心

int main()
{
    int i, j;
    char source[SORLEN];
    IDNode idtable[idtabsize];    // 固定，不够再分配

    printf("Source program:");
    scanf("%s",source);

    Scanner(source, idtable);    // 扫描器
    printf("idtable:\n");
    for(i = 0; i < tablei; i++)    // 打印符号表
    {
        printf("idtable[%d]:%s\n",i, idtable[i].id);
        printf("\t kind:%s\n", idtable[i].kind);
        printf("\t type:%s\n", idtable[i].type);
        printf("\t value:%s\n",idtable[i].value);
        printf("\t offset:%d\n",idtable[i].offset);
        if(idtable[i].extend)
        {
            printf("\t dimension:%d\n", idtable[i].extend->dimen);
            if(!strcmp(idtable[i].kind, "proc") || !strcmp(idtable[i].kind, "func"))
            {
                printf("\t\t dimension[0] : %d\n", idtable[i].extend->dlen[0]);
            }
            else
            {
                for(j = 0; j < idtable[i].extend->dimen; j ++)
                {
                    printf("\t\t dimension[%d] : %d\n",j, idtable[i].extend->dlen[j]);
                }
            }
        }
        else
        {
            printf("\t dimension:0\n");
        }
    }
    printf("mcodes:\n");
    for(i = 0; i < nextcode; i ++)    // 打印中间代码
    {
        if(!strcmp(":=", mcodes[i].oprnt))
        {
            printf("%d: %s %s %s\n",i, mcodes[i].result, mcodes[i].oprnt, mcodes[i].opr1);
        }
        else if(!strcmp("if", mcodes[i].oprnt))
        {
            printf("%d: %s %s %s %s\n", i, mcodes[i].oprnt, mcodes[i].opr1, mcodes[i].opr2, mcodes[i].result);
        }
        else if(!strcmp("goto", mcodes[i].oprnt) || !strcmp("param", mcodes[i].oprnt) || !strcmp("call", mcodes[i].oprnt))
        {
            printf("%d: %s %s %s\n",i, mcodes[i].oprnt, mcodes[i].result, mcodes[i].opr1);
        }
        else if(!strcmp("ret", mcodes[i].oprnt) || !strcmp("end", mcodes[i].oprnt))
        {
            printf("%d: %s\n", i, mcodes[i].oprnt);
        }
        else
        {
            printf("%d: %s := %s %s %s\n",i, mcodes[i].result, mcodes[i].opr1, mcodes[i].oprnt, mcodes[i].opr2);
        }
    }
    return 0;
}

int Scanner(char source[], IDNode idtable[])    // 根据单词首字母调用函数获取单词
{
    char token[TOKLEN]={'\0'}, ch;
    int tokenum, i, j, actitem, action[STATENUM][ENDMARK], goitem, gototable[STATENUM][NONTERNUM];    // next:标识是否读下一个字符，prorlset：从字符串中拆分出的产生式的每个右部的终结非终结符数
    FILE *fs = NULL, *fd = NULL, *fe = NULL, *fa = NULL, *fg = NULL, *ft = NULL;    // 各种文件指针
    TypNode typtbl[typtblsize];
    ParStack sstack, cstack;    // 状态和token栈
    SemStack tnfstack;    // 符号表和offset的栈
    /*打开相关文件*/
    if((fs = fopen(source,"r")) == NULL)    // 打开源程序
    {
        printf("Fail to open source program %s!\n",source);
    }
    if((fd = fopen("firststep.txt","w")) == NULL)    // 打开词汇分析输出文件
    {
        printf("Fail to open destination file firststep.txt!\n");
        exit(0);
    }
    if((fa = fopen("action.txt","r")) == NULL)    // 打开action 表文件
    {
        printf("Fail to open the file action.txt!\n");
        exit(0);
    }
    if((fg = fopen("goto.txt","r")) == NULL)    // 打开goto 表文件
    {
        printf("Fail to open the file goto.txt!\n");
        exit(0);
    }
    if((ft = fopen("type.txt","r")) == NULL)    // 打开错误输出文件
    {
        printf("Fail to open the file type.txt!\n");
        exit(0);
    }
    if((fe = fopen("error.txt","w")) == NULL)    // 打开错误输出文件
    {
        printf("Fail to open destination file firststep.txt!\n");
        exit(0);
    }
    /*读入action表*/
    while(!feof(fa))
    {
        fscanf(fa,"%d %d %d", &i, &j, &actitem);
        action[i][j] = actitem;
    }
    /*读入goto表*/
    while(!feof(fg))
    {
        fscanf(fg,"%d %d %d", &i, &j, &goitem);
        gototable[i][j] = goitem;
    }
    /*读入类型表*/
    typti = 0;
    while(!feof(ft))
    {
        fscanf(ft,"%s",typtbl[typti].name);
        fscanf(ft,"%s",token);
        typtbl[typti].width = atoi(token);
        typti ++;
    }
    /*初始化*/
    InitStack(&cstack, &sstack, &tnfstack);    // 初始化栈
    mcodes = (TupleF*)malloc(codenum * sizeof(TupleF));    // 初始化中间代码存储器
    temps = (int *)malloc(tnum * sizeof(int));    // 初始化临时变量池------------是否初始默认为0-----------------//
    settype = (ExtNode *)malloc(sizeof(ExtNode));
    settype->dimen = 0;
    settype->dlen = (int *)malloc(BASDIMEN * sizeof(int));
    /*读源文件文件*/
    while(!feof(fs))
    {
        ch = fgetc(fs);
        memset(token, '\0', TOKLEN);    // 清空原单词
        while(ch == ' ' || ch == '\t' || ch == '\n')
        {
            if(ch == '\n')
            {
                row ++;
            }
            ch = fgetc(fs);
        }
        if (!feof(fs))
        {
            tokenum = GetToken(fs, ch, token, fd, fe);    // 词法分析器
            //------以下添加语法分析器-----//
            if(tokenum != 0)
            {
                Parser(tokenum, token, idtable, typtbl, &sstack, &cstack, &tnfstack, gototable, action, fe);
            }
        }
    }
    Parser(ENDMARK, token, idtable, typtbl, &sstack, &cstack, &tnfstack, gototable, action, fe);

    fclose(fs);
    fclose(fd);
    fclose(ft);
    fclose(fe);
    return 0;
}

int GetToken(FILE *fs, char cho, char token[], FILE *fd, FILE *fe)    // 根据已扫描的首字母继续分类扫描
{
    char ch = cho;

    if(isdigit(ch))
    {
        return GetDigit(fs, ch, token, fd);    // 识别数字
    }
    else if(ch == '\'')
    {
        return GetString(fs, ch, token, fd);    // 识别字符串
    }
    else if(isalpha(ch) || ch == '_')
    {
        return GetID(fs, ch, token, fd);    // 识别 ID 或者关键字
    }
    else
    {
        switch (ch)
        {    // case后面的break 也都省掉了，yinweireturn的存在...
        case '=':
            token[0] = ch;
            printf("row %d:",row);
            Output(EQ, token, 0, fd);
            return EQ;
        case ';':
            token[0] = ch;
            printf("row %d:",row);
            Output(SEMIC, token, 0, fd);
            return SEMIC;
        case '+':
            token[0] = ch;
            printf("row %d:",row);
            Output(PLUS, token, 0, fd);
            return PLUS;
        case '-':
            token[0] = ch;
            printf("row %d:",row);
            Output(MINUS, token, 0, fd);
            return MINUS;
        case '*':
            token[0] = ch;
            ch = fgetc(fs);
            if(ch == '*')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(EXP, token, 0, fd);
                return EXP;
            }
            else
            {
                printf("row %d:",row);
                Output(MULTI, token, 0, fd);
                fseek(fs,-1,SEEK_CUR);
                return MULTI;
            }
        case '/':
            token[0] = ch;
            printf("row %d:",row);
            Output(RDIV, token, 0, fd);
            return RDIV;
        case '<':
            token[0] = ch;
            ch = fgetc(fs);
            if(ch == '=')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(LE, token, 0, fd);
                return LE;
            }
            else if(ch == '>')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(NE, token, 0, fd);
                return NE;
            }
            else
            {
                printf("row %d:",row);
                Output(LT, token, 0, fd);
                fseek(fs,-1,SEEK_CUR);
                return LT;
            }
        case '>':
            token[0] = ch;
            ch = fgetc(fs);
            if(ch == '=')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(GE, token, 0, fd);
                return GE;
            }
            else
            {
                printf("row %d:",row);
                Output(GT, token, 0, fd);
                fseek(fs,-1,SEEK_CUR);
                return GT;
            }
        case '(':
            token[0] = ch;
            printf("row %d:",row);
            Output(LR_BRAC, token, 0, fd);
            return LR_BRAC;
        case ')':
            token[0] = ch;
            printf("row %d:",row);
            Output(RR_BRAC, token, 0, fd);
            return RR_BRAC;
        case ',':
            token[0] = ch;
            printf("row %d:",row);
            Output(COMMA, token, 0, fd);
            return COMMA;
        case '.':
            token[0] = ch;
            ch = fgetc(fs);
            if(ch == '.')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(RANGE, token, 0, fd);
                return RANGE;
            }
            else
            {
                printf("row %d:",row);
                Output(F_STOP, token, 0, fd);
                if(!feof(fs))    // 只此一处，因为PASCAL 末尾以‘end.’结束
                {
                    fseek(fs,-1,SEEK_CUR);
                }
                return F_STOP;
            }
        case ':':
            token[0] = ch;
            ch = fgetc(fs);
            if(ch == '=')
            {
                token[1] = ch;
                printf("row %d:",row);
                Output(ASSIGN, token, 0, fd);
                return ASSIGN;
            }
            else
            {
                printf("row %d:",row);
                Output(COLON, token, 0, fd);
                fseek(fs,-1,SEEK_CUR);
                return COLON;
            }
        case '^':
            token[0] = ch;
            printf("row %d:",row);
            Output(CAP, token, 0, fd);
            return CAP;
        case '[':
            token[0] = ch;
            printf("row %d:",row);
            Output(LS_BRAC, token, 0, fd);
            return LS_BRAC;
        case ']':
            token[0] = ch;
            printf("row %d:",row);
            Output(RS_BRAC, token, 0, fd);
            return RS_BRAC;
        case '{':    // 注释
            ch = fgetc(fs);
            if(ch == '*')    // 多行注释
            {
                while(1)
                {
                    ch = fgetc(fs);
                    if(ch == '\n')
                    {
                        row ++;
                    }
                    else if(ch == '*')
                    {
                        ch = fgetc(fs);
                        if(ch == '}')
                        {
                            break;    // 注释结束
                        }
                        else
                        {
                            fseek(fs,-1,SEEK_CUR);
                        }
                    }
                }
            }
            else    // 单行注释
            {
                fseek(fs,-1,SEEK_CUR);
                while(ch != '}')
                {
                    if(ch == '\n')
                    {
                        row ++;
                        Error(fe, row, "New line is not allowed in one-line comment.");
                    }
                  ch = fgetc(fs);
                }
            }
            break;
        default:
            Error(fe, row, "Invalid character.");
            return -1;    // ------------------------注意出错处理----------------------------//
        }
    }
    return 0;    // 只有注释才会执行到这
}

int GetDigit(FILE *fs, char cho, char token[], FILE *fd)    // 识别数字
{
    char ch = cho;
    int index = 0,state = 0, cut = 0;

    while(1)
    {
        switch(state)
        {
        case 0:
            if(ch == '0')
            {
                state = 2;
            }
            else
            {
                state = 1;
            }
            token[index] = ch;
            index ++;
            ch = fgetc(fs);
            break;
        case 1:
            if(isdigit(ch))
            {
                state = 1;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else if(ch == '.')
            {
                state = 3;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else if(ch == 'E')
            {
                state = 5;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else
            {
                if(!feof(fs))
                {
                    fseek(fs,-1,SEEK_CUR);
                }
                printf("row %d:",row);
                Output(INT, token, 0, fd);    // 识别出整数:非0,该状态只能识别出整数
                return INT;
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
            }
            break;
        case 2:
            if(ch == '.')
            {
                state = 3;
                token[index] = ch;
                index ++;
                ch = fgetc(fs);
            }
            else
            {
                if(!feof(fs))
                {
                    fseek(fs,-1,SEEK_CUR);
                }
                printf("row %d:",row);
                Output(INT, token, 0, fd);    // 识别出整数:0
                return INT;
            }
            break;
        case 3:
            if(isdigit(ch))
            {
                state = 4;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
                token[index-2] = '\0';// 截到‘.’之前
            }
            break;
        case 4:
            if(isdigit(ch))
            {
                state = 4;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else if(ch == 'E')
            {
                state = 5;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else
            {
                if(!feof(fs))
                {
                    fseek(fs, -1, SEEK_CUR);
                }
                printf("row %d:",row);
                Output(REAL, token, 0, fd);    // 识别出实数,该状态只能识别出非科学记数法实数
                return REAL;
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
            }
            break;
        case 5:
            if(ch >= '1' && ch <= '9')
            {
                state = 7;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else if(ch == '+' || ch == '-')
            {
                state = 6;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
                token[index-2] = '\0';    // 截到 ‘E’之前
            }
            break;
        case 6:
            if(ch >= '1' && ch <= '9')
            {
                state = 7;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
                token[index-3] = '\0';    // 截到'E'之前
            }
            break;
        case 7:
            if(isdigit(ch))
            {
                state = 7;
                if(!cut)
                {
                    token[index] = ch;
                    index ++;
                }
                ch = fgetc(fs);
            }
            else
            {
                if(!feof(fs))
                {
                    fseek(fs, -1, SEEK_CUR);
                }
                printf("row %d:",row);
                Output(EREAL, token, 0, fd);    // 识别出科学记数法形式实数,该状态只能识别出该种数字
                return EREAL;    //---------科学记数法-----有空再填-------//
            }
            if(!cut && index > TOKLEN - 2)    // 截断
            {
                cut = 1;
            }
            break;
        default:
            printf("GetDigit break down, please contact Song Jiafei!");
            exit(0);
        }
    }
}

int GetString(FILE *fs, char cho, char token[], FILE *fd)    // 识别字符串
{
    char ch = cho;
    int index = 0, cut = 0;

    token[index] = '\'';
    index ++;
    ch = fgetc(fs);
    while(ch != '\'')
    {
        if(index <= TOKLEN - 2)    // 过长则忽略
        {
            token[index] = ch;
            index ++;
        }
        else
        {
            cut = 1;
        }
        ch = fgetc(fs);
    }
    if(!cut)
    {
        token[index] = ch;
    }
    else
    {
        token[index - 1] = '\'';
    }
    printf("row %d:",row);
    Output(STRING, token, 0, fd);    // 识别出字符串
    return STRING;
}

int GetID(FILE *fs, char cho, char token[], FILE *fd)
{
    char ch = cho;
    int index = 0, cut = 0, iskey;

    if(isalpha(ch))
    {
        ch = tolower(ch);
    }
    token[index] = ch;
    index ++;
    ch = fgetc(fs);
    while(1)
    {
        if(isalpha(ch) || ch == '_' || isdigit(ch))
        {
            if(isalpha(ch))
            {
                ch = tolower(ch);
            }
            if(!cut)
            {
                token[index] = ch;
                index ++;
            }
            ch = fgetc(fs);
        }
        else
        {
            if(!feof(fs))
            {
                fseek(fs, -1, SEEK_CUR);
            }
            iskey = IsKeyW(token);
            if(iskey)
            {
                printf("row %d:",row);
                Output(iskey, token, 0, fd);
                return iskey;
            }
            else
            {
//                position = UpdateTable(token, trihead, idtable);
                printf("row %d:",row);
                Output(ID, token, -1, fd);
                return ID;
            }
            // 本来该语句块后面为 break， 但是再修改过程中语句块多了return，故不用break
        }
        if(index > TOKLEN - 2)    // 截断
        {
            cut = 1;
            token[TOKLEN - 1] = '\0';
            // 此时一定不是关键字，因为关键字不会被截断....
        }
    }
    return 0;    // 也不会被执行到
}

int IsKeyW(char chs[])    // 判断是否为保留字
{
    int low = 0, mid , high = KEYNUM - 1;    // 采用二分查找

    if(strlen(chs) > 9)
    {
        return 0;
    }

    mid = (low + high)/2;
    while(low < high && strcasecmp(keywords[mid],chs))    // PACAL 不区分大小写
    {
        if(strcasecmp(keywords[mid],chs) < 0)
        {
            low = mid + 1;
        }
        else if(strcasecmp(keywords[mid],chs) > 0)
        {
            high = mid - 1;
        }
        mid = (low + high)/2;
    }
    if(!strcasecmp(keywords[mid],chs))    // 即 strcasecmp 返回0，即在关键字表中找到刚刚识别出的ID
    {
        if(mid == 23)
        {
            defproc = 1;
        }
        else if(mid == 12)
        {
            deffunc = 1;
        }
        return (mid + 1);
    }
    return 0;
}

void InitTblH(TableHead *outer, TableHead *newtbl, char name[])    // 初始化符号表表头
{
    int i;
    strcpy(newtbl->name, name);
    newtbl->outer = outer;
    newtbl->insize = 10;
    newtbl->inner = (TableHead **)malloc(sizeof(TableHead *) * 10);
    for(i = 0; i < 10; i ++)
    {
        newtbl->inner[i] = (TableHead *)malloc(sizeof(TableHead));
    }
    newtbl->nextin = 0;
    newtbl->trihead = (TrieNode *)malloc(sizeof(TrieNode));
    (newtbl->trihead)->letter = '\n';
    (newtbl->trihead)->position = -1;
    for(i = 0; i < CHNUM;i ++)
    {
        (newtbl->trihead)->next[i] = NULL;
    }
}

int UpdateTable(char token[], TrieNode *trihead, IDNode *idtable)    // 更新符号表
{
    TrieNode *tmp = trihead;
    int i,j,in;
    for(i = 0; token[i] != '\0'; i++)
    {
        if(isalpha(token[i]))
        {
            in = (int)token[i] - 87;    // PASCAL 不分大小写
        }
        else if(isdigit(token[i]))
        {
            in = (int)token[i] - 48;
        }
        else if(token[i] == '_')
        {
            in = CHNUM - 1;
        }
        if(tmp->next[in] == NULL)
        {
            TrieNode *newnode =  (TrieNode *)malloc(sizeof(TrieNode));    // 动态内存分配，新trie树结点
            for(j = 0; j < CHNUM; j++)
            {
                newnode->next[j] = NULL;    // 无子树
            }
            newnode->letter = token[i];
            newnode->position = -1;
            tmp->next[in] = newnode;
        }
        tmp = tmp->next[in];
    }
    if(tmp->position == -1)
    {
        tmp->position = tablei;    // 符号表中存储该ID 的下标
        strcpy(idtable[tablei].id, token);
        idtable[tablei].extend = NULL;
        tablei ++;    // 下一个 ID 的存储位置
        if(tablei >= idtabsize)    // 扩充符号表
        {
            idtabsize *= 2;
            IDNode *newtable = (IDNode *)malloc(idtabsize * sizeof(IDNode));
            if(newtable == NULL)
            {
                printf("IDTable overflow!\n");
                exit(0);
            }
            for(i = 0; i < tablei / 2; i ++)
            {
                newtable[i] = idtable[i];
            }
            free(idtable);
            idtable = newtable;
        }
        return tablei - 1;
    }
    return tmp->position;    // 表中已经存在
}

int SearchTable(char setname[], TableHead *tbhead)    // 查符号表
{
    TrieNode *tmp = NULL;
    int i, in;
    if(!tbhead)
    {
        return -1;
    }
    tmp = tbhead -> trihead;
    for(i = 0; setname[i] != '\0'; i++)
    {
        if(isalpha(setname[i]))
        {
            in = (int)setname[i] - 87;    // PASCAL 不分大小写
        }
        else if(isdigit(setname[i]))
        {
            in = (int)setname[i] - 48;
        }
        else if(setname[i] == '_')
        {
            in = CHNUM - 1;
        }
        if(tmp->next[in] == NULL)
        {
            return SearchTable(setname, tbhead->outer);
        }
        tmp = tmp->next[in];
    }
    if(tmp->position == -1)
    {
        return SearchTable(setname, tbhead->outer);
    }

    return tmp->position;
}

void InitStack(ParStack *cstack, ParStack *sstack, SemStack *tnfstack)    // 初始化各种栈
{
    /*状态栈和符号栈*/
    cstack->stack[0] = ENDMARK;
    sstack->stack[0] = 0;
    cstack->top = 0;
    sstack->top = 0;
    /*符号表和偏移栈*/
    tnfstack->top = -1;
    /*id栈*/
    idstack.top = -1;
    /*constant 栈*/
    constack.top = -1;
    /*operator 栈*/
    oprstack.top = -1;
    tmpoprstack.top = -1;
    /*临时变量 栈*/
    tmstack.top = -1;
    /*list 栈*/
    truelist.top = -1;    // truelist
    memset(truelist.stack, -1, sizeof(truelist.stack));
    falselist.top = -1;    // falselist
    memset(falselist.stack, -1, sizeof(falselist.stack));
    nextlist.top = -1;    // nextlist
    memset(nextlist.stack, -1, sizeof(nextlist.stack));
    /*Mquad 栈*/
    Mquad.top = -1;
    /*again 栈*/
    Magain.top = -1;
    /*for_down 栈*/
    for_down.top = -1;    // 之所以用栈是因为可能出现嵌套循环
    /*case栈*/
    caselist.top = -1;
    memset(caselist.stack, -1, sizeof(caselist.stack));
    return;
}

void PushParstack(ParStack *cstack, ParStack *sstack, int tokenum, int state)    // 压栈，状态栈和符号栈
{
    (*cstack).top ++;
    (*sstack).top ++;
    (*cstack).stack[(*cstack).top] = tokenum;
    (*sstack).stack[(*sstack).top] = state;
}

void PopParstack(ParStack *cstack, ParStack *sstack)    // 弹栈，状态栈和符号栈
{
    (*cstack).top --;    // 符号栈出栈
    (*sstack).top --;    // 状态栈出栈
}

void PushSemstack(SemStack *tnfstack, TableHead *tblhead)    // 压栈，符号表和偏移；弹栈简单未设函数
{
    tnfstack->top ++;
    tnfstack->tblhead[tnfstack->top] = tblhead;
    tnfstack->offset[tnfstack->top] = 0;
}

void PopTokenstack(StrStack *tokenstack)    // 弹栈，(id,constant,opratpr)，辅助语义分析用，压栈简单未设函数
{
    memset(tokenstack->stack[tokenstack->top], '\0', TOKLEN);    // 清空原位置
    tokenstack->top --;
    return;
}

//int UpdateTyp(TypNode typtbl[], char token[], int amount)    // 更新类型表
//{
//    int base;
//    strcpy(typtbl[typti].name, token);
//    typtbl[typti].amount = amount;
//    if(amount > 1)
//    {
//        base = GetType(typtbl, token);
//        if(base == -1)
//        {
//            printf("Cannot find base type:%s!\n",token);
//        }
//        typtbl[typti].width = amount * typtbl[base].width;
//    }
//    typti ++;
//    return typti - 1;
//}

int GetType(TypNode typtbl[], char token[])    // 获取类型在类型表中的位置
{
    int i = 0;
    while(i < typti)
    {
        if(strlen(typtbl[i].name) != strlen(token))
        {
            i ++;
            continue;
        }
        if(!strcasecmp(typtbl[i].name, token))    // 相同
        {
            return i;
        }
        i ++;
    }
    return -1;
}

void GenCode(char result[], char oprnt[], char opr1[], char opr2[])    // 生成中间代码
{
    int i;
    if(!result)
    {
        memset(mcodes[nextcode].result, '\0', TOKLEN);
    }
    else
    {
        strcpy(mcodes[nextcode].result, result);
    }
    if(!oprnt)
    {
        memset(mcodes[nextcode].oprnt, '\0', TOKLEN);
    }
    else
    {
        strcpy(mcodes[nextcode].oprnt, oprnt);
    }
    if(!opr1)
    {
        memset(mcodes[nextcode].opr1, '\0', TOKLEN);
    }
    else
    {
        strcpy(mcodes[nextcode].opr1, opr1);
    }
    if(!opr2)
    {
        memset(mcodes[nextcode].opr2, '\0', TOKLEN);
    }
    else
    {
        strcpy(mcodes[nextcode].opr2, opr2);
    }
    nextcode ++;
    if(nextcode  >= codenum)
    {
        codenum *= 2;
        TupleF *newcodes = (TupleF*)malloc(codenum * sizeof(TupleF));
        for(i = 0; i < codenum / 2; i ++)
        {
            if(!mcodes[i].result)
            {
                memset(newcodes[i].result, '\0', TOKLEN);
            }
            else
            {
                strcpy(newcodes[i].result, mcodes[i].result);
            }
            if(!mcodes[i].oprnt)
            {
                memset(newcodes[i].oprnt, '\0', TOKLEN);
            }
            else
            {
                strcpy(newcodes[i].oprnt, mcodes[i].oprnt);
            }
            if(!mcodes[i].opr1)
            {
                memset(newcodes[i].opr1, '\0', TOKLEN);
            }
            else
            {
                strcpy(newcodes[i].opr1, mcodes[i].opr1);
            }
            if(!mcodes[i].opr2)
            {
                memset(newcodes[i].opr2, '\0', TOKLEN);
            }
            else
            {
                strcpy(newcodes[i].opr2, mcodes[i].opr2);
            }
        }
        free(mcodes);
        mcodes = newcodes;
    }
    return;
}

int NewTemp()    // 获取新的临时变量
{
    int i;
    if(maxt == -1)
    {
        maxt ++;
        return 0;
    }
    else
    {
        for(i = 0;i <= maxt; i ++)
        {
            if(!temps[i])
            {
                temps[i] = 1;
                return i;
            }
        }
        maxt ++;
        if(maxt >= tnum)
        {
            tnum *= 2;
            int *newtemps = (int *)malloc(sizeof(int));
            for(i = 0;i <= tnum / 2; i ++)    // --------------int数组初始是否默认0-----------------------//
            {
                newtemps[i] = temps[i];
            }
            free(temps);
            temps = newtemps;
        }
        return maxt;
    }
//    maxt ++;
    return maxt;
}

void PushTmpstack(int index, char addr[TOKLEN])    // 压栈， 临时变量栈
{
    tmstack.top ++;
    tmstack.stack[tmstack.top].index = index;
    if(addr)
    {
        strcpy(tmstack.stack[tmstack.top].addr, addr);
    }
    return;
}

void PopTmpstack()    // 弹栈， 临时变量栈
{
    memset(tmstack.stack[tmstack.top].addr, '\0', TOKLEN);    // 清空原位置
    tmstack.top --;
    return;
}

void PushListstack(ListStack *onelist, int *list)    // 压栈， list栈
{
    int i = 0, tmp = 0;
    onelist->top ++;
    if(onelist->stack[onelist->top][i] != -1)
    {
//        memset(onelist->stack[onelist->top], -1, LISTLEN);    // 清空原来的数据，不然影响以后的
        for(i = 0;i<LISTLEN;i ++)
        {
            onelist->stack[onelist->top][i] = -1;
        }
    }
    i = 0;
    while(list[i] != -1)
    {
//        printf("i = %d\n", i);
        onelist->stack[onelist->top][i] = list[i];
        list[i] = -1;    // 清空原来的数据，不然影响以后的
        i ++;
    }
    return;
}

void Backpach(int list[], int mq)    // 回填
{
    int i = 0;
    char row[TOKLEN];
    itoa(mq, row, 10);
    while(list[i] != -1)
    {
        strcpy(mcodes[list[i]].result, row);
        list[i] = -1;    // 清空
        i ++;
    }
    return;
}

void Merge(int *onelist, int *twolist, int relist[])    // 合并
{
    int i = 0, j = 0, k, q, flag;
    while(onelist[i] != -1)
    {
        relist[i] = onelist[i];
        onelist[i] = -1;
        i ++;
    }
    q = i;
    while(twolist[j] != -1)
    {
        flag = 1;
        for(k = 0; k < i; k ++)
        {
            if(twolist[j] == relist[k])
            {
                flag = 0;
                break;
            }
        }
        if(flag)
        {
            relist[q] = twolist[j];
            q ++;
        }
        twolist[j] = -1;
        j ++;
    }
    for(;q < LISTLEN; q ++)
    {
        relist[q] = -1;
    }
//    printf("After merge: \n");
//    for(i = 0;i < LISTLEN;i ++)
//    {
//        printf("%d,", relist[i]);
//    }
    return;
}

void Parser(int tokenum, char token[], IDNode idtable[], TypNode typtbl[],
            ParStack *sstack, ParStack *cstack, SemStack *tnfstack,
            int gototable[][NONTERNUM], int action[][ENDMARK], FILE *fe)    // 语法分析
{
    int i, j, tops, next = 1, act,position, typos, amount;    // next标识是否读入这个token，若发生归约则未，下次还应再读
    int tmpi, mq1, mq2, width;    // tmpi:使用的临时变量的下标的整型表示, mqx: mquad临时存储
    int *tmplist1 = NULL, *tmplist2 = NULL, *tmplist3 = NULL;    // list临时存储区
    int tmplist4[LISTLEN], tmplist5[LISTLEN];    // list临时存储区, 使用之后需要还原， 不然可能影响之后的
    char tmpic[TEMINPLAC], tmpich[TEMINPLAC];    // (辅助中间代码生成)tmpic,tmpich:使用的临时变量的下标的字符串表示,
    char constyp[TOKLEN], type[TOKLEN], tmpopr[TOKLEN];    // constyp: constant 的类型，声明时用;tmpopr:临时存储运算符
    char result[TOKLEN], oprnt[TOKLEN], opr1[TOKLEN], opr2[TOKLEN]; // 中间代码：四元组元素
    char errmssg[ERRLEN];    // 错误信息
    TableHead *inhead, *outhead, *tmp;
    TMP tmpvarone, tmpvartwo, tmpvarthree;    // 暂存临时变量栈顶部的， 运算用

    memset(tmplist4, -1, sizeof(tmplist4));    // 初始化
    memset(tmplist5, -1, sizeof(tmplist5));    // 初始化


    if(!strcmp(token, "downto"))
    {
        for_down.top ++;
        for_down.stack[for_down.top] = 1;
    }
    else if((tokenum == ID && !IsKeyW(token)))    // 存放id，谨慎，影响新符号表建立时的名字
    {
        idstack.top ++;
        strcpy(idstack.stack[idstack.top], token);
    }
    else if(tokenum == REAL || tokenum == INT || tokenum == STRING || tokenum == NIL)    // 存放 (unsigned)constant 以及 constant的类型（隐式，推出）
    {
        constack.top ++;
        strcpy(constack.stack[constack.top], token);
    }
    else if((tokenum >= PLUS && tokenum <=NE) || tokenum == AND || tokenum == OR || tokenum == DIV || tokenum == MOD)    // 操作符
    {
        oprstack.top ++;
        strcpy(oprstack.stack[oprstack.top], token);
    }
    do
    {
        // ----------------------debug 用，记得删除 --------------------------------------//
//        for(i = 0; i < STACKSIZE; i ++)
//        {
//            for(j = 0; j < LISTLEN; j ++)
//            {
//                if(nextlist.stack[i][j]== 65535)
//                {
//                    printf("i=%d,j=%d", i, j);
//                    exit(0);
//                }
//            }
//        }
//        if(truelist.top < -1|| falselist.top < -1 || nextlist.top < -1)
//        {
//            if(truelist.top < -1)
//            {
//                printf("truelist\n");
//            }
//            if(falselist.top < -1)
//            {
//                printf("falselist\n");
//            }
//            if(nextlist.top < -1)
//            {
//                printf("nextlist\n");
//            }
//            exit(0);
//        }

//        if(nextcode == 43)
//        {
//            exit(0);
//        }
        // ---------------------------------------------------------------------------------//
        tops = (*sstack).stack[(*sstack).top];    // sstack的栈顶
        act = action[tops][tokenum];    // 查action表
        if(act > 0)   // shift
        {
            next = 1;
            if(act < STATENUM)
            {
                PushParstack(cstack, sstack, tokenum, act);
            }
        }
        else if(act < 0)    // reduce
        {
            next = 0;
            switch (act)
            {
            case -1:
                printf("<program>' => <program>;\n");    // 对应产生式
                break;
            case -2:
                /*语法分析*/
                printf("<program> => <program_heading> semi <main_idtable> <program_block>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 0, gototable[sstack->stack[sstack->top]][0]);    // 0:刚规约出来的非终结符代号
                /*语义分析*/
                (tnfstack->tblhead[tnfstack->top]) -> offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;
                printf("<program>' => <program>;\n");
                GenCode('\0', "end", '\0', '\0');
                break;
            case -3:
                printf("<program_heading> => program ID;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 1, gototable[(*sstack).stack[(*sstack).top]][1]);    // 1:刚规约出来的非终结符代号
                break;
            case -4:
                printf("<program_heading> => program ID ( <program_parameters> );\n");    // 对应产生式
                for(i = 5; i > 0; i --)    // 5:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 1, gototable[(*sstack).stack[(*sstack).top]][1]);    // 1:刚规约出来的非终结符代号
                break;
            case -5:
                /*语法分析*/
                printf("<program_parameters> => <identifier_list>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 2, gototable[(*sstack).stack[(*sstack).top]][2]);    // 2:刚规约出来的非终结符代号
                /*语义分析*/
                while(countidlis > 0)    // 无用，弹出
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -6:
                /*语法分析*/
                printf("<identifier_list> => <identifier_list> , ID;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 3, gototable[(*sstack).stack[(*sstack).top]][3]);    // 3:刚规约出来的非终结符代号
                /*语义分析*/
                countidlis ++;    // 记录idlist长度，主要用于变量声明，一系列同类型变量
                break;
            case -7:
                /*语法分析*/
                printf("<identifier_list> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 3, gototable[(*sstack).stack[(*sstack).top]][3]);    // 3:刚规约出来的非终结符代号
                /*语义分析*/
                countidlis ++;
                break;
            case -8:
                printf("<main_idtable> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 4, gototable[sstack->stack[sstack->top]][4]);    // 4:刚规约出来的非终结符代号
                /*语义分析*/
                TableHead *maintable = (TableHead *)malloc(sizeof(TableHead));
                InitTblH(NULL, maintable, idstack.stack[idstack.top]);    // ----- 能不能弹到这里为名字还需检查------//
                PopTokenstack(&idstack);
                PushSemstack(tnfstack, maintable);
                entridtbl = maintable;
                break;
            case -9:
                /*语法分析*/
                printf("<program_block> => <block> . ;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 5, gototable[(*sstack).stack[(*sstack).top]][5]);    // 5:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -10:
                printf("<block> => <constant_definitions> <variable_declarations> <procedure_function_declarations> <statement_part>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 6, gototable[(*sstack).stack[(*sstack).top]][6]);    // 6:刚规约出来的非终结符代号
                break;
            case -11:
                printf("<constant_definitions> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 7, gototable[(*sstack).stack[(*sstack).top]][7]);    // 7:刚规约出来的非终结符代号
                break;
            case -12:
                printf("<constant_definitions> => const <constant_definition_sequence> semi;\n");
                for(i = 3; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 7, gototable[(*sstack).stack[(*sstack).top]][7]);    // 7:刚规约出来的非终结符代号
                break;
            case -13:
                printf("<constant_definition_sequence> => <constant_definition_sequence> semi <constant_definition>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 8, gototable[(*sstack).stack[(*sstack).top]][8]);    // 8:刚规约出来的非终结符代号
                break;
            case -14:
                printf("<constant_definition_sequence> => <constant_definition>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 8, gototable[(*sstack).stack[(*sstack).top]][8]);    // 8:刚规约出来的非终结符代号
                break;
            case -15:
                /*语法分析*/
                printf("<constant_definition> => ID = <constant>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 9, gototable[(*sstack).stack[(*sstack).top]][9]);    // 9:刚规约出来的非终结符代号
                /*语义分析*/
                position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                PopTokenstack(&idstack);    // 使用之后弹出

                strcpy(idtable[position].value, constack.stack[constack.top]);    // 赋值的填表动作
                strcpy(idtable[position].kind, "const");    // 更新符号表中 符号的种类
                idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                typos = GetType(typtbl, constyp);
                if(typos == -1)
                {
                    printf("Undefined type: %s\n", constyp);
                    typos = 0;
                }
                if(!strcasecmp(constyp, "char"))    // 字符串，作为字符数组
                {
                    strcpy(type, "[");
                    strcat(type, constyp);
                    strcpy(idtable[position].type, type);    // 更新符号表中 符号的类型
                    amount = strlen(constack.stack[constack.top]);    // 数组大小  ------ 记得检查准确性--------------//
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 1;
                    idtable[position].extend->dlen = (int *)malloc(1 * sizeof(int));
                    idtable[position].extend->dlen[0] = amount - 2;    // 减去引号

                    tnfstack->offset[tnfstack->top] += (amount - 2) * typtbl[typos].width;    // 更新栈中偏移
                }
                else
                {
                    strcpy(idtable[position].type, constyp);    // 更新符号表中 符号的类型
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // 更新栈中偏移
                }
                PopTokenstack(&constack);    // 使用之后弹出
                break;
            case -16:
                /*语法分析*/
                printf("<constant> => INT;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(constyp, "integer");
                break;
            case -17:
                /*语法分析*/
                printf("<constant> => REAL;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(constyp, "real");
                break;
            case -18:
                /*语法分析*/
                printf("<constant> => STRING;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(constyp, "char");
                break;
            case -19:
                printf("<variable_declarations> =>;\n");
                PushParstack(cstack, sstack, 11, gototable[(*sstack).stack[(*sstack).top]][11]);    // 11:刚规约出来的非终结符代号
                break;
            case -20:
                printf("<variable_declarations> => var <variable_declaration_sequence> semi;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 11, gototable[(*sstack).stack[(*sstack).top]][11]);    // 11:刚规约出来的非终结符代号
                break;
            case -21:
                printf("<variable_declaration_sequence> => <variable_declaration_sequence> semi <variable_declaration>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 12, gototable[(*sstack).stack[(*sstack).top]][12]);    // 12:刚规约出来的非终结符代号
                break;
            case -22:
                printf("<variable_declaration_sequence> => <variable_declaration>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 12, gototable[(*sstack).stack[(*sstack).top]][12]);    // 12:刚规约出来的非终结符代号
                break;
            case -23:
                /*语法分析*/
                printf("<variable_declaration> => <identifier_list> : <type_denoter>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 13, gototable[(*sstack).stack[(*sstack).top]][13]);    // 13:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(type, idstack.stack[idstack.top]);
                if(idstack.stack[idstack.top][0] == '^')    // 指针类型
                {
                    type[0] = idstack.stack[idstack.top][1];
                    for(i = 2; i < strlen(idstack.stack[idstack.top]); i++)
                    {
                        type[i - 1] = idstack.stack[idstack.top][i];
                    }
                    type[i - 1] = '\0';
                }
                else
                {
                    strcpy(type, idstack.stack[idstack.top]);
                }
                typos = GetType(typtbl, type);    // 查找类型在类型表中的位置
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                if(idstack.stack[idstack.top][0] == '^')    // 指针类型
                {
                    strcpy(type, idstack.stack[idstack.top]);
                    width = 4;
                }
                else
                {
                    width = typtbl[typos].width;
                }
                PopTokenstack(&idstack);    // 弹出，类型
                if(settype->dimen == 0)    // 非数组类型
                {
                    for(i = countidlis - 1; i >= 0;i --)    // 一个个赋予类型和偏移值
                    {
                        position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);    // 更新并获取在符号表中的位置,按顺序所以-i
                        strcpy(idtable[position].kind, "var");    // 种类
                        strcpy(idtable[position].type, type);    // 类型
                        idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                        tnfstack->offset[tnfstack->top] += width;    // 更新栈中偏移
                    }
                }
                else    // 数组类型
                {
                    strcpy(constyp, "[");
                    strcat(constyp, type);
                    amount = 1;
                    for(j = 0; j < settype->dimen; j++)
                    {
                        amount *= settype->dlen[j];
                    }
                    amount *= typtbl[typos].width;
                    for(i = countidlis - 1; i >= 0;i --)    // 一个个赋予类型和偏移值
                    {
                        position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);    // 更新并获取在符号表中的位置,按顺序所以-i
                        strcpy(idtable[position].kind, "var");    // 种类
                        strcpy(idtable[position].type, constyp);    // 类型
                        idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                        idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));    // 扩展属性
                        idtable[position].extend->dimen = settype->dimen;    // 维数
                        idtable[position].extend->dlen = (int *)malloc(settype->dimen * sizeof(int));    // 各维长度
                        for(j = 0; j < settype->dimen; j++)
                        {
                            idtable[position].extend->dlen[j] = settype->dlen[j];
                            tnfstack->offset[tnfstack->top] += amount;    // 更新栈中偏移
                        }
                        tnfstack->offset[tnfstack->top] -= amount;    // 多了
                    }
                    settype->dimen = 0;
                }
                while(countidlis > 0)    // 弹出已经语义分析过的符号
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -24:
                printf("<type_denoter> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 14, gototable[(*sstack).stack[(*sstack).top]][14]);    // 14:刚规约出来的非终结符代号
                break;
            case -25:
                printf("<procedure_function_declarations> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 15, gototable[(*sstack).stack[(*sstack).top]][15]);    // 15:刚规约出来的非终结符代号
                break;
            case -26:
                printf("<procedure_function_declarations> => <procedure_function_declaration_sequence>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 15, gototable[(*sstack).stack[(*sstack).top]][15]);    // 15:刚规约出来的非终结符代号
                break;
            case -27:
                printf("<procedure_function_declaration_sequence> => <procedure_function_declaration>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 16, gototable[(*sstack).stack[(*sstack).top]][16]);    // 16:刚规约出来的非终结符代号
                break;
            case -28:
                printf("<procedure_function_declaration_sequence> => <procedure_function_declaration_sequence> <procedure_function_declaration>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 16, gototable[(*sstack).stack[(*sstack).top]][16]);    // 16:刚规约出来的非终结符代号
                break;
            case -29:
                /*语法分析*/
                printf("<procedure_function_declaration> => <function_declaration>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 17, gototable[(*sstack).stack[(*sstack).top]][17]);    // 17:刚规约出来的非终结符代号
                /*语义分析*/
                GenCode('\0', "ret", '\0', '\0');
                break;
            case -30:
                /*语法分析*/
                printf("<function_declaration> => <function_heading> semi <inner_idtable> <function_block>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 18, gototable[(*sstack).stack[(*sstack).top]][18]);    // 18:刚规约出来的非终结符代号
                /*语义分析*/
                inhead = tnfstack->tblhead[tnfstack->top];
                inhead->offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;    // 其符号表建成，弹出栈
                outhead = tnfstack->tblhead[tnfstack->top];
                outhead->inner[outhead->nextin] = inhead;
                idtable[porfh].extend->dlen[0] = outhead->nextin;    // 符号表记录中标记该函数符号表表头在外层的哪个 内层指针存储
                outhead->nextin ++;
                if(outhead->nextin >= outhead->insize)    // 动态扩充
                {
                    outhead->insize *= 2;
                    TableHead **newinner = (TableHead **)malloc(sizeof(TableHead *) * outhead->insize);
                    for(i = 0; i < outhead->nextin; i ++)    // 复制
                    {
                        newinner[i] = (TableHead *)malloc(sizeof(TableHead));
                        if(i < outhead->nextin / 2)
                        {
                            newinner[i] = outhead->inner[i];
                        }
                    }
                    free(outhead->inner);
                    outhead->inner = newinner;
                }
                break;
            case -31:
                /*语法分析*/
                printf("<function_heading> => function <function_identifier> : <type_identifier>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 19, gototable[(*sstack).stack[(*sstack).top]][19]);    // 19:刚规约出来的非终结符代号
                /*语义分析*/
                deffunc = 0;
                strcpy(idtable[porfh].type, idstack.stack[idstack.top]);    // 函数返回值类型
                // PopTokenstack(&idstack);    不弹出，还会使用
                typos = GetType(typtbl, idtable[porfh].type);    // 查找类型在类型表中的位置
                if(typos == -1)
                {
                    strcpy(errmssg, "Unfefined type :");
                    strcat(errmssg, idtable[porfh].type);
                    Error(fe, row, errmssg);
                }
                break;
            case -32:
                /*语法分析*/
                printf("<function_heading> => function <function_identifier> ( <formal_parameter_list> ) : <type_identifier>;\n");    // 对应产生式
                for(i = 7; i > 0; i --)    // 7:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 19, gototable[(*sstack).stack[(*sstack).top]][19]);    // 19:刚规约出来的非终结符代号
                /*语义分析*/
                deffunc = 0;
                strcpy(idtable[porfh].type, idstack.stack[idstack.top]);    // 函数返回值类型
                // PopTokenstack(&idstack);    不弹出，还会使用
                typos = GetType(typtbl, type);    // 查找类型在类型表中的位置
                if(typos == -1)
                {
                    strcpy(errmssg, "Unfefined type :");
                    strcat(errmssg, idtable[porfh].type);
                    Error(fe, row, errmssg);
                }
                break;
            case -33:
                printf("<type_identifier> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 20, gototable[(*sstack).stack[(*sstack).top]][20]);    // 20:刚规约出来的非终结符代号
                break;
            case -34:
                printf("<formal_parameter_list> => <formal_parameter_section>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 21, gototable[(*sstack).stack[(*sstack).top]][21]);    // 21:刚规约出来的非终结符代号
                break;
            case -35:
                printf("<formal_parameter_list> => <formal_parameter_list> semi <formal_parameter_section>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 21, gototable[(*sstack).stack[(*sstack).top]][21]);    // 21:刚规约出来的非终结符代号
                break;
            case -36:
                printf("<formal_parameter_section> => <value_parameter_specification>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 22, gototable[(*sstack).stack[(*sstack).top]][22]);    // 22:刚规约出来的非终结符代号
                break;
            case -37:
                printf("<formal_parameter_section> => <variable_parameter_specification>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 22, gototable[(*sstack).stack[(*sstack).top]][22]);    // 22:刚规约出来的非终结符代号
                break;
            case -38:
                /*语法分析*/
                printf("<value_parameter_specification> => <identifier_list> : <type_identifier>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 23, gototable[(*sstack).stack[(*sstack).top]][23]);    // 23:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(type, idstack.stack[idstack.top]);
                typos = GetType(typtbl, type);    // 查找类型在类型表中的位置
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                PopTokenstack(&idstack);    // 弹出，类型
                idtable[porfh].extend->dimen += countidlis;    // 函数或者函数在外层表的记录位置记录 参数数量
//                printf("%d: +%d\n", porfh, idtable[porfh].extend->dimen);
//                exit(0);
                for(i = countidlis - 1; i >= 0;i --)    // 一个个赋予类型和偏移值
                {
                    // 更新并获取在符号表中的位置,按顺序所以-i
                    position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "para");    // 种类
                    strcpy(idtable[position].type, type);    // 类型
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // 更新栈中偏移
                }
                while(countidlis > 0)    // 无用，弹出
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -39:
                /*语法分析*/
                printf("<variable_parameter_specification> => var <identifier_list> : <type_identifier>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 24, gototable[(*sstack).stack[(*sstack).top]][24]);    // 24:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(type, idstack.stack[idstack.top]);
                typos = GetType(typtbl, type);    // 查找类型在类型表中的位置
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                PopTokenstack(&idstack);    // 弹出，类型
                idtable[porfh].extend->dimen += countidlis;    // 函数或者函数在外层表的记录位置记录 参数数量
                constyp[0] = 'v';
                for(i = 0; i < strlen(type); i ++)
                {
                    constyp[i + 1] = type[i];
                }
                constyp[i + 1] = '\0';
                for(i = countidlis - 1; i >= 0;i --)    // 一个个赋予类型和偏移值
                {
                    // 更新并获取在符号表中的位置,按顺序所以-i
                    position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "para");    // 种类
                    strcpy(idtable[position].type, constyp);    // 类型
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // 更新栈中偏移
                }
                while(countidlis > 0)    // 无用，弹出
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -40:
                /*语法分析*/
                printf("<inner_idtable> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 25, gototable[(*sstack).stack[(*sstack).top]][25]);    // 25:刚规约出来的非终结符代号
                /*语义分析*/
                TableHead *innertable = (TableHead *)malloc(sizeof(TableHead));
                if(fun_or_proc)
                {
                    fun_or_proc = 0;
                    InitTblH(tnfstack->tblhead[tnfstack->top], innertable, idstack.stack[idstack.top - 1]);    // ----- 能不能弹到这里为名字还需检查------//
                    PushSemstack(tnfstack, innertable);
                    strcpy(type, idstack.stack[idstack.top]);
                    PopTokenstack(&idstack);    // 弹出，类型
                    typos = GetType(typtbl, type);    // 查找类型在类型表中的位置
                    if(typos == -1)
                    {
                        printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                        exit(0);
                    }
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "ret");    // 种类
                    strcpy(idtable[position].type, type);    // 类型
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // 更新符号表中 符号的偏移
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // 更新栈中偏移
                }
                else
                {
                    InitTblH(tnfstack->tblhead[tnfstack->top], innertable, idstack.stack[idstack.top]);    // ----- 能不能弹到这里为名字还需检查------//
                    PushSemstack(tnfstack, innertable);
                }
                PopTokenstack(&idstack);
                break;
            case -41:
                /*语法分析*/
                printf("<function_block> => <block> semi;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 26, gototable[(*sstack).stack[(*sstack).top]][26]);    // 26:刚规约出来的非终结符代号
                break;
            case -42:
                printf("<statement_part> => <compound_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 27, gototable[(*sstack).stack[(*sstack).top]][27]);    // 27:刚规约出来的非终结符代号
                break;
            case -43:
                printf("<compound_statement> => begin <statement_sequence> end;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 28, gototable[(*sstack).stack[(*sstack).top]][28]);    // 28:刚规约出来的非终结符代号
                break;
            case -44:
                printf("<statement_sequence> => <statement> semi;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 29, gototable[(*sstack).stack[(*sstack).top]][29]);    // 29:刚规约出来的非终结符代号
                break;
            case -45:
                /*语法分析*/
                printf("<procedure_function_declaration> => <procedure_declaration>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 17, gototable[(*sstack).stack[(*sstack).top]][17]);    // 17:刚规约出来的非终结符代号
                /*语义分析*/
                GenCode('\0', "ret", '\0', '\0');
                break;
            case -46:
                /*语法分析*/
                printf("<procedure_declaration> => <procedure_heading> semi <inner_idtable> <procedure_block>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                /*语义分析*/
                inhead = tnfstack->tblhead[tnfstack->top];
                inhead->offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;
                outhead = tnfstack->tblhead[tnfstack->top];
                outhead->inner[outhead->nextin] = inhead;
                idtable[porfh].extend->dlen[0] = outhead->nextin;    // 符号表记录中标记该函数符号表表头在外层的哪个 内层指针存储
                outhead->nextin ++;
                if(outhead->nextin >= outhead->insize)    // 动态扩充
                {
                    outhead->insize *= 2;
                    TableHead **newinner = (TableHead **)malloc(sizeof(TableHead *) * outhead->insize);
                    for(i = 0; i < outhead->nextin; i ++)    // 复制
                    {
                        newinner[i] = (TableHead *)malloc(sizeof(TableHead));
                        if(i < outhead->nextin / 2)
                        {
                            newinner[i] = outhead->inner[i];
                        }
                    }
                    free(outhead->inner);
                    outhead->inner = newinner;
                }
                PushParstack(cstack, sstack, 30, gototable[(*sstack).stack[(*sstack).top]][30]);    // 30:刚规约出来的非终结符代号
                break;
            case -47:
                /*语法分析*/
                printf("<procedure_block> => <block> semi;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 31, gototable[(*sstack).stack[(*sstack).top]][31]);    // 31:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -48:
                /*语法分析*/
                printf("<procedure_heading> => procedure <procedure_identifier>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 32, gototable[(*sstack).stack[(*sstack).top]][32]);    // 32:刚规约出来的非终结符代号
                /*语义分析*/
                defproc = 0;
                break;
            case -49:
                /*语法分析*/
                printf("<procedure_heading> => procedure <procedure_identifier> ( <formal_parameter_list> );\n");    // 对应产生式
                for(i = 5; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 32, gototable[(*sstack).stack[(*sstack).top]][32]);    // 32:刚规约出来的非终结符代号
                /*语义分析*/
                defproc = 0;
                break;
            case -50:
                /*语法分析*/
                printf("<statement_sequence> => <statement_sequence> <bool_mark_M> <statement> semi;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 29, gototable[(*sstack).stack[(*sstack).top]][29]);    // 29:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = nextlist.stack[nextlist.top];    // 取出来
                nextlist.top --;
//                printf("%d\n", nextlist.top);
                tmplist2 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist2, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&nextlist, tmplist1);    // 再压回去
                break;
            case -51:
                printf("<statement> => <simple_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 33, gototable[(*sstack).stack[(*sstack).top]][33]);    // 33:刚规约出来的非终结符代号
                break;
            case -52:
                /*语法分析*/
                printf("<simple_statement> => <assignment_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 34, gototable[(*sstack).stack[(*sstack).top]][34]);    // 34:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist4[0] = -1;
                PushListstack(&nextlist, tmplist4);    // 压入空链
//                nextlist.top ++;    // 相当于压入空链
                break;
            case -53:
                 /*语法分析*/
                printf("<assignment_statement> => <variable_access> := <expression>\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 35, gototable[(*sstack).stack[(*sstack).top]][35]);    // 35:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvarone = tmstack.stack[tmstack.top];    // 右侧
                PopTmpstack();
                tmpvartwo = tmstack.stack[tmstack.top];    // 左侧
                PopTmpstack();
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                if(tmpvartwo.index == -1)    // 未使用临时变量
                {
                    strcpy(result, tmpvartwo.addr);
                }
                else    // 使用了临时变量
                {
                    strcpy(result, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(result, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, ":=", opr1, '\0');
                break;
            case -54:
                printf("<variable_access> => <variable_identifier>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:刚规约出来的非终结符代号
                break;
            case -55:
                /*语法分析*/
                printf("<variable_identifier> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 37, gototable[(*sstack).stack[(*sstack).top]][37]);    // 37:刚规约出来的非终结符代号
                /*语义分析*/
                if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // 未定义变量
                {
                    strcpy(errmssg, "Unfefined variable : ");
                    strcat(errmssg, idstack.stack[idstack.top]);
                    Error(fe, row, errmssg);
                }
                PushTmpstack(-1, idstack.stack[idstack.top]);    // 不申请临时变量，直接压入临时变量栈
                PopTokenstack(&idstack);
                break;
            case -56:
                printf("<expression> => <simple_expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 38, gototable[(*sstack).stack[(*sstack).top]][38]);    // 38:刚规约出来的非终结符代号
                break;
            case -57:
                printf("<simple_expression> => <term>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:刚规约出来的非终结符代号
                break;
            case -58:
                 /*语法分析*/
                printf("<simple_expression> => <sign> <term>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:刚规约出来的非终结符代号
                /*语义分析*/
                if(!strcmp(oprstack.stack[oprstack.top], "-"))
                {
                    tmpvarone = tmstack.stack[tmstack.top];
                    tmpi = NewTemp();
                    strcpy(result, "t");
                    itoa(tmpi, tmpic, 10);
                    strcat(result, tmpic);
                    strcpy(opr1, "minus");
                    if(tmpvarone.index == -1)
                    {
                        strcat(opr1, tmpvarone.addr);
                    }
                    else
                    {
                        strcat(opr1, "t");
                        itoa(tmpvarone.index, tmpic, 10);
                        strcat(opr1, tmpic);
                        temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                    }
                    PopTmpstack();
                    GenCode(result, ":=", opr1, '\0');
                    PushTmpstack(tmpi, '\0');
                }
                PopTokenstack(&oprstack);
                break;
            case -59:
                printf("<simple_expression> => <term> <simple_expression_sequence>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:刚规约出来的非终结符代号
                break;
            case -60:
                /*语法分析*/
                printf("<simple_expression> => <sign> <term> <simple_expression_sequence>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:刚规约出来的非终结符代号
                /*语义分析*/
                if(!strcmp(oprstack.stack[oprstack.top], "-"))
                {
                    tmpvarone = tmstack.stack[tmstack.top];
                    tmpi = NewTemp();
                    strcpy(result, "t");
                    itoa(tmpi, tmpic, 10);
                    strcat(result, tmpic);
                    strcpy(opr1, "minus");
                    // 一定使用了临时变量
                    strcat(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                    PopTmpstack();
                    GenCode(result, ":=", opr1, '\0');
                    PushTmpstack(tmpi, '\0');
                }
                PopTokenstack(&oprstack);
                break;
            case -61:
                printf("<term> => <factor>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 40, gototable[(*sstack).stack[(*sstack).top]][40]);    // 40:刚规约出来的非终结符代号
                break;
            case -62:
                printf("<term> => <factor> <term_sequence>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 40, gototable[(*sstack).stack[(*sstack).top]][40]);    // 40:刚规约出来的非终结符代号
                break;
            case -63:
                /*语法分析*/
                printf("<factor> => <unsigned_constant>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:刚规约出来的非终结符代号
                /*语义分析*/
                PushTmpstack(-1, constack.stack[constack.top]);    // 压入临时变量栈，但其实并不需要申请临时变量，因此下标为-1
                PopTokenstack(&constack);
                break;
            case -64:
                printf("<unsigned_constant>=> INT;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:刚规约出来的非终结符代号
                break;
            case -65:
                printf("<unsigned_constant>=> REAL;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:刚规约出来的非终结符代号
                break;
            case -66:
                printf("<unsigned_constant>=> STRING;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:刚规约出来的非终结符代号
                break;
            case -67:
                printf("<unsigned_constant>=> nil;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:刚规约出来的非终结符代号
                break;
            case -68:
                /*语法分析*/
                printf("<term_sequence> => <multiplying_operator> <factor>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 43, gototable[(*sstack).stack[(*sstack).top]][43]);    // 43:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "*") && strcmp(tmpopr, "/") && strcmp(tmpopr, "mod") && strcmp(tmpopr, "div"))
                {    // 不是*，/，div，mod，优先级顺序定义导致
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // 再按顺序压回
                {
                    strcpy(tmpopr, tmpoprstack.stack[tmpoprstack.top]);
                    PopTokenstack(&tmpoprstack);
                    oprstack.top ++;
                    strcpy(oprstack.stack[oprstack.top], tmpopr);
                }
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr2, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr2, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr2, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -69:
                /*语法分析*/
                printf("<term_sequence> => <term_sequence> <multiplying_operator> <factor>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 43, gototable[(*sstack).stack[(*sstack).top]][43]);    // 43:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "*") && strcmp(tmpopr, "/") && strcmp(tmpopr, "mod") && strcmp(tmpopr, "div"))
                {    // 不是*，/，div，mod，优先级顺序定义导致
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // 再按顺序压回
                {
                    strcpy(tmpopr, tmpoprstack.stack[tmpoprstack.top]);
                    PopTokenstack(&tmpoprstack);
                    oprstack.top ++;
                    strcpy(oprstack.stack[oprstack.top], tmpopr);
                }
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr2, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr2, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr2, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -70:
                printf("<multiplying_operator> => *;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:刚规约出来的非终结符代号
                break;
            case -71:
                printf("<multiplying_operator> => /;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:刚规约出来的非终结符代号
                break;
            case -72:
                printf("<multiplying_operator> => div;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:刚规约出来的非终结符代号
                break;
            case -73:
                printf("<multiplying_operator> => mod;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:刚规约出来的非终结符代号
                break;
            case -74:
                printf("<sign> => +;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 45, gototable[(*sstack).stack[(*sstack).top]][45]);    // 45:刚规约出来的非终结符代号
                break;
            case -75:
                printf("<sign> => -;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 45, gototable[(*sstack).stack[(*sstack).top]][45]);    // 45:刚规约出来的非终结符代号
                break;
            case -76:
                 /*语法分析*/
                printf("<simple_expression_sequence> => <adding_operator> <term>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 46, gototable[(*sstack).stack[(*sstack).top]][46]);    // 46:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "+") && strcmp(tmpopr, "-"))    // 不是+ 也不是-，优先级和输入顺序不一致导致
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // 再按顺序压回
                {
                    strcpy(tmpopr, tmpoprstack.stack[tmpoprstack.top]);
                    PopTokenstack(&tmpoprstack);
                    oprstack.top ++;
                    strcpy(oprstack.stack[oprstack.top], tmpopr);
                }
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr2, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr2, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr2, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -77:
                /*语法分析*/
                printf("<simple_expression_sequence> => <simple_expression_sequence> <adding_operator> <term>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 46, gototable[(*sstack).stack[(*sstack).top]][46]);    // 46:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "+") && strcmp(tmpopr, "-"))    // 不是+ 也不是-，优先级和输入顺序不一致导致
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // 再按顺序压回
                {
                    strcpy(tmpopr, tmpoprstack.stack[tmpoprstack.top]);
                    PopTokenstack(&tmpoprstack);
                    oprstack.top ++;
                    strcpy(oprstack.stack[oprstack.top], tmpopr);
                }
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr2, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr2, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr2, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -78:
                printf("<adding_operator> => +;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 47, gototable[(*sstack).stack[(*sstack).top]][47]);    // 47:刚规约出来的非终结符代号
                break;
            case -79:
                printf("<adding_operator> => -;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 47, gototable[(*sstack).stack[(*sstack).top]][47]);    // 47:刚规约出来的非终结符代号
                break;
            case -80:
                printf("<type_denoter> => <new_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 14, gototable[(*sstack).stack[(*sstack).top]][14]);    // 14:刚规约出来的非终结符代号
                break;
            case -81:
                printf("<new_type> => <new_structured_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 48, gototable[(*sstack).stack[(*sstack).top]][48]);    // 48:刚规约出来的非终结符代号
                break;
            case -82:
                printf("<new_structured_type> => <unpacked_structured_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 49, gototable[(*sstack).stack[(*sstack).top]][49]);    // 49:刚规约出来的非终结符代号
                break;
            case -83:
                printf("<unpacked_structured_type> => <array_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 50, gototable[(*sstack).stack[(*sstack).top]][50]);    // 50:刚规约出来的非终结符代号
                break;
            case -84:
                printf("<array_type> => array [ <index_type> <index_type_sequence> ] of <component_type>;\n");    // 对应产生式
                for(i = 7; i > 0; i --)    // 7:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 51, gototable[(*sstack).stack[(*sstack).top]][51]);    // 51:刚规约出来的非终结符代号
                break;
            case -85:
                printf("<index_type_sequence> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 52, gototable[(*sstack).stack[(*sstack).top]][52]);    // 52:刚规约出来的非终结符代号
                break;
            case -86:
                printf("<index_type_sequence> => <index_type_sequence> , <index_type>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 52, gototable[(*sstack).stack[(*sstack).top]][52]);    // 52:刚规约出来的非终结符代号
                break;
            case -87:
                printf("<index_type> => <ordinal_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 53, gototable[(*sstack).stack[(*sstack).top]][53]);    // 53:刚规约出来的非终结符代号
                break;
            case -88:
                printf("<ordinal_type> => <new_ordinal_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 54, gototable[(*sstack).stack[(*sstack).top]][54]);    // 54:刚规约出来的非终结符代号
                break;
            case -89:
                printf("<new_ordinal_type> => <subrange_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 55, gototable[(*sstack).stack[(*sstack).top]][55]);    // 55:刚规约出来的非终结符代号
                break;
            case -90:
                /*语法分析*/
                printf("<subrange_type> => <constant> .. <constant>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 56, gototable[(*sstack).stack[(*sstack).top]][56]);    // 56:刚规约出来的非终结符代号
                /*语义分析*/
                settype->dlen[settype->dimen] = atoi(constack.stack[constack.top]) - atoi(constack.stack[constack.top - 1]) + 1;
                PopTokenstack(&constack);
                PopTokenstack(&constack);
                settype->dimen ++;
                if(settype->dimen > BASDIMEN)
                {
                    if(settype->dimen > sizeof(settype->dlen) / sizeof(int))
                    {
                        int *tmpl = (int *)malloc(settype->dimen * sizeof(int));
                        for(i = 0; i < settype->dimen - 1; i++)
                        {
                            tmpl[i] = settype->dlen[i];
                        }
                        free(settype->dlen);
                        settype->dlen = tmpl;
                    }
                }
                break;
            case -91:
                printf("<component_type> => <type_denoter>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 57, gototable[(*sstack).stack[(*sstack).top]][57]);    // 57:刚规约出来的非终结符代号
                break;
            case -92:
                printf("<variable_access> => <component_variable>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:刚规约出来的非终结符代号
                break;
            case -93:
                /*语法分析*/
                printf("<component_variable> => <indexed_variable>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 58, gototable[(*sstack).stack[(*sstack).top]][58]);    // 58:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                strcpy(opr1, "t");
                itoa(tmpvarone.index, tmpic, 10);
                strcat(opr1, tmpic);
                temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                strcat(opr1, "[");
                strcat(opr1, "t");
                itoa(tmpvartwo.index, tmpic, 10);
                strcat(opr1, tmpic);
                temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                strcat(opr1, "]");
                GenCode(result, ":=", opr1, '\0');
                PushTmpstack(tmpi, '\0');
                arryflag = 0;
                break;
            case -94:
                /*语法分析*/
                printf("<indexed_variable> => <index_expression_sequence> ];\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 59, gototable[(*sstack).stack[(*sstack).top]][59]);    // 59:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpi = NewTemp();
                strcpy(result, "t");
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(oprnt, "*");
                itoa(setypevar.width, tmpic, 10);
                strcpy(opr2, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, oprnt, opr1, opr2);    // 数组型变量位移

                tmpi = NewTemp();
                strcpy(result, "t");
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                itoa(setypevar.fixepart, tmpic, 10);
                strcpy(opr1, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, ":=", opr1, '\0');    // 数组型变量基址
                break;
            case -95:
                /*语法分析*/
                printf("<index_expression_sequence> => <array_variable> [ <index_expression>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 60, gototable[(*sstack).stack[(*sstack).top]][60]);    // 60:刚规约出来的非终结符代号
                /*语义分析*/
                setypevar.ndim = 1;
                break;
            case -96:
                /*语法分析*/
                printf("<index_expression_sequence> => <index_expression_sequence> , <index_expression>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 60, gototable[(*sstack).stack[(*sstack).top]][60]);    // 60:刚规约出来的非终结符代号
                /*语义分析*/
                // 计算数组型变量的偏移
                tmpvarone = tmstack.stack[tmstack.top];    // 临时变量栈栈顶
                tmpvartwo = tmstack.stack[tmstack.top - 1];    // 临时变量栈次顶

                setypevar.ndim ++;
                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr1, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvartwo.index] = 0;        // 使用了临时变量， 归还临时变量池
                }
                strcpy(oprnt, "*");
                itoa(setypevar.dlen[setypevar.ndim - 1], tmpic, 10);
                strcpy(opr2, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, oprnt, opr1, opr2);

                tmpvartwo = tmpvarone;    // 临时变量栈次顶(刚刚进行了压栈)
                tmpvarone = tmstack.stack[tmstack.top];    // 临时变量栈栈顶

                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                strcpy(opr1, "t");
                itoa(tmpvarone.index, tmpic, 10);
                strcat(opr1, tmpic);
                temps[tmpvarone.index] = 0;        // 使用了临时变量， 归还临时变量池
                strcpy(oprnt, "+");
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr2, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr2, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr2, tmpic);
                    temps[tmpvartwo.index] = 0;        // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, oprnt, opr1, opr2);
                PopTmpstack();
                PopTmpstack();
                PopTmpstack();
                PushTmpstack(tmpi, '\0');
                break;
            case -97:
                /*语法分析*/
                printf("<array_variable> => <variable_access>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 61, gototable[(*sstack).stack[(*sstack).top]][61]);    // 61:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(setypevar.base, tmstack.stack[tmstack.top].addr);    // 数组型变量的名字
                j = SearchTable(setypevar.base, tnfstack->tblhead[tnfstack->top]);    // 根据名字查idtable
                PopTmpstack();    // 弹栈
                if(j == -1)    // 未定义变量
                {
                    strcpy(errmssg, "Unfefined variable : ");
                    strcat(errmssg, idstack.stack[idstack.top]);
                    Error(fe, row, errmssg);
                }
                //printf("top trihead = %d", tnfstack->top);
//                if(strcasecmp("[", idtable[j].type))
//                {
//                    printf("%s is not array type", setypevar.base);
//                    exit(0);
//                }
                constyp[0] = idtable[j].type[1];
                for(i = 2; i < strlen(idtable[j].type); i++)
                {
                    constyp[i - 1] = idtable[j].type[i];
                }
                constyp[i - 1] = '\0';
                tmpi = GetType(typtbl, constyp);
                setypevar.width = typtbl[tmpi].width;    // 获取宽度
                //    计算固定偏移量
//                if(idtable[j].extend->dimen < 2)
//                {
//                    setypevar.fixepart = idtable[j].offset;
//                }
//                else
//                {
//                    j = 1;
//                    for(i = 2; i <= idtable[j].extend->dimen; i ++)
//                    {
//                        j *= idtable[j].extend->dlen[i - 1];
//                    }
//                    setypevar.fixepart = idtable[j].offset - j;
//                }
                setypevar.fixepart = idtable[j].offset - 0;
                setypevar.dlen = idtable[j].extend->dlen;    // 各维维长
                // 以上语义分析根据名字将数组型临时变量的属性保存在 setypevar 中，省去以后一次次查表，从setypevar中取即可
                break;
            case -98:
                printf("<index_expression> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 62, gototable[(*sstack).stack[(*sstack).top]][62]);    // 62:刚规约出来的非终结符代号
                break;
            case -99:
                /*语法分析*/
                printf("<factor> => <variable_access>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:刚规约出来的非终结符代号
                break;
            case -100:
                /*语法分析*/
                printf("<factor> => ( <expression> );\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:刚规约出来的非终结符代号
                break;
            case -101:
                /*语法分析*/
                printf("<statement> => <structured_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 33, gototable[(*sstack).stack[(*sstack).top]][33]);    // 33:刚规约出来的非终结符代号
                break;
            case -102:
                /*语法分析*/
                printf("<structured_statement> => <compound_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:刚规约出来的非终结符代号
                break;
            case -103:
                /*语法分析*/
                printf("<structured_statement> => <conditional_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:刚规约出来的非终结符代号
                break;
            case -104:
                /*语法分析*/
                printf("<conditional_statement> => <if_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 64, gototable[(*sstack).stack[(*sstack).top]][64]);    // 64:刚规约出来的非终结符代号
                break;
            case -105:
                /*语法分析*/
                printf("<if_statement> => if <boolean_expression> then <bool_mark_M> <statement>;\n");    // 对应产生式
                for(i = 5; i > 0; i --)    // 5:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 65, gototable[(*sstack).stack[(*sstack).top]][65]);    // 65:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&nextlist, tmplist4);
                break;
            case -106:
                /*语法分析*/
                printf("<if_statement> => if <boolean_expression> then <bool_mark_M> <statement> <bool_mark_N> <else_part>;\n");    // 对应产生式
                for(i = 7; i > 0; i --)    // 7:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 65, gototable[(*sstack).stack[(*sstack).top]][65]);    // 65:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                tmplist2 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Merge(tmplist4, tmplist1, tmplist5);
                PushListstack(&nextlist, tmplist5);
                break;
            case -107:
                /*语法分析*/
                printf("<boolean_expression> => <boolean_term>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:刚规约出来的非终结符代号
                break;
            case -108:
                /*语法分析*/
                printf("<boolean_expression> => not <boolean_term>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:刚规约出来的非终结符代号
                /*语义分析*/
                i = 0;
                while(truelist.stack[truelist.top][i] != -1)    // 备份
                {
                    tmplist4[i] = truelist.stack[truelist.top][i];
                    i ++;
                }
//                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                PushListstack(&truelist, tmplist2);
                tmplist1 = tmplist4;
                PushListstack(&falselist, tmplist1);

                break;
            case -109:
                /*语法分析*/
                printf("<boolean_expression> => <boolean_term> <boolean_expression_sequence>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:刚规约出来的非终结符代号
                break;
            case -110:
                /*语法分析*/
                printf("<boolean_expression> => not <boolean_term> <boolean_expression_sequence>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                PushListstack(&truelist, tmplist2);
                PushListstack(&falselist, tmplist1);
                break;
            case -111:
                /*语法分析*/
                printf("<boolean_factor> => ( <boolean_expression> );\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 68, gototable[(*sstack).stack[(*sstack).top]][68]);    // 68:刚规约出来的非终结符代号
                break;
            case -112:
                /*语法分析*/
                printf("<boolean_term> => <boolean_factor>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 67, gototable[(*sstack).stack[(*sstack).top]][67]);    // 67:刚规约出来的非终结符代号
                break;
            case -113:
                /*语法分析*/
                printf("<boolean_term> => <boolean_factor> <boolean_term_sequence>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 67, gototable[(*sstack).stack[(*sstack).top]][67]);    // 67:刚规约出来的非终结符代号
                break;
            case -114:
                /*语法分析*/
                printf("<boolean_factor> => <expression> <relational_operator> <expression>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 68, gototable[(*sstack).stack[(*sstack).top]][68]);    // 68:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmplist4[0] = nextcode;
                PushListstack(&truelist, tmplist4);
                tmplist4[0] = nextcode + 1;
                PushListstack(&falselist, tmplist4);
//                printf("CHECK >>>> ------ >>>> --------- %d: %d\n", falselist.top, falselist.stack[falselist.top][0]);
                tmplist4[0] = -1;    // 还原，不然影响以后的
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(!strcmp(tmpopr, "and") || !strcmp(tmpopr, "or"))    // 是+ 或者是-，优先级与输入顺序不一致导致
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcat(opr1, tmpopr);
                while(tmpoprstack.top >= 0)    // 再按顺序压回
                {
                    strcpy(tmpopr, tmpoprstack.stack[tmpoprstack.top]);
                    PopTokenstack(&tmpoprstack);
                    oprstack.top ++;
                    strcpy(oprstack.stack[oprstack.top], tmpopr);
                }
                if(tmpvartwo.index == -1)
                {
                    strcat(opr1, tmpvartwo.addr);
                }
                else
                {
                    strcat(opr1, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode('\0', "if", opr1, "goto");
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -115:
                /*语法分析*/
                printf("<relational_operator> => =;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -116:
                /*语法分析*/
                printf("<relational_operator> => <>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -117:
                /*语法分析*/
                printf("<relational_operator> => <;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -118:
                /*语法分析*/
                printf("<relational_operator> => >;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -119:
                /*语法分析*/
                printf("<relational_operator> => <=;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -120:
                /*语法分析*/
                printf("<relational_operator> => >=;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -121:
                /*语法分析*/
                printf("<relational_operator> => in;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:刚规约出来的非终结符代号
                break;
            case -122:
                /*语法分析*/
                printf("<boolean_term_sequence> => and <bool_mark_M> <boolean_factor>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 70, gototable[(*sstack).stack[(*sstack).top]][70]);    // 70:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(truelist.stack[truelist.top],Mquad.stack[Mquad.top]);
                truelist.top --;
                Mquad.top --;
                PushListstack(&truelist, tmplist1);    // 再压入
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&falselist, tmplist4);
                break;
            case -123:
                /*语法分析*/
                printf("<boolean_term_sequence> => <boolean_term_sequence> and <bool_mark_M> <boolean_factor>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 70, gototable[(*sstack).stack[(*sstack).top]][70]);    // 70:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(truelist.stack[truelist.top],Mquad.stack[Mquad.top]);
                truelist.top --;
                Mquad.top --;
                PushListstack(&truelist, tmplist1);    // 再压入
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&falselist, tmplist4);
                break;
            case -124:
                /*语法分析*/
                printf("<boolean_expression_sequence> => or <bool_mark_M> <boolean_term>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 71, gototable[(*sstack).stack[(*sstack).top]][71]);    // 71:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist2 = falselist.stack[falselist.top];    // 取出
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&falselist, tmplist2);    // 再压入
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&truelist,tmplist4);
                break;
            case -125:
                /*语法分析*/
                printf("<boolean_expression_sequence> => <boolean_expression_sequence> or <bool_mark_M> <boolean_term>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 71, gototable[(*sstack).stack[(*sstack).top]][71]);    // 71:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist2 = falselist.stack[falselist.top];    // 取出
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&falselist, tmplist2);    // 再压入
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&truelist,tmplist4);
                break;
            case -126:
                /*语法分析*/
                printf("<bool_mark_M> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 72, gototable[(*sstack).stack[(*sstack).top]][72]);    // 72:刚规约出来的非终结符代号
                /*语义分析*/
                Mquad.top ++;
                Mquad.stack[Mquad.top] = nextcode;
                break;
            case -127:
                /*语法分析*/
                printf("<bool_mark_N> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 73, gototable[(*sstack).stack[(*sstack).top]][73]);    // 73:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist4[0] = nextcode;
                PushListstack(&nextlist, tmplist4);
                tmplist4[0] = -1;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -128:
                /*语法分析*/
                printf("<else_part> => else <bool_mark_M> <statement>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 74, gototable[(*sstack).stack[(*sstack).top]][74]);    // 74:刚规约出来的非终结符代号
                break;
            case -129:
                printf("<structured_statement> => <repetitive_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:刚规约出来的非终结符代号
                break;
            case -130:
                printf("<repetitive_statement> => <while_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:刚规约出来的非终结符代号
                break;
            case -131:
                /*语法分析*/
                printf("<while_statement> => while <bool_mark_M> <boolean_expression> do <bool_mark_M> <statement>;\n");    // 对应产生式
                for(i = 6; i > 0; i --)    // 6:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 76, gototable[(*sstack).stack[(*sstack).top]][76]);    // 76:刚规约出来的非终结符代号
                /*语义分析*/
                mq2 = Mquad.stack[Mquad.top];
                Mquad.top --;
                mq1 = Mquad.stack[Mquad.top];
                Mquad.top --;
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, mq1);
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(tmplist2, mq2);
                tmplist3 = falselist.stack[falselist.top];
                falselist.top --;
                PushListstack(&nextlist, tmplist3);
                itoa(mq1, result, 10);
                GenCode(result, "goto", '\0', '\0');
                break;
             case -132:
                printf("<repetitive_statement> => <for_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:刚规约出来的非终结符代号
                break;
            case -133:
                /*语法分析*/
                printf("<for_statement> => for <control_variable> := <initial_value> to <final_value> do <for_mark_M> <statement>;\n");    // 对应产生式
                for(i = 9; i > 0; i --)    // 9:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 77, gototable[(*sstack).stack[(*sstack).top]][77]);    // 77:刚规约出来的非终结符代号
                /*语义分析*/
                for_down.top --;
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                mq1 = Magain.stack[Magain.top];
                Magain.top --;
                Backpach(tmplist1, mq1);
                itoa(mq1, result, 10);
                GenCode(result, "goto", '\0', '\0');
                break;
            case -134:
                /*语法分析*/
                printf("<for_statement> => for <control_variable> := <initial_value> downto <final_value> do <for_mark_M> <statement>;\n");    // 对应产生式
                for(i = 9; i > 0; i --)    // 9:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 77, gototable[(*sstack).stack[(*sstack).top]][77]);    // 77:刚规约出来的非终结符代号
                /*语义分析*/
                for_down.top --;
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                mq1 = Magain.stack[Magain.top];
                Magain.top --;
                Backpach(tmplist1, mq1);
                itoa(mq1, result, 10);
                GenCode(result, "goto", '\0', '\0');
                break;
            case -135:
                printf("<control_variable> => <entire_variable>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 78, gototable[(*sstack).stack[(*sstack).top]][78]);    // 78:刚规约出来的非终结符代号
                break;
            case -136:
                printf("<entire_variable> => <variable_identifier>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 79, gototable[(*sstack).stack[(*sstack).top]][79]);    // 79:刚规约出来的非终结符代号
                break;
            case -137:
                printf("<initial_value> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 80, gototable[(*sstack).stack[(*sstack).top]][80]);    // 80:刚规约出来的非终结符代号
                break;
            case -138:
                printf("<final_value> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 81, gototable[(*sstack).stack[(*sstack).top]][81]);    // 81:刚规约出来的非终结符代号
                break;
            case -139:
                /*语法分析*/
                printf("<for_mark_M> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 82, gototable[(*sstack).stack[(*sstack).top]][82]);    // 82:刚规约出来的非终结符代号
                /*语义分析*/
                tmpvarthree = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                strcpy(result, tmpvarone.addr);
                if(tmpvartwo.index == -1)
                {
                    strcpy(opr1, tmpvartwo.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvartwo.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, ":=", opr1, '\0');
                tmpi = NewTemp();
                strcpy(result, "t");
                itoa(tmpi, tmpich, 10);
                strcat(result, tmpich);    // 使用 tmpich， 因为后面会用到
                if(tmpvarthree.index == -1)
                {
                    strcpy(opr1, tmpvarthree.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarthree.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarthree.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                GenCode(result, ":=", opr1, '\0');
                mq1 = nextcode;
                tmpi = mq1 + 2;
                itoa(tmpi, result, 10);
                GenCode(result, "goto", '\0', '\0');
                Magain.top ++;
                Magain.stack[Magain.top] = mq1 + 1;
                strcpy(result, tmpvarone.addr);
                strcpy(opr1, tmpvarone.addr);
                if(!for_down.stack[for_down.top])    // to
                {
                    GenCode(result, "+", opr1, "1");
                }
                else    // downto
                {
                    GenCode(result, "-", opr1, "1");
                }
                tmplist4[0] = nextcode;
                for(i = 0;i < LISTLEN; i ++)
                {
                    printf("%d: %d,", i, tmplist4[i]);
                }
                PushListstack(&nextlist, tmplist4);
//                printf("nextlist.top[2] = %d\n", nextlist.stack[nextlist.top][2]);
                tmplist4[0] = -1;
                strcpy(opr1, tmpvarone.addr);
                if(!for_down.stack[for_down.top])    // to
                {
                    strcat(opr1, ">");
                }
                else    // downto
                {
                    strcat(opr1, "<");
                }
                strcat(opr1, "t");
                strcat(opr1, tmpich);
                GenCode('\0', "if", opr1, "goto");
                break;
            case -140:
                printf("<repetitive_statement> => <repeat_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:刚规约出来的非终结符代号
                break;
            case -141:
                /*语法分析*/
                printf("<repeat_statement> => repeat <bool_mark_M> <statement_sequence> until <repeat_mark_N> <boolean_expression>;\n");    // 对应产生式
                for(i = 6; i > 0; i --)    // 6:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 83, gototable[(*sstack).stack[(*sstack).top]][83]);    // 83:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                PushListstack(&nextlist, tmplist2);
                break;
            case -142:
                /*语法分析*/
                printf("<repeat_mark_N> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 84, gototable[(*sstack).stack[(*sstack).top]][84]);    // 84:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -143:
                /*语法分析*/
                printf("<new_type> => <new_pointer_type>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 48, gototable[(*sstack).stack[(*sstack).top]][48]);    // 48:刚规约出来的非终结符代号
                break;
            case -144:
                /*语法分析*/
                printf("<new_pointer_type> => ^ <domain_type>;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 85, gototable[(*sstack).stack[(*sstack).top]][85]);    // 85:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(type, "^");
                strcat(type, idstack.stack[idstack.top]);
                PopTokenstack(&idstack);
                idstack.top ++;
                strcpy(idstack.stack[idstack.top], type);
                break;
            case -145:
                /*语法分析*/
                printf("<domain_type> => <type_identifier>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 86, gototable[(*sstack).stack[(*sstack).top]][86]);    // 86:刚规约出来的非终结符代号
                break;
            case -146:
                /*语法分析*/
                printf("<procedure_identifier> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 87, gototable[(*sstack).stack[(*sstack).top]][87]);    // 87:刚规约出来的非终结符代号
                /*语义分析*/
                // 不弹出，建内层符号表时还需要
                if(defproc)    // 过程定义时
                {
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                    porfh = position;
                    strcpy(idtable[position].id, idstack.stack[idstack.top]);
                    strcpy(idtable[position].kind, "proc");
                    idtable[position].offset = -1;    // 并不占据空间，仅仅记录以便于分析
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 0;
                    idtable[position].extend->dlen = (int *)malloc(sizeof(int));
                    idtable[position].extend->dlen[0] = 0;
                }
                else    // 过程调用
                {
//                    printf("%d\n", SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]));
                    if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // 未定义过程
                    {
                        strcpy(errmssg, "Unfefined procedure : ");
                        strcat(errmssg, idstack.stack[idstack.top]);
                        Error(fe, row, errmssg);
                    }
                }
                break;
            case -147:
                /*语法分析*/
                printf("<function_identifier> => ID;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 88, gototable[(*sstack).stack[(*sstack).top]][88]);    // 88:刚规约出来的非终结符代号
                /*语义分析*/
                // 不弹出，建内层符号表时还需要
                if(deffunc)    // 函数定义时
                {
                    fun_or_proc = 1;
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                    porfh = position;
                    strcpy(idtable[position].id, idstack.stack[idstack.top]);
                    strcpy(idtable[position].kind, "func");
                    idtable[position].offset = -1;    // 并不占据空间，仅仅记录以便于分析
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 0;
                    idtable[position].extend->dlen = (int *)malloc(sizeof(int));
                    idtable[position].extend->dlen[0] = 0;
                }
                else    // 函数调用
                {
                    if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // 未定义函数
                    {
                        strcpy(errmssg, "Unfefined function : ");
                        strcat(errmssg, idstack.stack[idstack.top]);
                        Error(fe, row, errmssg);
                    }
                }
                break;
            case -148:
                /*语法分析*/
                printf("<variable_access> => <identified_variable>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:刚规约出来的非终结符代号
                break;
            case -149:
                /*语法分析*/
                printf("<identified_variable> => <pointer_variable> ^;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 89, gototable[(*sstack).stack[(*sstack).top]][89]);    // 89:刚规约出来的非终结符代号
                /*语义分析*/
                strcat(tmstack.stack[tmstack.top].addr, "^");    // 标记为指针
                break;
            case -150:
                /*语法分析*/
                printf("<pointer_variable> => <variable_access>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 90, gototable[(*sstack).stack[(*sstack).top]][90]);    // 90:刚规约出来的非终结符代号
                break;
            case -151:
                /*语法分析*/
                printf("<simple_statement> => <procedure_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 34, gototable[(*sstack).stack[(*sstack).top]][34]);    // 34:刚规约出来的非终结符代号
                /*语义分析*/
                tmplist4[0] = -1;
                PushListstack(&nextlist, tmplist4);    // 压入空链
//                nextlist.top ++;
                break;
            case -152:
                /*语法分析*/
                printf("<procedure_statement> => <procedure_identifier> ( <actual_parameter_list> );\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:刚规约出来的非终结符代号
                /*语义分析*/
                while(countidlis > 0)
                {
                    tmpvarone = tmstack.stack[tmstack.top];
                    PopTmpstack();
                    if(tmpvarone.index == -1)    // 未使用临时变量
                    {
                        strcpy(result,tmpvarone.addr);
                    }
                    else    // 使用临时变量
                    {
                        strcpy(result,"t");
                        itoa(tmpvarone.index, tmpic, 10);
                        temps[tmpvarone.index] = 0;    // 使用了临时变量， 归还临时变量池
                        strcat(result, tmpic);
                    }
                    GenCode(result, "param", '\0', '\0');
                    countidlis --;
                }
                strcpy(result,idstack.stack[idstack.top]);
                PopTokenstack(&idstack);
                GenCode(result,"call",'\0', '\0');
                break;
            case -153:
                /*语法分析*/
                printf("<procedure_statement> => <procedure_identifier> ( );\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:刚规约出来的非终结符代号
                /*语义分析*/
                strcpy(result,idstack.stack[idstack.top]);
                PopTokenstack(&idstack);
                GenCode(result,"call",'\0', '\0');
                break;
            case -154:
                /*语法分析*/
                printf("<actual_parameter_list> => <actual_parameter>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 92, gototable[(*sstack).stack[(*sstack).top]][92]);    // 92:刚规约出来的非终结符代号
                /*语义分析*/
                countidlis ++;
                break;
            case -155:
                /*语法分析*/
                printf("<actual_parameter_list> => <actual_parameter_list> , <actual_parameter>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 92, gototable[(*sstack).stack[(*sstack).top]][92]);    // 92:刚规约出来的非终结符代号
                /*语义分析*/
                countidlis ++;
                break;
            case -156:
                /*语法分析*/
                printf("<actual_parameter> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 93, gototable[(*sstack).stack[(*sstack).top]][93]);    // 93:刚规约出来的非终结符代号
                break;
            case -157:
                /*语法分析*/
                printf("<conditional_statement> => <case_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 64, gototable[(*sstack).stack[(*sstack).top]][64]);    // 64:刚规约出来的非终结符代号
                break;
            case -158:
                /*语法分析*/
                printf("<case_statement> => case <case_index> of <case_list_element_sequence> end;\n");    // 对应产生式
                for(i = 5; i > 0; i --)    // 5:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 94, gototable[(*sstack).stack[(*sstack).top]][94]);    // 94:刚规约出来的非终结符代号
                /*语义分析*/
                if(casenext != -1)
                {
                    tmplist4[0] = casenext;
                    Merge(caselist.stack, tmplist4, tmplist5);
                    tmplist4[0] = -1;
                }
                Backpach(tmplist5, nextcode);
                break;
            case -159:
                /*语法分析*/
                printf("<case_list_element_sequence> => <case_list_element> semi;\n");    // 对应产生式
                for(i = 2; i > 0; i --)    // 2:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 95, gototable[(*sstack).stack[(*sstack).top]][95]);    // 95:刚规约出来的非终结符代号
                /*语义分析*/
                caselist.top ++;
                caselist.stack[caselist.top] = nextcode;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -160:
                /*语法分析*/
                printf("<case_list_element_sequence> => <case_list_element_sequence> <case_list_element> semi;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 95, gototable[(*sstack).stack[(*sstack).top]][95]);    // 95:刚规约出来的非终结符代号
                /*语义分析*/
                caselist.top ++;
                caselist.stack[caselist.top] = nextcode;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -161:
                /*语法分析*/
                printf("<case_index> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 96, gototable[(*sstack).stack[(*sstack).top]][96]);    // 96:刚规约出来的非终结符代号
                /*语义分析*/
                casecore = tmstack.stack[tmstack.top];    // 存储
                PopTmpstack();
                break;
            case -162:
                /*语法分析*/
                printf("<case_list_element> => <case_constant> <case_mark_N> : <statement>;\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 97, gototable[(*sstack).stack[(*sstack).top]][97]);    // 97:刚规约出来的非终结符代号
                break;
            case -163:
                /*语法分析*/
                printf("<case_constant> => <constant>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 98, gototable[(*sstack).stack[(*sstack).top]][98]);    // 98:刚规约出来的非终结符代号
                break;
            case -164:
                /*语法分析*/
                printf("<case_mark_N> =>;\n");    // 对应产生式
                PushParstack(cstack, sstack, 99, gototable[(*sstack).stack[(*sstack).top]][99]);    // 99:刚规约出来的非终结符代号
                /*语义分析*/
                if(casenext != -1)
                {
                    tmplist4[0] = casenext;
                    Backpach(tmplist4, nextcode);
                    tmplist4[0] = -1;
                }
                casenext = nextcode;
                if(casecore.index == -1)
                {
                    strcpy(opr1, casecore.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(casecore.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    printf("CHECK in %d\n", casecore.index);
                    temps[casecore.index] = 0;    // 使用了临时变量， 归还临时变量池
                }
                strcat(opr1, "<>");
                strcat(opr1, constack.stack[constack.top]);
                PopTokenstack(&constack);
                GenCode('\0', "if", opr1, "goto");
                break;
            case -165:
                /*语法分析*/
                printf("<procedure_statement> => <io_procedure_statement>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:刚规约出来的非终结符代号
                break;
            case -166:
                /*语法分析*/
                printf("<io_procedure_statement> => read ( <read_parameter_list> );\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 100, gototable[(*sstack).stack[(*sstack).top]][100]);    // 100:刚规约出来的非终结符代号
                /*语义分析*/
                itoa(countpar, tmpic, 10);
                GenCode("SYSIN", "call", tmpic, '\0');
                countpar = 0;
                break;
            case -167:
                /*语法分析*/
                printf("<io_procedure_statement> => write ( <write_parameter_list> );\n");    // 对应产生式
                for(i = 4; i > 0; i --)    // 4:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 100, gototable[(*sstack).stack[(*sstack).top]][100]);    // 100:刚规约出来的非终结符代号
                /*语义分析*/
                itoa(countpar, tmpic, 10);
                GenCode("SYSOUT", "call", tmpic, '\0');
                countpar = 0;
                break;
            case -168:
                /*语法分析*/
                printf("<read_parameter_list> => <variable_access_sequence>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 101, gototable[(*sstack).stack[(*sstack).top]][101]);    // 101:刚规约出来的非终结符代号
                break;
            case -169:
                /*语法分析*/
                printf("<variable_access_sequence> => <variable_access>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 102, gototable[(*sstack).stack[(*sstack).top]][102]);    // 102:刚规约出来的非终结符代号
                /*语义分析*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // 使用了临时变量
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // 未使用临时变量
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // 使用了临时变量， 归还临时变量池
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -170:
                /*语法分析*/
                printf("<variable_access_sequence> => <variable_access_sequence> , <variable_access>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 102, gototable[(*sstack).stack[(*sstack).top]][102]);    // 102:刚规约出来的非终结符代号
                /*语义分析*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // 未使用临时变量
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // 使用临时变量
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // 使用了临时变量， 归还临时变量池
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -171:
                /*语法分析*/
                printf("<write_parameter_list> => <write_parameter_sequence>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 103, gototable[(*sstack).stack[(*sstack).top]][103]);    // 103:刚规约出来的非终结符代号
                break;
            case -172:
                /*语法分析*/
                printf("<write_parameter_sequence> => <expression>;\n");    // 对应产生式
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 104, gototable[(*sstack).stack[(*sstack).top]][104]);    // 104:刚规约出来的非终结符代号
                /*语义分析*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // 未使用临时变量
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // 使用临时变量
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // 使用了临时变量， 归还临时变量池
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -173:
                /*语法分析*/
                printf("<write_parameter_sequence> => <write_parameter_sequence> , <expression>;\n");    // 对应产生式
                for(i = 3; i > 0; i --)    // 3:产生式中符号个数
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 104, gototable[(*sstack).stack[(*sstack).top]][104]);    // 104:刚规约出来的非终结符代号
                /*语义分析*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // 使用了临时变量
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // 未使用临时变量
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // 使用了临时变量， 归还临时变量池
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            default:
                printf("table null\n");
                printf("act = %d\n", action[tops][tokenum]);
                printf("\t %d in stop:%d,ctop:%d\n", tokenum, tops, (*cstack).stack[(*cstack).top]);
                exit(0);
            }
        }

        else// accept
        {
            next = 1;
//            printf("%d in, sstop = %d,\n",tokenum, tops);
//            printf("act = %d\n",action[tops][tokenum]);
            printf("Accept!\n");
//            exit(0);

            /*展示符号表*/
            tmp = entridtbl;
            printTables(tmp);
        }
    }while(!next);
    return;
}

void PrintToken(TrieNode *trihead, int index, char oneword[])    // 打印单词，辅助字符表打印
{
    int i;
    if(trihead == NULL)
    {
        return;
    }
    oneword[index] = trihead->letter;
    if(trihead->position != -1)
    {
        oneword[index + 1] = '\0';
        printf("\t\t%d : %s\n", trihead->position, oneword);
    }
    for(i = 0; i < CHNUM; i++)
    {
        PrintToken(trihead->next[i], index + 1, oneword);
    }
    return;
}

void PrintTable(TrieNode *trihead)    // 打印字符表
{
    int i, index = 0;
    char oneword[TOKLEN];
    if(trihead != NULL)
    {
        for(i = 0; i < CHNUM; i++)
        {
            PrintToken(trihead->next[i], index, oneword);
        }
    }
    else
    {
        printf("No character table.\n");
    }
    return;
}

void printTables(TableHead *tmp)    // 打印所有小符号表，主要用来显示嵌套关系
{
    int i;
    if(!tmp)
    {
        return;
    }
    printf("Table:%s\n", tmp->name);
    printf("\t offset:%d\n", tmp->offset);
    printf("\t Outer: ");
    if(!tmp->outer)
    {
        printf("No\n");
    }
    else
    {
        printf("%s\n", tmp->outer->name);
    }
    printf("\t inner: %d\n",tmp->nextin);
    printf("\t id in this table:\n");
    PrintTable(tmp->trihead);
    for(i = 0; i < tmp->nextin; i++)
    {
        printTables(tmp->inner[i]);
    }
    return;
}

void Output(int code, char token[], int position, FILE *fd)    // 输出到文件和屏幕
{
    if (code == INT)    // 整型
    {
        printf("(%d,%d)\n",code,atoi(token));
        fprintf(fd,"(%d,%d)\n",code,atoi(token));
    }
    else if(code == REAL)    // 实型
    {
        printf("(%d,%f)\n",code,atof(token));
        fprintf(fd,"(%d,%f)\n",code,atof(token));
    }
    else if(code == EREAL)    // 科学记数法
    {
        printf("(%d,%E)\n",code,atof(token));
        fprintf(fd,"(%d,%E)\n",code,atof(token));
    }
    else if(code ==ID)
    {
        printf("(%d,%d)\n",code,position);
        fprintf(fd,"(%d,%d)\n",code,position);
    }
    else
    {
        printf("(%d,%s)\n",code,token);
        fprintf(fd,"(%d,%s)\n",code,token);
    }
}

void Error(FILE *fe, int row, char message[])    // 打印错误消息
{
    fprintf(fe,"ERROR:  Line %d: %s\n",row,message);
//    printf("ERROR:  Line %d: %s\n",row,message);
    return;
}
