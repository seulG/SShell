#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>

#define READFROMSTD_BUFFER 512
#define PARAS_BUFFER 20
#define PARA_BUFFER 124

typedef struct CNode {
    char cmdName[PARA_BUFFER];
    char *paras[PARAS_BUFFER];  
    char inReDir[PARA_BUFFER];
    char outRedir[PARA_BUFFER];
    int flag;
    struct CNode *next;
} CNode, *node;

//全局变量记录输入字符个数
int number;

//打印命令提示符部分
char *get_prompt() {
    struct passwd *pwd;
    char hostname[10];
    char dir[1024];
    char permission;
    gethostname(hostname,sizeof(hostname));
    getcwd(dir,sizeof(dir));
    permission = (getuid()==0) ? '#' : '$';
    pwd = getpwuid(getuid());
    printf("%s@%s:%s\n%c ",pwd->pw_name,hostname,dir,permission);
}

//从标准输入中读取，并且初步处理，将空格替换为'\0'
char *getFromStd() {
    int bufsize = READFROMSTD_BUFFER;
    number = 0;
    char *buffer = (char *) malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr,"内存分配失败\n");
        return 0;
    }

    while(1) {
        c = getchar();
        if(c==EOF || c=='\n') {
            buffer[number] = '\0';
            return buffer;
        } else if(c==' ') {
            buffer[number] = '\0';
        } else {
            buffer[number] = c;
        }

        number++;

        //内存不够时，动态申请更多的内存空间
        if(number >= bufsize) {
            bufsize = bufsize + READFROMSTD_BUFFER;
            buffer = (char *)realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr,"内存分配失败\n");
                return 0;
            }
        }
    }
    return buffer;
}


node creatNode() {
    node head = (node)malloc(sizeof(CNode));
    head->next = NULL;
    return head;
}

/* void showLinkList(node head) { */
/*     node Chead = head; */
/*     while(Chead->next) { */
/*         printf("%s",Chead->cmdName); */
/*         int index = 0; */
/*         while(1) { */
/*             printf("%s\n",Chead->paras[index]); */
/*             index++; */
/*             if(Chead->paras[index]==NULL || Chead->paras[index] == "") { */
/*                 break; */
/*             } */
/*         } */
/*     } */
/*     printf("yes"); */

/* } */

//生成每一条命令的结构体
node createNode(char *cmdStr,int length) {
    char *p = cmdStr;
    //去掉空格和｜
    while(*p == '|' || *p == '\0') {
        p++;
        length--;
    }
    
    node cnode = (node)malloc(sizeof(CNode));
    //经过处理后第一个字符串就是命令名
    strcpy(cnode->cmdName,p);
    /* copy(cnode->cmdName,p); */
    printf("%s\n",cnode->cmdName);

    int num = 0;
    int index = 0;
    while(num < length -1) {
        if(*p == '>' || *p == '<') {
            if(*p == '>') {
                p++;
                //>>的模式
                if(*p == '>') {
                    p++;
                    printf("%s\n",++p);
                    /* cnode->outRedir = ++p; */
                } else if(*p == '\0') { 
                    //>模式
                    /* cnode->outRedir = ++p; */
                    printf("%s\n",++p);
                }
            } else {
                //<<模式
                if(*p == '<') {
                    p++;
                    /* cnode->inReDir = ++p; */
                    printf("%s\n",++p);
                } else if(*p == '\0') {
                    //<模式
                    /* cnode->inReDir = ++p; */
                    printf("%s\n",++p);
                }

            }
            break;    
        }
        
        //因为一直在操作一个数组，所以要严格控制此处的代码不能越界
        while(*p != '\0') {
            p++;
            num++;
        }
        /* cnode->paras[index] = ++p; */
        p++;
        //这里会出现越界，而两个程序的链接点为|,因此可以规避
        if(*p != '|') {
            //c里面只有数组才能调用strcpy，char *类型会出现段错误
            /* strcpy(cnode->paras[index],p); */
            cnode->paras[index] = p;
            printf("%s ",cnode->paras[index]);
            index++;
        }
        num++;
    }

    //给参数最后一个指针设置为NULL
    cnode->paras[index] = NULL; 
    /* printf("%s\n",cnode->cmdName); */
    /* int i = 0; */
    /* while(i<index) { */
    /*     printf("%s ",cnode->paras[i]); */
    /*     i++; */
    /* } */
    return cnode;
}




//简单处理
int parasHandler(char *subInputStr,int length) {
    printf("\n");
    char *p = subInputStr;
    if(*p == '|') {
        node cnode = createNode(p,length);
    } else {
        node cnode = createNode(p,length);
    }
    
}

//粗粒度切割
int getSubStr(char *inputStr) {
    char *p = inputStr;
    char *a = inputStr;
    int num = 0,offset = 0;
    do {
        if(*p == '|') {
            parasHandler(a,num-offset);
            offset = num;
            a = p;
        }
        num++;
        p++;
    }while(num < number);
    parasHandler(a,number-offset);
}

int main()
{
    int yes;
    do {
        /* get_prompt(); */
        printf("> ");
        
        char *inputStr = getFromStd();
        getSubStr(inputStr); 
    } while(yes);
}
