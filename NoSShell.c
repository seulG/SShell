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

//生成每一条命令的结构体
node createNode(char *cmdStr,int length) {
    char *p = cmdStr;
    //去掉空格和｜
    node cnode = (node)malloc(sizeof(CNode));

    while(*p == '|' || *p == '\0') {
        p++;
        length--;
        //标记上一条命令启用了管道
        cnode->flag = 1;
    }
    
    //经过处理后第一个字符串就是命令名
    strcpy(cnode->cmdName,p);

    int num = 0;
    int index = 0;
    int flag = 0;
    while(num < length -1) {
        if(*p == '>' || *p == '<') {
            //标记位，不再继续向paras里面存入参数
            flag = 1;
            if(*p == '>') {
                p++;
                //>>的模式
                if(*p == '>') {
                    p++;
                    strcpy(cnode->outRedir,++p);
                } else if(*p == '\0') { 
                    //>模式
                    strcpy(cnode->outRedir,++p);
                }
            } else {
                //<<模式
                if(*p == '<') {
                    p++;
                    strcpy(cnode->inReDir,++p);
                } else if(*p == '\0') {
                    strcpy(cnode->inReDir,++p);
                }

            }
        }
        //因为一直在操作一个数组，所以要严格控制此处的代码不能越界
        while(*p != '\0') {
            p++;
            num++;
        }
        /* cnode->paras[index] = ++p; */
        p++;
        //这里会出现越界，而两个程序的链接点为|,因此可以规避
        //并且当遇到重定向就可以停止向paras写入参数了
        if(*p != '|' && flag == 0 && *p != '>' && *p != '<') {
            //c里面只有数组才能调用strcpy，char *类型会出现段错误
            /* strcpy(cnode->paras[index],p); */
            cnode->paras[index] = p;
            index++;
        }
        num++;
    }
    //给参数最后一个指针设置为NULL
    cnode->paras[index] = NULL; 
    return cnode;
}

//展示单个节点的信息
void showNode(node cnode) {
    int index = 0;
    printf("命令名：%s\n",cnode->cmdName);
    while(cnode->paras[index]) {
        printf("参数%d为：%s\n",index,cnode->paras[index++]);
    }
    printf("输入文件：%s\n",cnode->inReDir);
    printf("输出文件：%s\n",cnode->outRedir);
    printf("flag：%d\n",cnode->flag);
    printf("#########\n");
}

void showLinkList(node head) {
    node p;
    if(head->next)
        p = head->next;
    while(p) {
        showNode(p);
        p = p->next;
    }
}

//emm主要用来弥补我不太合理的设计
node parasHandler(char *subInputStr,int length) {
    char *p = subInputStr;
    node cnode;
    if(*p == '|') {
        cnode = createNode(p,length);
    } else {
        cnode = createNode(p,length);
    }
    return cnode;
}

//粗粒度切割,组装链表
node getSubStr(char *inputStr) {
    if(*inputStr == '\0'){
        printf("%c\n",*inputStr);
        return NULL;
    }
    char *p = inputStr;
    char *a = inputStr;
    int num = 0,offset = 0;
    node head = (node)malloc(sizeof(CNode));
    node np = head,cnode;
    do {
        if(*p == '|') {
            cnode  = parasHandler(a,num-offset);
            np->next = cnode;
            np = np->next;
            offset = num;
            a = p;
        }
        num++;
        p++;
    }while(num < number);
    cnode  = parasHandler(a,num-offset);
    np->next = cnode;
    return head;
}

int main()
{
    int yes = 1;
    do {
        /* get_prompt(); */
        printf("> ");
        
        char *inputStr = getFromStd();
        node head = getSubStr(inputStr); 
        if(head == NULL) {
            continue;
        }
        showLinkList(head);
    } while(yes);
}
