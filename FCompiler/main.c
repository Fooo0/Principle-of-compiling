#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ���������ʣ�SearchTable �������������
// ���ű��¼ �����function��procedure�����ǵı����� ע�ⵯջ����

#define SORLEN 20    // Դ����������
#define TOKLEN 11    // ��ʶ�����������10 λ
#define KEYNUM 39     // keyword ����
#define CHNUM 37    // Ӣ����ĸ��һλ���ֵ�������trie����
#define STACKSIZE 100    // ��ջ��С��---------�ǵõ����ʵ���---------
#define STATENUM 993    // ״̬��---------�ǵõ����ʵ���---------
#define NONTERNUM 105    // ���ս������--------�ǵõ����ʵ���---------
#define CODELEN 100    // һ���м����ĳ���--------�ǵõ����ʵ���---------
#define TEMINPLAC 3    // ��ʱ�����ش�С�� λ�� �������С 100����λ��Ϊ3��
#define BASDIMEN 2    // ��������Ĭ��ά����һ�㲻����2ά�������ٷ���
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
#define READ 26    // �﷨�����׶��¼���
#define READLINE 27    // �﷨�����׶��¼���
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
#define WRITE 38    // �﷨�����׶��¼���
#define WRITELN 39    // �﷨�����׶��¼���
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
// #define P_MARK 53 ɾȥ
#define F_STOP 57
#define RANGE 58
#define COLON 59
#define ASSIGN 60
#define SEMIC 61
#define CAP 62
#define EXP 63
#define LS_BRAC 64
#define RS_BRAC 65
#define Q_MARK 66    // ���ֵ�������ս������
#define EREAL 67    // ��ѧ������ �ݲ�ʵ�� ------------------ ���Գ����б���ֱ���֣�����----------------------------//
#define ENDMARK 67    // ��Ӧ�������е�$

typedef struct TrieNode    // trie���ڵ㣺����ID��trie����
{
    char letter;    // �ڵ��ַ�����ӡ�����
    struct TrieNode *next[CHNUM];    // �ڵ�ָ��
    int position;    // ���ű���λ��
}TrieNode;

typedef struct ExtNode    // ���ű������չ���ԣ�������,function,procedureʹ��
{
    int dimen;    // ����:ά��; func/proc:��������
    int *dlen;    // ����:��ά����; func/proc:���ǵķ��ű��������ű�Ĵ洢λ��
}ExtNode;

typedef struct IDNode    // ���ű���
{
    char id[TOKLEN];    // ��ʶ��
    char kind[KINDLEN];    // ��������[������const��������var�����̣�proc��������func��������para]
    char value[TOKLEN];    // ֵ
    char type[TOKLEN];    // ����
    int offset;    // ƫ��
    ExtNode *extend;    // ��չ��
}IDNode;

typedef struct TableHead
{
    char name[TOKLEN];
    TrieNode *trihead;    // �ñ��Ӧ��trie����һ���Ӧһ��trie����
    int offset, insize, nextin;    // insize:insize:�ڲ���ű�����,�ɶ�̬��չ;nextin:Ϊ��һ����inner�е��±�
    struct TableHead *outer;
    struct TableHead **inner;    // ���insize��̬���䣬��ʼΪ10
}TableHead;

typedef struct ParStack    //�﷨����ջ
{
    int stack[STACKSIZE];
    int top;    // ջ��
}ParStack;

typedef struct SemStack    // ����ջ
{
    int offset[STACKSIZE];
    TableHead *tblhead[STACKSIZE];
    int top;    // ջ��
}SemStack;

typedef struct StrStack    // token ջ�����������
{
    char stack[STACKSIZE][TOKLEN];
    int top;    // ջ��
}StrStack;

typedef struct TypNode    // �������ͱ�ı���
{
    char name[TOKLEN];    // ����
    int width;    // ƫ��
}TypNode;

typedef struct TMP    // ��ʱ����
{
    int index;    // index��addr��������൱�����ϵ�addr����index=-1��δ������ʱ������id�͹�Լ�ɷ��ս��
    char addr[TOKLEN];
}TMP;

typedef struct TmpStack    // ��ʱ����ջ
{
    TMP stack[STACKSIZE];
    int top;
}TmpStack;

typedef struct SETVar   // �����ͱ���
{
    char base[TOKLEN];    // ����array, �����ͱ���������
    int fixepart, width, ndim;    // ��ƫ�ƣ� ��� �� ά��
    int *dlen;    // ��ά����
}SETVar;

typedef struct TupleF    // �м������Ԫ��    //--------------- �����򳤶ȼǵ��޸�----------------------------//
{
    char oprnt[TOKLEN], opr1[3 * TOKLEN], opr2[3 * TOKLEN], result[3 * TOKLEN];
}TupleF;

typedef struct ListStack
{
    int stack[STACKSIZE][LISTLEN];
    int top;
}ListStack;

int Scanner(char source[], IDNode idtable[]);    // ���ݵ�������ĸ���ú�����ȡ����
int GetToken(FILE *fs, char cho, char token[], FILE *fd, FILE *fe);    // ������ɨ�������ĸ��������ɨ��
int GetDigit(FILE *fs, char cho, char token[], FILE *fd);    // ʶ������
int GetString(FILE *fs, char cho, char token[], FILE *fd);    // ʶ���ַ���
int GetID(FILE *fs, char cho, char token[], FILE *fd);    // ʶ���ʶ�����߹ؼ���
int IsKeyW(char ch[]);    // �ж��Ƿ�Ϊ������
void InitTblH(TableHead *outer, TableHead *newtbl, char name[]);    // ��ʼ�����ű�trie��
int UpdateTable(char token[], TrieNode *trihead, IDNode *idtable);    // ���·��ű�
int SearchTable(char setname[], TableHead *tbhead);    // ����ű�
void InitStack(ParStack *cstack, ParStack *sstack, SemStack *tnfstack);    // ��ʼ��ջ
void PushParStack(ParStack *cstack, ParStack *sstack, int tokenum, int state);    // ѹջ��״̬ջ�ͷ���ջ
void PopParStack(ParStack *cstack, ParStack *sstack);    // ��ջ��״̬ջ�ͷ���ջ
void PushSemstack(SemStack *tnfstack, TableHead *tblhead);    // ѹջ,���ű��ƫ��ջ
//int UpdateTyp(TypNode typtbl[], char token[], int amount);    // �������ͱ�
int GetType(TypNode typtbl[], char token[]);    // ��ȡ���������ͱ��е�λ��
void PopTokenstack(StrStack *tokenstack);    // ��ջ��idջ��������������ã�ѹջ��δ�躯��
void GenCode(char result[], char oprnt[], char opr1[], char opr2[]);    // �����м����
int NewTemp();    // ��ȡ�µ���ʱ����
void PushTmpstack(int index, char addr[TOKLEN]);    // ѹջ�� ��ʱ����ջ
void PopTmpstack();    // ��ջ�� ��ʱ����ջ
void PushListstack(ListStack *onelist,int *list);    // ѹջ�� listջ����ջ�򵥣�����Ҫ�躯��
void Backpach(int list[], int mq);    // ����
void Merge(int *onelist, int *twolist, int relist[]);    // �ϲ�
void Parser(int tokenum, char token[], IDNode idtable[], TypNode typtbl[],
           ParStack *sstack, ParStack *cstack, SemStack *tnfstack,
           int gototable[][NONTERNUM], int action[][ENDMARK], FILE *fe);    // �﷨����
void PrintToken(TrieNode *trihead, int index, char oneword[]);    // ��ӡ���ʣ������ַ����ӡ
void PrintTable(TrieNode *trihead);    // ��ӡĳ��С���ű�
void printTables(TableHead *tmp);    // ��ӡ����С���ű���Ҫ������ʾǶ�׹�ϵ
void Output(int code, char token[], int position, FILE *fd);    // ������ļ�����Ļ
void Error(FILE *fe, int row, char message[]);    // ��ӡ������Ϣ

int row = 1, tablei = 0, typti = 0;    // row����������¼���ű�ʹ�õ�����±꣬tablei:��¼���ű���һ������λ�ã�typti:��¼���ͱ���һ������λ��, countidlis:��¼idջ��
int countidlis = 0;    // ��¼idlist���ȣ���Ҫ���ڱ���������һϵ��ͬ���ͱ���
int idtabsize = 500, typtblsize = 50;    // idtabsize�����ű��С����̬�仯��, typtblsize: ���ͱ��С����̬�仯��
int codenum = 200, nextcode = 0;    // �ֱ�Ϊ�м������������һ������λ��
int tnum = 50, *temps, maxt = -1;    // ��ʱ�����ؼ����С, Ŀǰ����������غ�
int porfh, fun_or_proc = 0;    // porfh:���procedure ���� function �����ͷ����λ��
int deffunc = 0, defproc = 0;    // ���̻��ߺ��� �ڶ��廹���ڵ��õı�־��0:����;1:����;
int countpar = 0, arryflag = 0, casenext = -1;    // countpar:��¼����/�������������;arryflag:���һ������Ĺ�Լ
TableHead *entridtbl = NULL;    // ���ű���ڣ�ָ����������ű�
//char **mcodes;    // �洢�м����
TupleF *mcodes;    // �洢�м����
ExtNode *settype = NULL;    // ����ʱ����������
SETVar setypevar;    // ��������ʱ����
char keywords[][10] = {"and","array","begin","case","const","div","do","downto","else","end",
                        "file","for","function","goto","if","in","label","mod","nil","not","of",
                        "or","packed","procedure","program","read","readln","record","repeat",
                        "set","then","to","type","until","var","while","with","write","writeln"};
StrStack idstack, constack, oprstack, tmpoprstack;    // token ջ�����������(ID, constant, operator),tmpoprstack: ��ʱ�洢����������ȼ�ԭ��
TmpStack tmstack;    // ��ʱ����ջ�����������
ListStack truelist, falselist, nextlist;    // �������������
ParStack Mquad, Magain, for_down, caselist;    // for_down: ����for�е�to �� downto
TMP casecore;    // case �Ƚϵĺ���

