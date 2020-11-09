#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>

#define READ_BUFFER 1024
#define PARAS_BUFFER 20
#define STR_TOK " "

char *gsh_read_line(){
    int bufsize = READ_BUFFER;
    int position = 0;
    char *buffer = (char *) malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr,"内存分配失败\n");
        return 0;
    }

    while(1) {
        c = getchar();
        if(c==EOF || c=='\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize) {
            bufsize = bufsize + READ_BUFFER;
            buffer = (char *)realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr,"内存分配失败\n");
                return 0;
            }
        }
    }
    return buffer;
}


/*     while(command[i] != '\0') { */
/*         i++; */
/*     } */
/*     return i; */
/* } */

/* int getlens(char **paras) { */
/*     int i = 0; */
/*     while(paras[i]!=NULL) { */
/*         i++; */
/*     } */
/*     return i; */
/* } */

/* void copychar(char *source ,char *target,int index){ */
/*     int len = getlen(source); */
/*     int pos = 0; */
/*     while(pos < len) { */
/*         target[index] = source[pos]; */
/*         index++; */
/*         pos++; */
/*         printf("%c\n",target[index]); */
/*     } */
/* } */

/* int get_totol_len(char **paras, int start, int end) { */
/*     int totle = 0; */
/*     while(start <= end) { */
/*         totle += getlen(paras[start]); */
/*         start++; */
/*     } */
/*     return totle; */
/* } */


char **split_command(char *command) {
    int position=0,buffersize=PARAS_BUFFER,mergesize=PARAS_BUFFER,mergepos=0,len;
    char **paras = (char **)malloc(sizeof(char *) * buffersize);
    char *para;

    if(!para) {
        fprintf(stderr,"分配内存失败");
    }

    para = strtok(command,STR_TOK);
    while(para != NULL) {
        paras[position] = para;
        position++;
        if(position >= buffersize) {
            buffersize+=PARAS_BUFFER;
            paras = (char **)realloc(paras,buffersize);
            if(!paras) {
                fprintf(stderr,"内存分配失败");
            }
        }
        para = strtok(NULL,STR_TOK);
    }
    paras[position] = NULL;

    return paras;
}

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


/* 内置命令 */

/* cd */
int gsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

/* exit */

int gsh_exit(char **args)
{
  return 0;
}

char *innercmd[] = {
    "cd",
    "exit",
    "ll"
};

/* 函数声明 */
int launch_command(char **paras);
int launch_innercmd(int i, char **paras);

int command_execute(char **paras) {
   int i;
   if(paras[0] == NULL) {
        return 1;
   }

   for(i=0;i<sizeof(innercmd)/8;i++) {
       if(strcmp(innercmd[i],paras[0]) == 0) {
           return launch_innercmd(i,paras);
       }
   }
   return launch_command(paras);
}
/* 执行内部命令 */
int launch_innercmd(int i, char **paras) {
    switch(i) {
    case 0:
        return gsh_cd(paras);
        break;
    case 1:
        return gsh_exit(paras);
        break;
    }
}

//执行外部命令
int launch_command(char **paras) {
    int pid,wpid;
    int status;
    pid = fork();

    if(pid == 0) {
        if(execvp(paras[0],paras) == -1) {
            perror("error");
        }
        exit(1);
    } else if(pid < 0) {
        perror("命令启动失败");
    } else {
        do {
            wpid = waitpid(pid,&status,0);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int main()
{
    int yes;
    do {
        get_prompt();
        char *command = gsh_read_line();
        char **paras = split_command(command);
        yes = command_execute(paras);
    } while(yes);
}
