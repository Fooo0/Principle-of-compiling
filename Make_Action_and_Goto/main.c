#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PRODNUM 173    // ����ʽ����
#define PRODLEN 150    // ����ʽ���ȣ�ע����(�� LINESIZE �й���)
#define NONTERM 105    // ���ս������
#define TERM 67    // �ս������
#define STATE 833    // ״̬��
#define LINESIZE 150   // һ�����Ĵ�С��ע����(�� PRODLEN �й���)
#define MAXSTAP 4    // ���״̬(832)/������ʽ�����ռλ��(164) + 1 (����)

int main()
{
    char productions[PRODNUM][PRODLEN], production[PRODLEN], buffer[LINESIZE], shsta[MAXSTAP];
    FILE *fp = NULL, *fs = NULL, *fa = NULL, *fg = NULL;
    int i, j, k;
    if((fp = fopen("productions.txt","r")) == NULL)
    {
        printf("Fail to open productions.txt");
        exit(0);
    }
    if((fs = fopen("html.txt","r")) == NULL)
    {
        printf("Fail to open html.txt");
        exit(0);
    }
    if((fa = fopen("action.txt","w")) == NULL)
    {
        printf("Fail to open action.txt");
        exit(0);
    }
    if((fg = fopen("goto.txt","w")) == NULL)
    {
        printf("Fail to open goto.txt");
        exit(0);
    }

    for(i = 0; i < PRODNUM; i++)
    {
        fgets(production, LINESIZE, fp);
        production[strlen(production) - 2] = '\0';    // ��Ҫȥ��ĩβ�ķֺ�
        strcpy(productions[i], production);
    }

    while(!feof(fs))
    {
        fgets(buffer, LINESIZE, fs);
        buffer[strlen(buffer) - 1] = '\0';
        if(!strcmp(buffer, "<tr>"))
        {
            fgets(buffer, LINESIZE, fs);
            buffer[strlen(buffer) - 1] = '\0';
            i = atoi(buffer);
        }
        for(j = 1; j <= TERM; j ++)
        {
            fgets(buffer, LINESIZE, fs);
            buffer[strlen(buffer) - 1] = '\0';
            if(!strcmp(buffer, "e"))
            {
                continue;
            }
            else if(buffer[0] == 's')
            {
                memset(shsta, '\0', MAXSTAP);
                strncpy(shsta, buffer + 1, MAXSTAP);    // 1 ��s�ĳ���
                shsta[MAXSTAP] = '\0';
                fprintf(fa, "%d %d %s\n", i, j, shsta);
            }
            else if(buffer[0] == 'r')    // �ǵ��Ǹ���
            {
                memset(production, '\0', PRODLEN);
                strncpy(production, buffer + 1, PRODLEN);    // 1 ��s�ĳ���
                production[strlen(buffer) - 1] = '\0';
                for(k = 0; k < PRODNUM; k++)
                {
                    if(!strcmp(productions[k], production))
                    {
                        break;
                    }
                }
                fprintf(fa, "%d %d %d\n", i, j, -(k + 1));
            }
            else if(!strcmp(buffer, "a"))
            {
                fprintf(fa, "%d %d 0\n", i, j);
            }
            else
            {
                printf("Undifined words:%s\n",buffer);
                exit(0);
            }
        }

        for(j = 0; j < NONTERM; j ++)
        {
            fgets(buffer, LINESIZE, fs);
            buffer[strlen(buffer) - 1] = '\0';
            if(!strcmp(buffer, "e"))
            {
                    continue;
            }
            else
            {
                fprintf(fg, "%d %d %s\n", i, j, buffer);
            }
        }
        fgets(buffer, LINESIZE, fs);    // ȡ��</tr>
    }
    printf("Successful!\n");
    return 0;
}