int main()
{
    int i, j;
    char source[SORLEN];
    IDNode idtable[idtabsize];    // �̶��������ٷ���

    printf("Source program:");
    scanf("%s",source);

    Scanner(source, idtable);    // ɨ����
    printf("idtable:\n");
    for(i = 0; i < tablei; i++)    // ��ӡ���ű�
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
    for(i = 0; i < nextcode; i ++)    // ��ӡ�м����
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

int Scanner(char source[], IDNode idtable[])    // ���ݵ�������ĸ���ú�����ȡ����
{
    char token[TOKLEN]={'\0'}, ch;
    int tokenum, i, j, actitem, action[STATENUM][ENDMARK], goitem, gototable[STATENUM][NONTERNUM];    // next:��ʶ�Ƿ����һ���ַ���prorlset�����ַ����в�ֳ��Ĳ���ʽ��ÿ���Ҳ����ս���ս����
    FILE *fs = NULL, *fd = NULL, *fe = NULL, *fa = NULL, *fg = NULL, *ft = NULL;    // �����ļ�ָ��
    TypNode typtbl[typtblsize];
    ParStack sstack, cstack;    // ״̬��tokenջ
    SemStack tnfstack;    // ���ű��offset��ջ
    /*������ļ�*/
    if((fs = fopen(source,"r")) == NULL)    // ��Դ����
    {
        printf("Fail to open source program %s!\n",source);
    }
    if((fd = fopen("firststep.txt","w")) == NULL)    // �򿪴ʻ��������ļ�
    {
        printf("Fail to open destination file firststep.txt!\n");
        exit(0);
    }
    if((fa = fopen("action.txt","r")) == NULL)    // ��action ���ļ�
    {
        printf("Fail to open the file action.txt!\n");
        exit(0);
    }
    if((fg = fopen("goto.txt","r")) == NULL)    // ��goto ���ļ�
    {
        printf("Fail to open the file goto.txt!\n");
        exit(0);
    }
    if((ft = fopen("type.txt","r")) == NULL)    // �򿪴�������ļ�
    {
        printf("Fail to open the file type.txt!\n");
        exit(0);
    }
    if((fe = fopen("error.txt","w")) == NULL)    // �򿪴�������ļ�
    {
        printf("Fail to open destination file firststep.txt!\n");
        exit(0);
    }
    /*����action��*/
    while(!feof(fa))
    {
        fscanf(fa,"%d %d %d", &i, &j, &actitem);
        action[i][j] = actitem;
    }
    /*����goto��*/
    while(!feof(fg))
    {
        fscanf(fg,"%d %d %d", &i, &j, &goitem);
        gototable[i][j] = goitem;
    }
    /*�������ͱ�*/
    typti = 0;
    while(!feof(ft))
    {
        fscanf(ft,"%s",typtbl[typti].name);
        fscanf(ft,"%s",token);
        typtbl[typti].width = atoi(token);
        typti ++;
    }
    /*��ʼ��*/
    InitStack(&cstack, &sstack, &tnfstack);    // ��ʼ��ջ
    mcodes = (TupleF*)malloc(codenum * sizeof(TupleF));    // ��ʼ���м����洢��
    temps = (int *)malloc(tnum * sizeof(int));    // ��ʼ����ʱ������------------�Ƿ��ʼĬ��Ϊ0-----------------//
    settype = (ExtNode *)malloc(sizeof(ExtNode));
    settype->dimen = 0;
    settype->dlen = (int *)malloc(BASDIMEN * sizeof(int));
    /*��Դ�ļ��ļ�*/
    while(!feof(fs))
    {
        ch = fgetc(fs);
        memset(token, '\0', TOKLEN);    // ���ԭ����
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
            tokenum = GetToken(fs, ch, token, fd, fe);    // �ʷ�������
            //------��������﷨������-----//
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

int GetToken(FILE *fs, char cho, char token[], FILE *fd, FILE *fe)    // ������ɨ�������ĸ��������ɨ��
{
    char ch = cho;

    if(isdigit(ch))
    {
        return GetDigit(fs, ch, token, fd);    // ʶ������
    }
    else if(ch == '\'')
    {
        return GetString(fs, ch, token, fd);    // ʶ���ַ���
    }
    else if(isalpha(ch) || ch == '_')
    {
        return GetID(fs, ch, token, fd);    // ʶ�� ID ���߹ؼ���
    }
    else
    {
        switch (ch)
        {    // case�����break Ҳ��ʡ���ˣ�yinweireturn�Ĵ���...
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
                if(!feof(fs))    // ֻ��һ������ΪPASCAL ĩβ�ԡ�end.������
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
        case '{':    // ע��
            ch = fgetc(fs);
            if(ch == '*')    // ����ע��
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
                            break;    // ע�ͽ���
                        }
                        else
                        {
                            fseek(fs,-1,SEEK_CUR);
                        }
                    }
                }
            }
            else    // ����ע��
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
            return -1;    // ------------------------ע�������----------------------------//
        }
    }
    return 0;    // ֻ��ע�ͲŻ�ִ�е���
}

int GetDigit(FILE *fs, char cho, char token[], FILE *fd)    // ʶ������
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
                Output(INT, token, 0, fd);    // ʶ�������:��0,��״ֻ̬��ʶ�������
                return INT;
            }
            if(!cut && index > TOKLEN - 2)    // �ض�
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
                Output(INT, token, 0, fd);    // ʶ�������:0
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
            if(!cut && index > TOKLEN - 2)    // �ض�
            {
                cut = 1;
                token[index-2] = '\0';// �ص���.��֮ǰ
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
                Output(REAL, token, 0, fd);    // ʶ���ʵ��,��״ֻ̬��ʶ����ǿ�ѧ������ʵ��
                return REAL;
            }
            if(!cut && index > TOKLEN - 2)    // �ض�
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
            if(!cut && index > TOKLEN - 2)    // �ض�
            {
                cut = 1;
                token[index-2] = '\0';    // �ص� ��E��֮ǰ
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
            if(!cut && index > TOKLEN - 2)    // �ض�
            {
                cut = 1;
                token[index-3] = '\0';    // �ص�'E'֮ǰ
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
                Output(EREAL, token, 0, fd);    // ʶ�����ѧ��������ʽʵ��,��״ֻ̬��ʶ�����������
                return EREAL;    //---------��ѧ������-----�п�����-------//
            }
            if(!cut && index > TOKLEN - 2)    // �ض�
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

int GetString(FILE *fs, char cho, char token[], FILE *fd)    // ʶ���ַ���
{
    char ch = cho;
    int index = 0, cut = 0;

    token[index] = '\'';
    index ++;
    ch = fgetc(fs);
    while(ch != '\'')
    {
        if(index <= TOKLEN - 2)    // ���������
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
    Output(STRING, token, 0, fd);    // ʶ����ַ���
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
            // �������������Ϊ break�� �������޸Ĺ������������return���ʲ���break
        }
        if(index > TOKLEN - 2)    // �ض�
        {
            cut = 1;
            token[TOKLEN - 1] = '\0';
            // ��ʱһ�����ǹؼ��֣���Ϊ�ؼ��ֲ��ᱻ�ض�....
        }
    }
    return 0;    // Ҳ���ᱻִ�е�
}

int IsKeyW(char chs[])    // �ж��Ƿ�Ϊ������
{
    int low = 0, mid , high = KEYNUM - 1;    // ���ö��ֲ���

    if(strlen(chs) > 9)
    {
        return 0;
    }

    mid = (low + high)/2;
    while(low < high && strcasecmp(keywords[mid],chs))    // PACAL �����ִ�Сд
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
    if(!strcasecmp(keywords[mid],chs))    // �� strcasecmp ����0�����ڹؼ��ֱ����ҵ��ո�ʶ�����ID
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

void InitTblH(TableHead *outer, TableHead *newtbl, char name[])    // ��ʼ�����ű��ͷ
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

int UpdateTable(char token[], TrieNode *trihead, IDNode *idtable)    // ���·��ű�
{
    TrieNode *tmp = trihead;
    int i,j,in;
    for(i = 0; token[i] != '\0'; i++)
    {
        if(isalpha(token[i]))
        {
            in = (int)token[i] - 87;    // PASCAL ���ִ�Сд
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
            TrieNode *newnode =  (TrieNode *)malloc(sizeof(TrieNode));    // ��̬�ڴ���䣬��trie�����
            for(j = 0; j < CHNUM; j++)
            {
                newnode->next[j] = NULL;    // ������
            }
            newnode->letter = token[i];
            newnode->position = -1;
            tmp->next[in] = newnode;
        }
        tmp = tmp->next[in];
    }
    if(tmp->position == -1)
    {
        tmp->position = tablei;    // ���ű��д洢��ID ���±�
        strcpy(idtable[tablei].id, token);
        idtable[tablei].extend = NULL;
        tablei ++;    // ��һ�� ID �Ĵ洢λ��
        if(tablei >= idtabsize)    // ������ű�
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
    return tmp->position;    // �����Ѿ�����
}

int SearchTable(char setname[], TableHead *tbhead)    // ����ű�
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
            in = (int)setname[i] - 87;    // PASCAL ���ִ�Сд
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

void InitStack(ParStack *cstack, ParStack *sstack, SemStack *tnfstack)    // ��ʼ������ջ
{
    /*״̬ջ�ͷ���ջ*/
    cstack->stack[0] = ENDMARK;
    sstack->stack[0] = 0;
    cstack->top = 0;
    sstack->top = 0;
    /*���ű��ƫ��ջ*/
    tnfstack->top = -1;
    /*idջ*/
    idstack.top = -1;
    /*constant ջ*/
    constack.top = -1;
    /*operator ջ*/
    oprstack.top = -1;
    tmpoprstack.top = -1;
    /*��ʱ���� ջ*/
    tmstack.top = -1;
    /*list ջ*/
    truelist.top = -1;    // truelist
    memset(truelist.stack, -1, sizeof(truelist.stack));
    falselist.top = -1;    // falselist
    memset(falselist.stack, -1, sizeof(falselist.stack));
    nextlist.top = -1;    // nextlist
    memset(nextlist.stack, -1, sizeof(nextlist.stack));
    /*Mquad ջ*/
    Mquad.top = -1;
    /*again ջ*/
    Magain.top = -1;
    /*for_down ջ*/
    for_down.top = -1;    // ֮������ջ����Ϊ���ܳ���Ƕ��ѭ��
    /*caseջ*/
    caselist.top = -1;
    memset(caselist.stack, -1, sizeof(caselist.stack));
    return;
}

void PushParstack(ParStack *cstack, ParStack *sstack, int tokenum, int state)    // ѹջ��״̬ջ�ͷ���ջ
{
    (*cstack).top ++;
    (*sstack).top ++;
    (*cstack).stack[(*cstack).top] = tokenum;
    (*sstack).stack[(*sstack).top] = state;
}

void PopParstack(ParStack *cstack, ParStack *sstack)    // ��ջ��״̬ջ�ͷ���ջ
{
    (*cstack).top --;    // ����ջ��ջ
    (*sstack).top --;    // ״̬ջ��ջ
}

void PushSemstack(SemStack *tnfstack, TableHead *tblhead)    // ѹջ�����ű��ƫ�ƣ���ջ��δ�躯��
{
    tnfstack->top ++;
    tnfstack->tblhead[tnfstack->top] = tblhead;
    tnfstack->offset[tnfstack->top] = 0;
}

void PopTokenstack(StrStack *tokenstack)    // ��ջ��(id,constant,opratpr)��������������ã�ѹջ��δ�躯��
{
    memset(tokenstack->stack[tokenstack->top], '\0', TOKLEN);    // ���ԭλ��
    tokenstack->top --;
    return;
}

//int UpdateTyp(TypNode typtbl[], char token[], int amount)    // �������ͱ�
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

int GetType(TypNode typtbl[], char token[])    // ��ȡ���������ͱ��е�λ��
{
    int i = 0;
    while(i < typti)
    {
        if(strlen(typtbl[i].name) != strlen(token))
        {
            i ++;
            continue;
        }
        if(!strcasecmp(typtbl[i].name, token))    // ��ͬ
        {
            return i;
        }
        i ++;
    }
    return -1;
}

void GenCode(char result[], char oprnt[], char opr1[], char opr2[])    // �����м����
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

int NewTemp()    // ��ȡ�µ���ʱ����
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
            for(i = 0;i <= tnum / 2; i ++)    // --------------int�����ʼ�Ƿ�Ĭ��0-----------------------//
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

void PushTmpstack(int index, char addr[TOKLEN])    // ѹջ�� ��ʱ����ջ
{
    tmstack.top ++;
    tmstack.stack[tmstack.top].index = index;
    if(addr)
    {
        strcpy(tmstack.stack[tmstack.top].addr, addr);
    }
    return;
}

void PopTmpstack()    // ��ջ�� ��ʱ����ջ
{
    memset(tmstack.stack[tmstack.top].addr, '\0', TOKLEN);    // ���ԭλ��
    tmstack.top --;
    return;
}

void PushListstack(ListStack *onelist, int *list)    // ѹջ�� listջ
{
    int i = 0, tmp = 0;
    onelist->top ++;
    if(onelist->stack[onelist->top][i] != -1)
    {
//        memset(onelist->stack[onelist->top], -1, LISTLEN);    // ���ԭ�������ݣ���ȻӰ���Ժ��
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
        list[i] = -1;    // ���ԭ�������ݣ���ȻӰ���Ժ��
        i ++;
    }
    return;
}

void Backpach(int list[], int mq)    // ����
{
    int i = 0;
    char row[TOKLEN];
    itoa(mq, row, 10);
    while(list[i] != -1)
    {
        strcpy(mcodes[list[i]].result, row);
        list[i] = -1;    // ���
        i ++;
    }
    return;
}

void Merge(int *onelist, int *twolist, int relist[])    // �ϲ�
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
            int gototable[][NONTERNUM], int action[][ENDMARK], FILE *fe)    // �﷨����
{
    int i, j, tops, next = 1, act,position, typos, amount;    // next��ʶ�Ƿ�������token����������Լ��δ���´λ�Ӧ�ٶ�
    int tmpi, mq1, mq2, width;    // tmpi:ʹ�õ���ʱ�������±�����ͱ�ʾ, mqx: mquad��ʱ�洢
    int *tmplist1 = NULL, *tmplist2 = NULL, *tmplist3 = NULL;    // list��ʱ�洢��
    int tmplist4[LISTLEN], tmplist5[LISTLEN];    // list��ʱ�洢��, ʹ��֮����Ҫ��ԭ�� ��Ȼ����Ӱ��֮���
    char tmpic[TEMINPLAC], tmpich[TEMINPLAC];    // (�����м��������)tmpic,tmpich:ʹ�õ���ʱ�������±���ַ�����ʾ,
    char constyp[TOKLEN], type[TOKLEN], tmpopr[TOKLEN];    // constyp: constant �����ͣ�����ʱ��;tmpopr:��ʱ�洢�����
    char result[TOKLEN], oprnt[TOKLEN], opr1[TOKLEN], opr2[TOKLEN]; // �м���룺��Ԫ��Ԫ��
    char errmssg[ERRLEN];    // ������Ϣ
    TableHead *inhead, *outhead, *tmp;
    TMP tmpvarone, tmpvartwo, tmpvarthree;    // �ݴ���ʱ����ջ�����ģ� ������

    memset(tmplist4, -1, sizeof(tmplist4));    // ��ʼ��
    memset(tmplist5, -1, sizeof(tmplist5));    // ��ʼ��


    if(!strcmp(token, "downto"))
    {
        for_down.top ++;
        for_down.stack[for_down.top] = 1;
    }
    else if((tokenum == ID && !IsKeyW(token)))    // ���id��������Ӱ���·��ű���ʱ������
    {
        idstack.top ++;
        strcpy(idstack.stack[idstack.top], token);
    }
    else if(tokenum == REAL || tokenum == INT || tokenum == STRING || tokenum == NIL)    // ��� (unsigned)constant �Լ� constant�����ͣ���ʽ���Ƴ���
    {
        constack.top ++;
        strcpy(constack.stack[constack.top], token);
    }
    else if((tokenum >= PLUS && tokenum <=NE) || tokenum == AND || tokenum == OR || tokenum == DIV || tokenum == MOD)    // ������
    {
        oprstack.top ++;
        strcpy(oprstack.stack[oprstack.top], token);
    }
    do
    {
        // ----------------------debug �ã��ǵ�ɾ�� --------------------------------------//
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
        tops = (*sstack).stack[(*sstack).top];    // sstack��ջ��
        act = action[tops][tokenum];    // ��action��
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
                printf("<program>' => <program>;\n");    // ��Ӧ����ʽ
                break;
            case -2:
                /*�﷨����*/
                printf("<program> => <program_heading> semi <main_idtable> <program_block>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 0, gototable[sstack->stack[sstack->top]][0]);    // 0:�չ�Լ�����ķ��ս������
                /*�������*/
                (tnfstack->tblhead[tnfstack->top]) -> offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;
                printf("<program>' => <program>;\n");
                GenCode('\0', "end", '\0', '\0');
                break;
            case -3:
                printf("<program_heading> => program ID;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 1, gototable[(*sstack).stack[(*sstack).top]][1]);    // 1:�չ�Լ�����ķ��ս������
                break;
            case -4:
                printf("<program_heading> => program ID ( <program_parameters> );\n");    // ��Ӧ����ʽ
                for(i = 5; i > 0; i --)    // 5:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 1, gototable[(*sstack).stack[(*sstack).top]][1]);    // 1:�չ�Լ�����ķ��ս������
                break;
            case -5:
                /*�﷨����*/
                printf("<program_parameters> => <identifier_list>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 2, gototable[(*sstack).stack[(*sstack).top]][2]);    // 2:�չ�Լ�����ķ��ս������
                /*�������*/
                while(countidlis > 0)    // ���ã�����
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -6:
                /*�﷨����*/
                printf("<identifier_list> => <identifier_list> , ID;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 3, gototable[(*sstack).stack[(*sstack).top]][3]);    // 3:�չ�Լ�����ķ��ս������
                /*�������*/
                countidlis ++;    // ��¼idlist���ȣ���Ҫ���ڱ���������һϵ��ͬ���ͱ���
                break;
            case -7:
                /*�﷨����*/
                printf("<identifier_list> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 3, gototable[(*sstack).stack[(*sstack).top]][3]);    // 3:�չ�Լ�����ķ��ս������
                /*�������*/
                countidlis ++;
                break;
            case -8:
                printf("<main_idtable> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 4, gototable[sstack->stack[sstack->top]][4]);    // 4:�չ�Լ�����ķ��ս������
                /*�������*/
                TableHead *maintable = (TableHead *)malloc(sizeof(TableHead));
                InitTblH(NULL, maintable, idstack.stack[idstack.top]);    // ----- �ܲ��ܵ�������Ϊ���ֻ�����------//
                PopTokenstack(&idstack);
                PushSemstack(tnfstack, maintable);
                entridtbl = maintable;
                break;
            case -9:
                /*�﷨����*/
                printf("<program_block> => <block> . ;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 5, gototable[(*sstack).stack[(*sstack).top]][5]);    // 5:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -10:
                printf("<block> => <constant_definitions> <variable_declarations> <procedure_function_declarations> <statement_part>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 6, gototable[(*sstack).stack[(*sstack).top]][6]);    // 6:�չ�Լ�����ķ��ս������
                break;
            case -11:
                printf("<constant_definitions> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 7, gototable[(*sstack).stack[(*sstack).top]][7]);    // 7:�չ�Լ�����ķ��ս������
                break;
            case -12:
                printf("<constant_definitions> => const <constant_definition_sequence> semi;\n");
                for(i = 3; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 7, gototable[(*sstack).stack[(*sstack).top]][7]);    // 7:�չ�Լ�����ķ��ս������
                break;
            case -13:
                printf("<constant_definition_sequence> => <constant_definition_sequence> semi <constant_definition>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 8, gototable[(*sstack).stack[(*sstack).top]][8]);    // 8:�չ�Լ�����ķ��ս������
                break;
            case -14:
                printf("<constant_definition_sequence> => <constant_definition>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 8, gototable[(*sstack).stack[(*sstack).top]][8]);    // 8:�չ�Լ�����ķ��ս������
                break;
            case -15:
                /*�﷨����*/
                printf("<constant_definition> => ID = <constant>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 9, gototable[(*sstack).stack[(*sstack).top]][9]);    // 9:�չ�Լ�����ķ��ս������
                /*�������*/
                position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                PopTokenstack(&idstack);    // ʹ��֮�󵯳�

                strcpy(idtable[position].value, constack.stack[constack.top]);    // ��ֵ�������
                strcpy(idtable[position].kind, "const");    // ���·��ű��� ���ŵ�����
                idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                typos = GetType(typtbl, constyp);
                if(typos == -1)
                {
                    printf("Undefined type: %s\n", constyp);
                    typos = 0;
                }
                if(!strcasecmp(constyp, "char"))    // �ַ�������Ϊ�ַ�����
                {
                    strcpy(type, "[");
                    strcat(type, constyp);
                    strcpy(idtable[position].type, type);    // ���·��ű��� ���ŵ�����
                    amount = strlen(constack.stack[constack.top]);    // �����С  ------ �ǵü��׼ȷ��--------------//
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 1;
                    idtable[position].extend->dlen = (int *)malloc(1 * sizeof(int));
                    idtable[position].extend->dlen[0] = amount - 2;    // ��ȥ����

                    tnfstack->offset[tnfstack->top] += (amount - 2) * typtbl[typos].width;    // ����ջ��ƫ��
                }
                else
                {
                    strcpy(idtable[position].type, constyp);    // ���·��ű��� ���ŵ�����
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // ����ջ��ƫ��
                }
                PopTokenstack(&constack);    // ʹ��֮�󵯳�
                break;
            case -16:
                /*�﷨����*/
                printf("<constant> => INT;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(constyp, "integer");
                break;
            case -17:
                /*�﷨����*/
                printf("<constant> => REAL;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(constyp, "real");
                break;
            case -18:
                /*�﷨����*/
                printf("<constant> => STRING;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 10, gototable[(*sstack).stack[(*sstack).top]][10]);    // 10:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(constyp, "char");
                break;
            case -19:
                printf("<variable_declarations> =>;\n");
                PushParstack(cstack, sstack, 11, gototable[(*sstack).stack[(*sstack).top]][11]);    // 11:�չ�Լ�����ķ��ս������
                break;
            case -20:
                printf("<variable_declarations> => var <variable_declaration_sequence> semi;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 11, gototable[(*sstack).stack[(*sstack).top]][11]);    // 11:�չ�Լ�����ķ��ս������
                break;
            case -21:
                printf("<variable_declaration_sequence> => <variable_declaration_sequence> semi <variable_declaration>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 12, gototable[(*sstack).stack[(*sstack).top]][12]);    // 12:�չ�Լ�����ķ��ս������
                break;
            case -22:
                printf("<variable_declaration_sequence> => <variable_declaration>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 12, gototable[(*sstack).stack[(*sstack).top]][12]);    // 12:�չ�Լ�����ķ��ս������
                break;
            case -23:
                /*�﷨����*/
                printf("<variable_declaration> => <identifier_list> : <type_denoter>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 13, gototable[(*sstack).stack[(*sstack).top]][13]);    // 13:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(type, idstack.stack[idstack.top]);
                if(idstack.stack[idstack.top][0] == '^')    // ָ������
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
                typos = GetType(typtbl, type);    // �������������ͱ��е�λ��
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                if(idstack.stack[idstack.top][0] == '^')    // ָ������
                {
                    strcpy(type, idstack.stack[idstack.top]);
                    width = 4;
                }
                else
                {
                    width = typtbl[typos].width;
                }
                PopTokenstack(&idstack);    // ����������
                if(settype->dimen == 0)    // ����������
                {
                    for(i = countidlis - 1; i >= 0;i --)    // һ�����������ͺ�ƫ��ֵ
                    {
                        position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);    // ���²���ȡ�ڷ��ű��е�λ��,��˳������-i
                        strcpy(idtable[position].kind, "var");    // ����
                        strcpy(idtable[position].type, type);    // ����
                        idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                        tnfstack->offset[tnfstack->top] += width;    // ����ջ��ƫ��
                    }
                }
                else    // ��������
                {
                    strcpy(constyp, "[");
                    strcat(constyp, type);
                    amount = 1;
                    for(j = 0; j < settype->dimen; j++)
                    {
                        amount *= settype->dlen[j];
                    }
                    amount *= typtbl[typos].width;
                    for(i = countidlis - 1; i >= 0;i --)    // һ�����������ͺ�ƫ��ֵ
                    {
                        position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);    // ���²���ȡ�ڷ��ű��е�λ��,��˳������-i
                        strcpy(idtable[position].kind, "var");    // ����
                        strcpy(idtable[position].type, constyp);    // ����
                        idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                        idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));    // ��չ����
                        idtable[position].extend->dimen = settype->dimen;    // ά��
                        idtable[position].extend->dlen = (int *)malloc(settype->dimen * sizeof(int));    // ��ά����
                        for(j = 0; j < settype->dimen; j++)
                        {
                            idtable[position].extend->dlen[j] = settype->dlen[j];
                            tnfstack->offset[tnfstack->top] += amount;    // ����ջ��ƫ��
                        }
                        tnfstack->offset[tnfstack->top] -= amount;    // ����
                    }
                    settype->dimen = 0;
                }
                while(countidlis > 0)    // �����Ѿ�����������ķ���
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -24:
                printf("<type_denoter> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 14, gototable[(*sstack).stack[(*sstack).top]][14]);    // 14:�չ�Լ�����ķ��ս������
                break;
            case -25:
                printf("<procedure_function_declarations> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 15, gototable[(*sstack).stack[(*sstack).top]][15]);    // 15:�չ�Լ�����ķ��ս������
                break;
            case -26:
                printf("<procedure_function_declarations> => <procedure_function_declaration_sequence>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 15, gototable[(*sstack).stack[(*sstack).top]][15]);    // 15:�չ�Լ�����ķ��ս������
                break;
            case -27:
                printf("<procedure_function_declaration_sequence> => <procedure_function_declaration>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 16, gototable[(*sstack).stack[(*sstack).top]][16]);    // 16:�չ�Լ�����ķ��ս������
                break;
            case -28:
                printf("<procedure_function_declaration_sequence> => <procedure_function_declaration_sequence> <procedure_function_declaration>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 16, gototable[(*sstack).stack[(*sstack).top]][16]);    // 16:�չ�Լ�����ķ��ս������
                break;
            case -29:
                /*�﷨����*/
                printf("<procedure_function_declaration> => <function_declaration>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 17, gototable[(*sstack).stack[(*sstack).top]][17]);    // 17:�չ�Լ�����ķ��ս������
                /*�������*/
                GenCode('\0', "ret", '\0', '\0');
                break;
            case -30:
                /*�﷨����*/
                printf("<function_declaration> => <function_heading> semi <inner_idtable> <function_block>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 18, gototable[(*sstack).stack[(*sstack).top]][18]);    // 18:�չ�Լ�����ķ��ս������
                /*�������*/
                inhead = tnfstack->tblhead[tnfstack->top];
                inhead->offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;    // ����ű��ɣ�����ջ
                outhead = tnfstack->tblhead[tnfstack->top];
                outhead->inner[outhead->nextin] = inhead;
                idtable[porfh].extend->dlen[0] = outhead->nextin;    // ���ű��¼�б�Ǹú������ű��ͷ�������ĸ� �ڲ�ָ��洢
                outhead->nextin ++;
                if(outhead->nextin >= outhead->insize)    // ��̬����
                {
                    outhead->insize *= 2;
                    TableHead **newinner = (TableHead **)malloc(sizeof(TableHead *) * outhead->insize);
                    for(i = 0; i < outhead->nextin; i ++)    // ����
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
                /*�﷨����*/
                printf("<function_heading> => function <function_identifier> : <type_identifier>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 19, gototable[(*sstack).stack[(*sstack).top]][19]);    // 19:�չ�Լ�����ķ��ս������
                /*�������*/
                deffunc = 0;
                strcpy(idtable[porfh].type, idstack.stack[idstack.top]);    // ��������ֵ����
                // PopTokenstack(&idstack);    ������������ʹ��
                typos = GetType(typtbl, idtable[porfh].type);    // �������������ͱ��е�λ��
                if(typos == -1)
                {
                    strcpy(errmssg, "Unfefined type :");
                    strcat(errmssg, idtable[porfh].type);
                    Error(fe, row, errmssg);
                }
                break;
            case -32:
                /*�﷨����*/
                printf("<function_heading> => function <function_identifier> ( <formal_parameter_list> ) : <type_identifier>;\n");    // ��Ӧ����ʽ
                for(i = 7; i > 0; i --)    // 7:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 19, gototable[(*sstack).stack[(*sstack).top]][19]);    // 19:�չ�Լ�����ķ��ս������
                /*�������*/
                deffunc = 0;
                strcpy(idtable[porfh].type, idstack.stack[idstack.top]);    // ��������ֵ����
                // PopTokenstack(&idstack);    ������������ʹ��
                typos = GetType(typtbl, type);    // �������������ͱ��е�λ��
                if(typos == -1)
                {
                    strcpy(errmssg, "Unfefined type :");
                    strcat(errmssg, idtable[porfh].type);
                    Error(fe, row, errmssg);
                }
                break;
            case -33:
                printf("<type_identifier> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 20, gototable[(*sstack).stack[(*sstack).top]][20]);    // 20:�չ�Լ�����ķ��ս������
                break;
            case -34:
                printf("<formal_parameter_list> => <formal_parameter_section>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 21, gototable[(*sstack).stack[(*sstack).top]][21]);    // 21:�չ�Լ�����ķ��ս������
                break;
            case -35:
                printf("<formal_parameter_list> => <formal_parameter_list> semi <formal_parameter_section>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 21, gototable[(*sstack).stack[(*sstack).top]][21]);    // 21:�չ�Լ�����ķ��ս������
                break;
            case -36:
                printf("<formal_parameter_section> => <value_parameter_specification>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 22, gototable[(*sstack).stack[(*sstack).top]][22]);    // 22:�չ�Լ�����ķ��ս������
                break;
            case -37:
                printf("<formal_parameter_section> => <variable_parameter_specification>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 22, gototable[(*sstack).stack[(*sstack).top]][22]);    // 22:�չ�Լ�����ķ��ս������
                break;
            case -38:
                /*�﷨����*/
                printf("<value_parameter_specification> => <identifier_list> : <type_identifier>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 23, gototable[(*sstack).stack[(*sstack).top]][23]);    // 23:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(type, idstack.stack[idstack.top]);
                typos = GetType(typtbl, type);    // �������������ͱ��е�λ��
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                PopTokenstack(&idstack);    // ����������
                idtable[porfh].extend->dimen += countidlis;    // �������ߺ���������ļ�¼λ�ü�¼ ��������
//                printf("%d: +%d\n", porfh, idtable[porfh].extend->dimen);
//                exit(0);
                for(i = countidlis - 1; i >= 0;i --)    // һ�����������ͺ�ƫ��ֵ
                {
                    // ���²���ȡ�ڷ��ű��е�λ��,��˳������-i
                    position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "para");    // ����
                    strcpy(idtable[position].type, type);    // ����
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // ����ջ��ƫ��
                }
                while(countidlis > 0)    // ���ã�����
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -39:
                /*�﷨����*/
                printf("<variable_parameter_specification> => var <identifier_list> : <type_identifier>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 24, gototable[(*sstack).stack[(*sstack).top]][24]);    // 24:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(type, idstack.stack[idstack.top]);
                typos = GetType(typtbl, type);    // �������������ͱ��е�λ��
                if(typos == -1)
                {
                    printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                    exit(0);
                }
                PopTokenstack(&idstack);    // ����������
                idtable[porfh].extend->dimen += countidlis;    // �������ߺ���������ļ�¼λ�ü�¼ ��������
                constyp[0] = 'v';
                for(i = 0; i < strlen(type); i ++)
                {
                    constyp[i + 1] = type[i];
                }
                constyp[i + 1] = '\0';
                for(i = countidlis - 1; i >= 0;i --)    // һ�����������ͺ�ƫ��ֵ
                {
                    // ���²���ȡ�ڷ��ű��е�λ��,��˳������-i
                    position = UpdateTable(idstack.stack[idstack.top - i], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "para");    // ����
                    strcpy(idtable[position].type, constyp);    // ����
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // ����ջ��ƫ��
                }
                while(countidlis > 0)    // ���ã�����
                {
                    PopTokenstack(&idstack);
                    countidlis --;
                }
                break;
            case -40:
                /*�﷨����*/
                printf("<inner_idtable> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 25, gototable[(*sstack).stack[(*sstack).top]][25]);    // 25:�չ�Լ�����ķ��ս������
                /*�������*/
                TableHead *innertable = (TableHead *)malloc(sizeof(TableHead));
                if(fun_or_proc)
                {
                    fun_or_proc = 0;
                    InitTblH(tnfstack->tblhead[tnfstack->top], innertable, idstack.stack[idstack.top - 1]);    // ----- �ܲ��ܵ�������Ϊ���ֻ�����------//
                    PushSemstack(tnfstack, innertable);
                    strcpy(type, idstack.stack[idstack.top]);
                    PopTokenstack(&idstack);    // ����������
                    typos = GetType(typtbl, type);    // �������������ͱ��е�λ��
                    if(typos == -1)
                    {
                        printf("Unfefined type %s !\n", idstack.stack[idstack.top]);
                        exit(0);
                    }
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack->tblhead[tnfstack->top])->trihead, idtable);
                    strcpy(idtable[position].kind, "ret");    // ����
                    strcpy(idtable[position].type, type);    // ����
                    idtable[position].offset = tnfstack->offset[tnfstack->top];    // ���·��ű��� ���ŵ�ƫ��
                    tnfstack->offset[tnfstack->top] += typtbl[typos].width;    // ����ջ��ƫ��
                }
                else
                {
                    InitTblH(tnfstack->tblhead[tnfstack->top], innertable, idstack.stack[idstack.top]);    // ----- �ܲ��ܵ�������Ϊ���ֻ�����------//
                    PushSemstack(tnfstack, innertable);
                }
                PopTokenstack(&idstack);
                break;
            case -41:
                /*�﷨����*/
                printf("<function_block> => <block> semi;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 26, gototable[(*sstack).stack[(*sstack).top]][26]);    // 26:�չ�Լ�����ķ��ս������
                break;
            case -42:
                printf("<statement_part> => <compound_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 27, gototable[(*sstack).stack[(*sstack).top]][27]);    // 27:�չ�Լ�����ķ��ս������
                break;
            case -43:
                printf("<compound_statement> => begin <statement_sequence> end;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 28, gototable[(*sstack).stack[(*sstack).top]][28]);    // 28:�չ�Լ�����ķ��ս������
                break;
            case -44:
                printf("<statement_sequence> => <statement> semi;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 29, gototable[(*sstack).stack[(*sstack).top]][29]);    // 29:�չ�Լ�����ķ��ս������
                break;
            case -45:
                /*�﷨����*/
                printf("<procedure_function_declaration> => <procedure_declaration>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 17, gototable[(*sstack).stack[(*sstack).top]][17]);    // 17:�չ�Լ�����ķ��ս������
                /*�������*/
                GenCode('\0', "ret", '\0', '\0');
                break;
            case -46:
                /*�﷨����*/
                printf("<procedure_declaration> => <procedure_heading> semi <inner_idtable> <procedure_block>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                /*�������*/
                inhead = tnfstack->tblhead[tnfstack->top];
                inhead->offset = tnfstack->offset[tnfstack->top];
                tnfstack->top --;
                outhead = tnfstack->tblhead[tnfstack->top];
                outhead->inner[outhead->nextin] = inhead;
                idtable[porfh].extend->dlen[0] = outhead->nextin;    // ���ű��¼�б�Ǹú������ű��ͷ�������ĸ� �ڲ�ָ��洢
                outhead->nextin ++;
                if(outhead->nextin >= outhead->insize)    // ��̬����
                {
                    outhead->insize *= 2;
                    TableHead **newinner = (TableHead **)malloc(sizeof(TableHead *) * outhead->insize);
                    for(i = 0; i < outhead->nextin; i ++)    // ����
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
                PushParstack(cstack, sstack, 30, gototable[(*sstack).stack[(*sstack).top]][30]);    // 30:�չ�Լ�����ķ��ս������
                break;
            case -47:
                /*�﷨����*/
                printf("<procedure_block> => <block> semi;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 31, gototable[(*sstack).stack[(*sstack).top]][31]);    // 31:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -48:
                /*�﷨����*/
                printf("<procedure_heading> => procedure <procedure_identifier>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 32, gototable[(*sstack).stack[(*sstack).top]][32]);    // 32:�չ�Լ�����ķ��ս������
                /*�������*/
                defproc = 0;
                break;
            case -49:
                /*�﷨����*/
                printf("<procedure_heading> => procedure <procedure_identifier> ( <formal_parameter_list> );\n");    // ��Ӧ����ʽ
                for(i = 5; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 32, gototable[(*sstack).stack[(*sstack).top]][32]);    // 32:�չ�Լ�����ķ��ս������
                /*�������*/
                defproc = 0;
                break;
            case -50:
                /*�﷨����*/
                printf("<statement_sequence> => <statement_sequence> <bool_mark_M> <statement> semi;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 29, gototable[(*sstack).stack[(*sstack).top]][29]);    // 29:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = nextlist.stack[nextlist.top];    // ȡ����
                nextlist.top --;
//                printf("%d\n", nextlist.top);
                tmplist2 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist2, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&nextlist, tmplist1);    // ��ѹ��ȥ
                break;
            case -51:
                printf("<statement> => <simple_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 33, gototable[(*sstack).stack[(*sstack).top]][33]);    // 33:�չ�Լ�����ķ��ս������
                break;
            case -52:
                /*�﷨����*/
                printf("<simple_statement> => <assignment_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 34, gototable[(*sstack).stack[(*sstack).top]][34]);    // 34:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist4[0] = -1;
                PushListstack(&nextlist, tmplist4);    // ѹ�����
//                nextlist.top ++;    // �൱��ѹ�����
                break;
            case -53:
                 /*�﷨����*/
                printf("<assignment_statement> => <variable_access> := <expression>\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 35, gototable[(*sstack).stack[(*sstack).top]][35]);    // 35:�չ�Լ�����ķ��ս������
                /*�������*/
                tmpvarone = tmstack.stack[tmstack.top];    // �Ҳ�
                PopTmpstack();
                tmpvartwo = tmstack.stack[tmstack.top];    // ���
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                if(tmpvartwo.index == -1)    // δʹ����ʱ����
                {
                    strcpy(result, tmpvartwo.addr);
                }
                else    // ʹ������ʱ����
                {
                    strcpy(result, "t");
                    itoa(tmpvartwo.index, tmpic, 10);
                    strcat(result, tmpic);
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, ":=", opr1, '\0');
                break;
            case -54:
                printf("<variable_access> => <variable_identifier>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:�չ�Լ�����ķ��ս������
                break;
            case -55:
                /*�﷨����*/
                printf("<variable_identifier> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 37, gototable[(*sstack).stack[(*sstack).top]][37]);    // 37:�չ�Լ�����ķ��ս������
                /*�������*/
                if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // δ�������
                {
                    strcpy(errmssg, "Unfefined variable : ");
                    strcat(errmssg, idstack.stack[idstack.top]);
                    Error(fe, row, errmssg);
                }
                PushTmpstack(-1, idstack.stack[idstack.top]);    // ��������ʱ������ֱ��ѹ����ʱ����ջ
                PopTokenstack(&idstack);
                break;
            case -56:
                printf("<expression> => <simple_expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 38, gototable[(*sstack).stack[(*sstack).top]][38]);    // 38:�չ�Լ�����ķ��ս������
                break;
            case -57:
                printf("<simple_expression> => <term>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:�չ�Լ�����ķ��ս������
                break;
            case -58:
                 /*�﷨����*/
                printf("<simple_expression> => <sign> <term>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:�չ�Լ�����ķ��ս������
                /*�������*/
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
                        temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                    }
                    PopTmpstack();
                    GenCode(result, ":=", opr1, '\0');
                    PushTmpstack(tmpi, '\0');
                }
                PopTokenstack(&oprstack);
                break;
            case -59:
                printf("<simple_expression> => <term> <simple_expression_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:�չ�Լ�����ķ��ս������
                break;
            case -60:
                /*�﷨����*/
                printf("<simple_expression> => <sign> <term> <simple_expression_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 39, gototable[(*sstack).stack[(*sstack).top]][39]);    // 39:�չ�Լ�����ķ��ս������
                /*�������*/
                if(!strcmp(oprstack.stack[oprstack.top], "-"))
                {
                    tmpvarone = tmstack.stack[tmstack.top];
                    tmpi = NewTemp();
                    strcpy(result, "t");
                    itoa(tmpi, tmpic, 10);
                    strcat(result, tmpic);
                    strcpy(opr1, "minus");
                    // һ��ʹ������ʱ����
                    strcat(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                    PopTmpstack();
                    GenCode(result, ":=", opr1, '\0');
                    PushTmpstack(tmpi, '\0');
                }
                PopTokenstack(&oprstack);
                break;
            case -61:
                printf("<term> => <factor>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 40, gototable[(*sstack).stack[(*sstack).top]][40]);    // 40:�չ�Լ�����ķ��ս������
                break;
            case -62:
                printf("<term> => <factor> <term_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 40, gototable[(*sstack).stack[(*sstack).top]][40]);    // 40:�չ�Լ�����ķ��ս������
                break;
            case -63:
                /*�﷨����*/
                printf("<factor> => <unsigned_constant>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:�չ�Լ�����ķ��ս������
                /*�������*/
                PushTmpstack(-1, constack.stack[constack.top]);    // ѹ����ʱ����ջ������ʵ������Ҫ������ʱ����������±�Ϊ-1
                PopTokenstack(&constack);
                break;
            case -64:
                printf("<unsigned_constant>=> INT;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:�չ�Լ�����ķ��ս������
                break;
            case -65:
                printf("<unsigned_constant>=> REAL;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:�չ�Լ�����ķ��ս������
                break;
            case -66:
                printf("<unsigned_constant>=> STRING;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:�չ�Լ�����ķ��ս������
                break;
            case -67:
                printf("<unsigned_constant>=> nil;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 42, gototable[(*sstack).stack[(*sstack).top]][42]);    // 42:�չ�Լ�����ķ��ս������
                break;
            case -68:
                /*�﷨����*/
                printf("<term_sequence> => <multiplying_operator> <factor>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 43, gototable[(*sstack).stack[(*sstack).top]][43]);    // 43:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "*") && strcmp(tmpopr, "/") && strcmp(tmpopr, "mod") && strcmp(tmpopr, "div"))
                {    // ����*��/��div��mod�����ȼ�˳���嵼��
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // �ٰ�˳��ѹ��
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -69:
                /*�﷨����*/
                printf("<term_sequence> => <term_sequence> <multiplying_operator> <factor>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 43, gototable[(*sstack).stack[(*sstack).top]][43]);    // 43:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "*") && strcmp(tmpopr, "/") && strcmp(tmpopr, "mod") && strcmp(tmpopr, "div"))
                {    // ����*��/��div��mod�����ȼ�˳���嵼��
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // �ٰ�˳��ѹ��
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -70:
                printf("<multiplying_operator> => *;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:�չ�Լ�����ķ��ս������
                break;
            case -71:
                printf("<multiplying_operator> => /;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:�չ�Լ�����ķ��ս������
                break;
            case -72:
                printf("<multiplying_operator> => div;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:�չ�Լ�����ķ��ս������
                break;
            case -73:
                printf("<multiplying_operator> => mod;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 44, gototable[(*sstack).stack[(*sstack).top]][44]);    // 44:�չ�Լ�����ķ��ս������
                break;
            case -74:
                printf("<sign> => +;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 45, gototable[(*sstack).stack[(*sstack).top]][45]);    // 45:�չ�Լ�����ķ��ս������
                break;
            case -75:
                printf("<sign> => -;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 45, gototable[(*sstack).stack[(*sstack).top]][45]);    // 45:�չ�Լ�����ķ��ս������
                break;
            case -76:
                 /*�﷨����*/
                printf("<simple_expression_sequence> => <adding_operator> <term>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 46, gototable[(*sstack).stack[(*sstack).top]][46]);    // 46:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "+") && strcmp(tmpopr, "-"))    // ����+ Ҳ����-�����ȼ�������˳��һ�µ���
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // �ٰ�˳��ѹ��
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -77:
                /*�﷨����*/
                printf("<simple_expression_sequence> => <simple_expression_sequence> <adding_operator> <term>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 46, gototable[(*sstack).stack[(*sstack).top]][46]);    // 46:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(strcmp(tmpopr, "+") && strcmp(tmpopr, "-"))    // ����+ Ҳ����-�����ȼ�������˳��һ�µ���
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcpy(oprnt, tmpopr);
                while(tmpoprstack.top >= 0)    // �ٰ�˳��ѹ��
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, oprnt, opr1, opr2);
                PushTmpstack(tmpi, '\0');
                break;
            case -78:
                printf("<adding_operator> => +;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 47, gototable[(*sstack).stack[(*sstack).top]][47]);    // 47:�չ�Լ�����ķ��ս������
                break;
            case -79:
                printf("<adding_operator> => -;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 47, gototable[(*sstack).stack[(*sstack).top]][47]);    // 47:�չ�Լ�����ķ��ս������
                break;
            case -80:
                printf("<type_denoter> => <new_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 14, gototable[(*sstack).stack[(*sstack).top]][14]);    // 14:�չ�Լ�����ķ��ս������
                break;
            case -81:
                printf("<new_type> => <new_structured_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 48, gototable[(*sstack).stack[(*sstack).top]][48]);    // 48:�չ�Լ�����ķ��ս������
                break;
            case -82:
                printf("<new_structured_type> => <unpacked_structured_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 49, gototable[(*sstack).stack[(*sstack).top]][49]);    // 49:�չ�Լ�����ķ��ս������
                break;
            case -83:
                printf("<unpacked_structured_type> => <array_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 50, gototable[(*sstack).stack[(*sstack).top]][50]);    // 50:�չ�Լ�����ķ��ս������
                break;
            case -84:
                printf("<array_type> => array [ <index_type> <index_type_sequence> ] of <component_type>;\n");    // ��Ӧ����ʽ
                for(i = 7; i > 0; i --)    // 7:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 51, gototable[(*sstack).stack[(*sstack).top]][51]);    // 51:�չ�Լ�����ķ��ս������
                break;
            case -85:
                printf("<index_type_sequence> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 52, gototable[(*sstack).stack[(*sstack).top]][52]);    // 52:�չ�Լ�����ķ��ս������
                break;
            case -86:
                printf("<index_type_sequence> => <index_type_sequence> , <index_type>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 52, gototable[(*sstack).stack[(*sstack).top]][52]);    // 52:�չ�Լ�����ķ��ս������
                break;
            case -87:
                printf("<index_type> => <ordinal_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 53, gototable[(*sstack).stack[(*sstack).top]][53]);    // 53:�չ�Լ�����ķ��ս������
                break;
            case -88:
                printf("<ordinal_type> => <new_ordinal_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 54, gototable[(*sstack).stack[(*sstack).top]][54]);    // 54:�չ�Լ�����ķ��ս������
                break;
            case -89:
                printf("<new_ordinal_type> => <subrange_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 55, gototable[(*sstack).stack[(*sstack).top]][55]);    // 55:�չ�Լ�����ķ��ս������
                break;
            case -90:
                /*�﷨����*/
                printf("<subrange_type> => <constant> .. <constant>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 56, gototable[(*sstack).stack[(*sstack).top]][56]);    // 56:�չ�Լ�����ķ��ս������
                /*�������*/
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
                printf("<component_type> => <type_denoter>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 57, gototable[(*sstack).stack[(*sstack).top]][57]);    // 57:�չ�Լ�����ķ��ս������
                break;
            case -92:
                printf("<variable_access> => <component_variable>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:�չ�Լ�����ķ��ս������
                break;
            case -93:
                /*�﷨����*/
                printf("<component_variable> => <indexed_variable>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 58, gototable[(*sstack).stack[(*sstack).top]][58]);    // 58:�չ�Լ�����ķ��ս������
                /*�������*/
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
                temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                strcat(opr1, "[");
                strcat(opr1, "t");
                itoa(tmpvartwo.index, tmpic, 10);
                strcat(opr1, tmpic);
                temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                strcat(opr1, "]");
                GenCode(result, ":=", opr1, '\0');
                PushTmpstack(tmpi, '\0');
                arryflag = 0;
                break;
            case -94:
                /*�﷨����*/
                printf("<indexed_variable> => <index_expression_sequence> ];\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 59, gototable[(*sstack).stack[(*sstack).top]][59]);    // 59:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(oprnt, "*");
                itoa(setypevar.width, tmpic, 10);
                strcpy(opr2, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, oprnt, opr1, opr2);    // �����ͱ���λ��

                tmpi = NewTemp();
                strcpy(result, "t");
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                itoa(setypevar.fixepart, tmpic, 10);
                strcpy(opr1, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, ":=", opr1, '\0');    // �����ͱ�����ַ
                break;
            case -95:
                /*�﷨����*/
                printf("<index_expression_sequence> => <array_variable> [ <index_expression>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 60, gototable[(*sstack).stack[(*sstack).top]][60]);    // 60:�չ�Լ�����ķ��ս������
                /*�������*/
                setypevar.ndim = 1;
                break;
            case -96:
                /*�﷨����*/
                printf("<index_expression_sequence> => <index_expression_sequence> , <index_expression>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 60, gototable[(*sstack).stack[(*sstack).top]][60]);    // 60:�չ�Լ�����ķ��ս������
                /*�������*/
                // ���������ͱ�����ƫ��
                tmpvarone = tmstack.stack[tmstack.top];    // ��ʱ����ջջ��
                tmpvartwo = tmstack.stack[tmstack.top - 1];    // ��ʱ����ջ�ζ�

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
                    temps[tmpvartwo.index] = 0;        // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(oprnt, "*");
                itoa(setypevar.dlen[setypevar.ndim - 1], tmpic, 10);
                strcpy(opr2, tmpic);
                PushTmpstack(tmpi, '\0');
                GenCode(result, oprnt, opr1, opr2);

                tmpvartwo = tmpvarone;    // ��ʱ����ջ�ζ�(�ոս�����ѹջ)
                tmpvarone = tmstack.stack[tmstack.top];    // ��ʱ����ջջ��

                strcpy(result, "t");
                tmpi = NewTemp();
                itoa(tmpi, tmpic, 10);
                strcat(result, tmpic);
                strcpy(opr1, "t");
                itoa(tmpvarone.index, tmpic, 10);
                strcat(opr1, tmpic);
                temps[tmpvarone.index] = 0;        // ʹ������ʱ������ �黹��ʱ������
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
                    temps[tmpvartwo.index] = 0;        // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, oprnt, opr1, opr2);
                PopTmpstack();
                PopTmpstack();
                PopTmpstack();
                PushTmpstack(tmpi, '\0');
                break;
            case -97:
                /*�﷨����*/
                printf("<array_variable> => <variable_access>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 61, gototable[(*sstack).stack[(*sstack).top]][61]);    // 61:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(setypevar.base, tmstack.stack[tmstack.top].addr);    // �����ͱ���������
                j = SearchTable(setypevar.base, tnfstack->tblhead[tnfstack->top]);    // �������ֲ�idtable
                PopTmpstack();    // ��ջ
                if(j == -1)    // δ�������
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
                setypevar.width = typtbl[tmpi].width;    // ��ȡ���
                //    ����̶�ƫ����
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
                setypevar.dlen = idtable[j].extend->dlen;    // ��άά��
                // ������������������ֽ���������ʱ���������Ա����� setypevar �У�ʡȥ�Ժ�һ�δβ����setypevar��ȡ����
                break;
            case -98:
                printf("<index_expression> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 62, gototable[(*sstack).stack[(*sstack).top]][62]);    // 62:�չ�Լ�����ķ��ս������
                break;
            case -99:
                /*�﷨����*/
                printf("<factor> => <variable_access>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:�չ�Լ�����ķ��ս������
                break;
            case -100:
                /*�﷨����*/
                printf("<factor> => ( <expression> );\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 41, gototable[(*sstack).stack[(*sstack).top]][41]);    // 41:�չ�Լ�����ķ��ս������
                break;
            case -101:
                /*�﷨����*/
                printf("<statement> => <structured_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 33, gototable[(*sstack).stack[(*sstack).top]][33]);    // 33:�չ�Լ�����ķ��ս������
                break;
            case -102:
                /*�﷨����*/
                printf("<structured_statement> => <compound_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:�չ�Լ�����ķ��ս������
                break;
            case -103:
                /*�﷨����*/
                printf("<structured_statement> => <conditional_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:�չ�Լ�����ķ��ս������
                break;
            case -104:
                /*�﷨����*/
                printf("<conditional_statement> => <if_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 64, gototable[(*sstack).stack[(*sstack).top]][64]);    // 64:�չ�Լ�����ķ��ս������
                break;
            case -105:
                /*�﷨����*/
                printf("<if_statement> => if <boolean_expression> then <bool_mark_M> <statement>;\n");    // ��Ӧ����ʽ
                for(i = 5; i > 0; i --)    // 5:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 65, gototable[(*sstack).stack[(*sstack).top]][65]);    // 65:�չ�Լ�����ķ��ս������
                /*�������*/
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
                /*�﷨����*/
                printf("<if_statement> => if <boolean_expression> then <bool_mark_M> <statement> <bool_mark_N> <else_part>;\n");    // ��Ӧ����ʽ
                for(i = 7; i > 0; i --)    // 7:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 65, gototable[(*sstack).stack[(*sstack).top]][65]);    // 65:�չ�Լ�����ķ��ս������
                /*�������*/
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
                /*�﷨����*/
                printf("<boolean_expression> => <boolean_term>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:�չ�Լ�����ķ��ս������
                break;
            case -108:
                /*�﷨����*/
                printf("<boolean_expression> => not <boolean_term>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:�չ�Լ�����ķ��ս������
                /*�������*/
                i = 0;
                while(truelist.stack[truelist.top][i] != -1)    // ����
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
                /*�﷨����*/
                printf("<boolean_expression> => <boolean_term> <boolean_expression_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:�չ�Լ�����ķ��ս������
                break;
            case -110:
                /*�﷨����*/
                printf("<boolean_expression> => not <boolean_term> <boolean_expression_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 66, gototable[(*sstack).stack[(*sstack).top]][66]);    // 66:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                PushListstack(&truelist, tmplist2);
                PushListstack(&falselist, tmplist1);
                break;
            case -111:
                /*�﷨����*/
                printf("<boolean_factor> => ( <boolean_expression> );\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 68, gototable[(*sstack).stack[(*sstack).top]][68]);    // 68:�չ�Լ�����ķ��ս������
                break;
            case -112:
                /*�﷨����*/
                printf("<boolean_term> => <boolean_factor>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 67, gototable[(*sstack).stack[(*sstack).top]][67]);    // 67:�չ�Լ�����ķ��ս������
                break;
            case -113:
                /*�﷨����*/
                printf("<boolean_term> => <boolean_factor> <boolean_term_sequence>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 67, gototable[(*sstack).stack[(*sstack).top]][67]);    // 67:�չ�Լ�����ķ��ս������
                break;
            case -114:
                /*�﷨����*/
                printf("<boolean_factor> => <expression> <relational_operator> <expression>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 68, gototable[(*sstack).stack[(*sstack).top]][68]);    // 68:�չ�Լ�����ķ��ս������
                /*�������*/
                tmpvartwo = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmpvarone = tmstack.stack[tmstack.top];
                PopTmpstack();
                tmplist4[0] = nextcode;
                PushListstack(&truelist, tmplist4);
                tmplist4[0] = nextcode + 1;
                PushListstack(&falselist, tmplist4);
//                printf("CHECK >>>> ------ >>>> --------- %d: %d\n", falselist.top, falselist.stack[falselist.top][0]);
                tmplist4[0] = -1;    // ��ԭ����ȻӰ���Ժ��
                if(tmpvarone.index == -1)
                {
                    strcpy(opr1, tmpvarone.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarone.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcpy(tmpopr, oprstack.stack[oprstack.top]);
                PopTokenstack(&oprstack);
                while(!strcmp(tmpopr, "and") || !strcmp(tmpopr, "or"))    // ��+ ������-�����ȼ�������˳��һ�µ���
                {
                    tmpoprstack.top ++;
                    strcpy(tmpoprstack.stack[tmpoprstack.top], tmpopr);
                    strcpy(tmpopr, oprstack.stack[oprstack.top]);
                    PopTokenstack(&oprstack);
                }
                strcat(opr1, tmpopr);
                while(tmpoprstack.top >= 0)    // �ٰ�˳��ѹ��
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode('\0', "if", opr1, "goto");
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -115:
                /*�﷨����*/
                printf("<relational_operator> => =;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -116:
                /*�﷨����*/
                printf("<relational_operator> => <>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -117:
                /*�﷨����*/
                printf("<relational_operator> => <;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -118:
                /*�﷨����*/
                printf("<relational_operator> => >;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -119:
                /*�﷨����*/
                printf("<relational_operator> => <=;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -120:
                /*�﷨����*/
                printf("<relational_operator> => >=;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -121:
                /*�﷨����*/
                printf("<relational_operator> => in;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 69, gototable[(*sstack).stack[(*sstack).top]][69]);    // 69:�չ�Լ�����ķ��ս������
                break;
            case -122:
                /*�﷨����*/
                printf("<boolean_term_sequence> => and <bool_mark_M> <boolean_factor>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 70, gototable[(*sstack).stack[(*sstack).top]][70]);    // 70:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(truelist.stack[truelist.top],Mquad.stack[Mquad.top]);
                truelist.top --;
                Mquad.top --;
                PushListstack(&truelist, tmplist1);    // ��ѹ��
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&falselist, tmplist4);
                break;
            case -123:
                /*�﷨����*/
                printf("<boolean_term_sequence> => <boolean_term_sequence> and <bool_mark_M> <boolean_factor>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 70, gototable[(*sstack).stack[(*sstack).top]][70]);    // 70:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Backpach(truelist.stack[truelist.top],Mquad.stack[Mquad.top]);
                truelist.top --;
                Mquad.top --;
                PushListstack(&truelist, tmplist1);    // ��ѹ��
                tmplist2 = falselist.stack[falselist.top];
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&falselist, tmplist4);
                break;
            case -124:
                /*�﷨����*/
                printf("<boolean_expression_sequence> => or <bool_mark_M> <boolean_term>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 71, gototable[(*sstack).stack[(*sstack).top]][71]);    // 71:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist2 = falselist.stack[falselist.top];    // ȡ��
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&falselist, tmplist2);    // ��ѹ��
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&truelist,tmplist4);
                break;
            case -125:
                /*�﷨����*/
                printf("<boolean_expression_sequence> => <boolean_expression_sequence> or <bool_mark_M> <boolean_term>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 71, gototable[(*sstack).stack[(*sstack).top]][71]);    // 71:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist2 = falselist.stack[falselist.top];    // ȡ��
                falselist.top --;
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                PushListstack(&falselist, tmplist2);    // ��ѹ��
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                tmplist1 = truelist.stack[truelist.top];
                truelist.top --;
                Merge(tmplist1, tmplist2, tmplist4);
                PushListstack(&truelist,tmplist4);
                break;
            case -126:
                /*�﷨����*/
                printf("<bool_mark_M> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 72, gototable[(*sstack).stack[(*sstack).top]][72]);    // 72:�չ�Լ�����ķ��ս������
                /*�������*/
                Mquad.top ++;
                Mquad.stack[Mquad.top] = nextcode;
                break;
            case -127:
                /*�﷨����*/
                printf("<bool_mark_N> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 73, gototable[(*sstack).stack[(*sstack).top]][73]);    // 73:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist4[0] = nextcode;
                PushListstack(&nextlist, tmplist4);
                tmplist4[0] = -1;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -128:
                /*�﷨����*/
                printf("<else_part> => else <bool_mark_M> <statement>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 74, gototable[(*sstack).stack[(*sstack).top]][74]);    // 74:�չ�Լ�����ķ��ս������
                break;
            case -129:
                printf("<structured_statement> => <repetitive_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 63, gototable[(*sstack).stack[(*sstack).top]][63]);    // 63:�չ�Լ�����ķ��ս������
                break;
            case -130:
                printf("<repetitive_statement> => <while_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:�չ�Լ�����ķ��ս������
                break;
            case -131:
                /*�﷨����*/
                printf("<while_statement> => while <bool_mark_M> <boolean_expression> do <bool_mark_M> <statement>;\n");    // ��Ӧ����ʽ
                for(i = 6; i > 0; i --)    // 6:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 76, gototable[(*sstack).stack[(*sstack).top]][76]);    // 76:�չ�Լ�����ķ��ս������
                /*�������*/
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
                printf("<repetitive_statement> => <for_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:�չ�Լ�����ķ��ս������
                break;
            case -133:
                /*�﷨����*/
                printf("<for_statement> => for <control_variable> := <initial_value> to <final_value> do <for_mark_M> <statement>;\n");    // ��Ӧ����ʽ
                for(i = 9; i > 0; i --)    // 9:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 77, gototable[(*sstack).stack[(*sstack).top]][77]);    // 77:�չ�Լ�����ķ��ս������
                /*�������*/
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
                /*�﷨����*/
                printf("<for_statement> => for <control_variable> := <initial_value> downto <final_value> do <for_mark_M> <statement>;\n");    // ��Ӧ����ʽ
                for(i = 9; i > 0; i --)    // 9:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 77, gototable[(*sstack).stack[(*sstack).top]][77]);    // 77:�չ�Լ�����ķ��ս������
                /*�������*/
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
                printf("<control_variable> => <entire_variable>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 78, gototable[(*sstack).stack[(*sstack).top]][78]);    // 78:�չ�Լ�����ķ��ս������
                break;
            case -136:
                printf("<entire_variable> => <variable_identifier>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 79, gototable[(*sstack).stack[(*sstack).top]][79]);    // 79:�չ�Լ�����ķ��ս������
                break;
            case -137:
                printf("<initial_value> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 80, gototable[(*sstack).stack[(*sstack).top]][80]);    // 80:�չ�Լ�����ķ��ս������
                break;
            case -138:
                printf("<final_value> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 81, gototable[(*sstack).stack[(*sstack).top]][81]);    // 81:�չ�Լ�����ķ��ս������
                break;
            case -139:
                /*�﷨����*/
                printf("<for_mark_M> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 82, gototable[(*sstack).stack[(*sstack).top]][82]);    // 82:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[tmpvartwo.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                GenCode(result, ":=", opr1, '\0');
                tmpi = NewTemp();
                strcpy(result, "t");
                itoa(tmpi, tmpich, 10);
                strcat(result, tmpich);    // ʹ�� tmpich�� ��Ϊ������õ�
                if(tmpvarthree.index == -1)
                {
                    strcpy(opr1, tmpvarthree.addr);
                }
                else
                {
                    strcpy(opr1, "t");
                    itoa(tmpvarthree.index, tmpic, 10);
                    strcat(opr1, tmpic);
                    temps[tmpvarthree.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
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
                printf("<repetitive_statement> => <repeat_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 75, gototable[(*sstack).stack[(*sstack).top]][75]);    // 75:�չ�Լ�����ķ��ս������
                break;
            case -141:
                /*�﷨����*/
                printf("<repeat_statement> => repeat <bool_mark_M> <statement_sequence> until <repeat_mark_N> <boolean_expression>;\n");    // ��Ӧ����ʽ
                for(i = 6; i > 0; i --)    // 6:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 83, gototable[(*sstack).stack[(*sstack).top]][83]);    // 83:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = falselist.stack[falselist.top];
                falselist.top --;
                Backpach(tmplist1, Mquad.stack[Mquad.top]);
                Mquad.top --;
                tmplist2 = truelist.stack[truelist.top];
                truelist.top --;
                PushListstack(&nextlist, tmplist2);
                break;
            case -142:
                /*�﷨����*/
                printf("<repeat_mark_N> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 84, gototable[(*sstack).stack[(*sstack).top]][84]);    // 84:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist1 = nextlist.stack[nextlist.top];
                nextlist.top --;
                Backpach(tmplist1, nextcode);
                break;
            case -143:
                /*�﷨����*/
                printf("<new_type> => <new_pointer_type>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 48, gototable[(*sstack).stack[(*sstack).top]][48]);    // 48:�չ�Լ�����ķ��ս������
                break;
            case -144:
                /*�﷨����*/
                printf("<new_pointer_type> => ^ <domain_type>;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 85, gototable[(*sstack).stack[(*sstack).top]][85]);    // 85:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(type, "^");
                strcat(type, idstack.stack[idstack.top]);
                PopTokenstack(&idstack);
                idstack.top ++;
                strcpy(idstack.stack[idstack.top], type);
                break;
            case -145:
                /*�﷨����*/
                printf("<domain_type> => <type_identifier>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 86, gototable[(*sstack).stack[(*sstack).top]][86]);    // 86:�չ�Լ�����ķ��ս������
                break;
            case -146:
                /*�﷨����*/
                printf("<procedure_identifier> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 87, gototable[(*sstack).stack[(*sstack).top]][87]);    // 87:�չ�Լ�����ķ��ս������
                /*�������*/
                // �����������ڲ���ű�ʱ����Ҫ
                if(defproc)    // ���̶���ʱ
                {
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                    porfh = position;
                    strcpy(idtable[position].id, idstack.stack[idstack.top]);
                    strcpy(idtable[position].kind, "proc");
                    idtable[position].offset = -1;    // ����ռ�ݿռ䣬������¼�Ա��ڷ���
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 0;
                    idtable[position].extend->dlen = (int *)malloc(sizeof(int));
                    idtable[position].extend->dlen[0] = 0;
                }
                else    // ���̵���
                {
//                    printf("%d\n", SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]));
                    if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // δ�������
                    {
                        strcpy(errmssg, "Unfefined procedure : ");
                        strcat(errmssg, idstack.stack[idstack.top]);
                        Error(fe, row, errmssg);
                    }
                }
                break;
            case -147:
                /*�﷨����*/
                printf("<function_identifier> => ID;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 88, gototable[(*sstack).stack[(*sstack).top]][88]);    // 88:�չ�Լ�����ķ��ս������
                /*�������*/
                // �����������ڲ���ű�ʱ����Ҫ
                if(deffunc)    // ��������ʱ
                {
                    fun_or_proc = 1;
                    position = UpdateTable(idstack.stack[idstack.top], (tnfstack -> tblhead[tnfstack->top])->trihead, idtable);
                    porfh = position;
                    strcpy(idtable[position].id, idstack.stack[idstack.top]);
                    strcpy(idtable[position].kind, "func");
                    idtable[position].offset = -1;    // ����ռ�ݿռ䣬������¼�Ա��ڷ���
                    idtable[position].extend = (ExtNode *)malloc(sizeof(ExtNode));
                    idtable[position].extend->dimen = 0;
                    idtable[position].extend->dlen = (int *)malloc(sizeof(int));
                    idtable[position].extend->dlen[0] = 0;
                }
                else    // ��������
                {
                    if(SearchTable(idstack.stack[idstack.top], tnfstack->tblhead[tnfstack->top]) == -1)    // δ���庯��
                    {
                        strcpy(errmssg, "Unfefined function : ");
                        strcat(errmssg, idstack.stack[idstack.top]);
                        Error(fe, row, errmssg);
                    }
                }
                break;
            case -148:
                /*�﷨����*/
                printf("<variable_access> => <identified_variable>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 36, gototable[(*sstack).stack[(*sstack).top]][36]);    // 36:�չ�Լ�����ķ��ս������
                break;
            case -149:
                /*�﷨����*/
                printf("<identified_variable> => <pointer_variable> ^;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 89, gototable[(*sstack).stack[(*sstack).top]][89]);    // 89:�չ�Լ�����ķ��ս������
                /*�������*/
                strcat(tmstack.stack[tmstack.top].addr, "^");    // ���Ϊָ��
                break;
            case -150:
                /*�﷨����*/
                printf("<pointer_variable> => <variable_access>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 90, gototable[(*sstack).stack[(*sstack).top]][90]);    // 90:�չ�Լ�����ķ��ս������
                break;
            case -151:
                /*�﷨����*/
                printf("<simple_statement> => <procedure_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 34, gototable[(*sstack).stack[(*sstack).top]][34]);    // 34:�չ�Լ�����ķ��ս������
                /*�������*/
                tmplist4[0] = -1;
                PushListstack(&nextlist, tmplist4);    // ѹ�����
//                nextlist.top ++;
                break;
            case -152:
                /*�﷨����*/
                printf("<procedure_statement> => <procedure_identifier> ( <actual_parameter_list> );\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:�չ�Լ�����ķ��ս������
                /*�������*/
                while(countidlis > 0)
                {
                    tmpvarone = tmstack.stack[tmstack.top];
                    PopTmpstack();
                    if(tmpvarone.index == -1)    // δʹ����ʱ����
                    {
                        strcpy(result,tmpvarone.addr);
                    }
                    else    // ʹ����ʱ����
                    {
                        strcpy(result,"t");
                        itoa(tmpvarone.index, tmpic, 10);
                        temps[tmpvarone.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
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
                /*�﷨����*/
                printf("<procedure_statement> => <procedure_identifier> ( );\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:�չ�Լ�����ķ��ս������
                /*�������*/
                strcpy(result,idstack.stack[idstack.top]);
                PopTokenstack(&idstack);
                GenCode(result,"call",'\0', '\0');
                break;
            case -154:
                /*�﷨����*/
                printf("<actual_parameter_list> => <actual_parameter>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 92, gototable[(*sstack).stack[(*sstack).top]][92]);    // 92:�չ�Լ�����ķ��ս������
                /*�������*/
                countidlis ++;
                break;
            case -155:
                /*�﷨����*/
                printf("<actual_parameter_list> => <actual_parameter_list> , <actual_parameter>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 92, gototable[(*sstack).stack[(*sstack).top]][92]);    // 92:�չ�Լ�����ķ��ս������
                /*�������*/
                countidlis ++;
                break;
            case -156:
                /*�﷨����*/
                printf("<actual_parameter> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 93, gototable[(*sstack).stack[(*sstack).top]][93]);    // 93:�չ�Լ�����ķ��ս������
                break;
            case -157:
                /*�﷨����*/
                printf("<conditional_statement> => <case_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 64, gototable[(*sstack).stack[(*sstack).top]][64]);    // 64:�չ�Լ�����ķ��ս������
                break;
            case -158:
                /*�﷨����*/
                printf("<case_statement> => case <case_index> of <case_list_element_sequence> end;\n");    // ��Ӧ����ʽ
                for(i = 5; i > 0; i --)    // 5:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 94, gototable[(*sstack).stack[(*sstack).top]][94]);    // 94:�չ�Լ�����ķ��ս������
                /*�������*/
                if(casenext != -1)
                {
                    tmplist4[0] = casenext;
                    Merge(caselist.stack, tmplist4, tmplist5);
                    tmplist4[0] = -1;
                }
                Backpach(tmplist5, nextcode);
                break;
            case -159:
                /*�﷨����*/
                printf("<case_list_element_sequence> => <case_list_element> semi;\n");    // ��Ӧ����ʽ
                for(i = 2; i > 0; i --)    // 2:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 95, gototable[(*sstack).stack[(*sstack).top]][95]);    // 95:�չ�Լ�����ķ��ս������
                /*�������*/
                caselist.top ++;
                caselist.stack[caselist.top] = nextcode;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -160:
                /*�﷨����*/
                printf("<case_list_element_sequence> => <case_list_element_sequence> <case_list_element> semi;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 95, gototable[(*sstack).stack[(*sstack).top]][95]);    // 95:�չ�Լ�����ķ��ս������
                /*�������*/
                caselist.top ++;
                caselist.stack[caselist.top] = nextcode;
                GenCode('\0', "goto", '\0', '\0');
                break;
            case -161:
                /*�﷨����*/
                printf("<case_index> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 96, gototable[(*sstack).stack[(*sstack).top]][96]);    // 96:�չ�Լ�����ķ��ս������
                /*�������*/
                casecore = tmstack.stack[tmstack.top];    // �洢
                PopTmpstack();
                break;
            case -162:
                /*�﷨����*/
                printf("<case_list_element> => <case_constant> <case_mark_N> : <statement>;\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 97, gototable[(*sstack).stack[(*sstack).top]][97]);    // 97:�չ�Լ�����ķ��ս������
                break;
            case -163:
                /*�﷨����*/
                printf("<case_constant> => <constant>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 98, gototable[(*sstack).stack[(*sstack).top]][98]);    // 98:�չ�Լ�����ķ��ս������
                break;
            case -164:
                /*�﷨����*/
                printf("<case_mark_N> =>;\n");    // ��Ӧ����ʽ
                PushParstack(cstack, sstack, 99, gototable[(*sstack).stack[(*sstack).top]][99]);    // 99:�չ�Լ�����ķ��ս������
                /*�������*/
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
                    temps[casecore.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                }
                strcat(opr1, "<>");
                strcat(opr1, constack.stack[constack.top]);
                PopTokenstack(&constack);
                GenCode('\0', "if", opr1, "goto");
                break;
            case -165:
                /*�﷨����*/
                printf("<procedure_statement> => <io_procedure_statement>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 91, gototable[(*sstack).stack[(*sstack).top]][91]);    // 91:�չ�Լ�����ķ��ս������
                break;
            case -166:
                /*�﷨����*/
                printf("<io_procedure_statement> => read ( <read_parameter_list> );\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 100, gototable[(*sstack).stack[(*sstack).top]][100]);    // 100:�չ�Լ�����ķ��ս������
                /*�������*/
                itoa(countpar, tmpic, 10);
                GenCode("SYSIN", "call", tmpic, '\0');
                countpar = 0;
                break;
            case -167:
                /*�﷨����*/
                printf("<io_procedure_statement> => write ( <write_parameter_list> );\n");    // ��Ӧ����ʽ
                for(i = 4; i > 0; i --)    // 4:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 100, gototable[(*sstack).stack[(*sstack).top]][100]);    // 100:�չ�Լ�����ķ��ս������
                /*�������*/
                itoa(countpar, tmpic, 10);
                GenCode("SYSOUT", "call", tmpic, '\0');
                countpar = 0;
                break;
            case -168:
                /*�﷨����*/
                printf("<read_parameter_list> => <variable_access_sequence>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 101, gototable[(*sstack).stack[(*sstack).top]][101]);    // 101:�չ�Լ�����ķ��ս������
                break;
            case -169:
                /*�﷨����*/
                printf("<variable_access_sequence> => <variable_access>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 102, gototable[(*sstack).stack[(*sstack).top]][102]);    // 102:�չ�Լ�����ķ��ս������
                /*�������*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // ʹ������ʱ����
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // δʹ����ʱ����
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -170:
                /*�﷨����*/
                printf("<variable_access_sequence> => <variable_access_sequence> , <variable_access>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 102, gototable[(*sstack).stack[(*sstack).top]][102]);    // 102:�չ�Լ�����ķ��ս������
                /*�������*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // δʹ����ʱ����
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // ʹ����ʱ����
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -171:
                /*�﷨����*/
                printf("<write_parameter_list> => <write_parameter_sequence>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 103, gototable[(*sstack).stack[(*sstack).top]][103]);    // 103:�չ�Լ�����ķ��ս������
                break;
            case -172:
                /*�﷨����*/
                printf("<write_parameter_sequence> => <expression>;\n");    // ��Ӧ����ʽ
                PopParstack(cstack, sstack);
                PushParstack(cstack, sstack, 104, gototable[(*sstack).stack[(*sstack).top]][104]);    // 104:�չ�Լ�����ķ��ս������
                /*�������*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // δʹ����ʱ����
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // ʹ����ʱ����
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
                    strcat(result, tmpic);
                }
                GenCode(result, "param", '\0', '\0');
                break;
            case -173:
                /*�﷨����*/
                printf("<write_parameter_sequence> => <write_parameter_sequence> , <expression>;\n");    // ��Ӧ����ʽ
                for(i = 3; i > 0; i --)    // 3:����ʽ�з��Ÿ���
                {
                    PopParstack(cstack, sstack);
                }
                PushParstack(cstack, sstack, 104, gototable[(*sstack).stack[(*sstack).top]][104]);    // 104:�չ�Լ�����ķ��ս������
                /*�������*/
                countpar ++;
                tmpvarone = tmstack.stack[tmstack.top];
                if(tmpvarone.index == -1)    // ʹ������ʱ����
                {
                    strcpy(result,tmpvarone.addr);
                }
                else    // δʹ����ʱ����
                {
                    strcpy(result,"t");
                    itoa(tmpvarone.index, tmpic, 10);
                    temps[tmpvarthree.index] = 0;    // ʹ������ʱ������ �黹��ʱ������
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

            /*չʾ���ű�*/
            tmp = entridtbl;
            printTables(tmp);
        }
    }while(!next);
    return;
}

void PrintToken(TrieNode *trihead, int index, char oneword[])    // ��ӡ���ʣ������ַ����ӡ
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

void PrintTable(TrieNode *trihead)    // ��ӡ�ַ���
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

void printTables(TableHead *tmp)    // ��ӡ����С���ű���Ҫ������ʾǶ�׹�ϵ
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

void Output(int code, char token[], int position, FILE *fd)    // ������ļ�����Ļ
{
    if (code == INT)    // ����
    {
        printf("(%d,%d)\n",code,atoi(token));
        fprintf(fd,"(%d,%d)\n",code,atoi(token));
    }
    else if(code == REAL)    // ʵ��
    {
        printf("(%d,%f)\n",code,atof(token));
        fprintf(fd,"(%d,%f)\n",code,atof(token));
    }
    else if(code == EREAL)    // ��ѧ������
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

void Error(FILE *fe, int row, char message[])    // ��ӡ������Ϣ
{
    fprintf(fe,"ERROR:  Line %d: %s\n",row,message);
//    printf("ERROR:  Line %d: %s\n",row,message);
    return;
}
