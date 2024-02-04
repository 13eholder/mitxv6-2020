#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(2, "usage: xargs <command> <arguments>\n");
    exit(1);
  }
  int i,len=0;
  char *args[MAXARG];
  char buf[512];
  char c;
  memset(buf, 0, 512);
  for (i = 1; i < argc; i++) {
    args[i - 1] = (char *)malloc(strlen(argv[i] + 1));
    strcpy(args[i - 1], argv[i]);
    args[i-1][strlen(argv[i])]='\0';
  }
  int start = i-1;
  while (read(0, &c, 1)) {
    if (c != '\n')
      buf[len++] = c;
    else{
      buf[len] = '\0';
      args[start++] = (char *)malloc(strlen(buf + 1));
      strcpy(args[start - 1], buf);
      args[start-1][strlen(buf)]='\0';
      len = 0;
      if (fork() == 0) {
        exec(argv[1], args);
      } else {
        wait(0);
        for (int j = i; j < start; j++)
          free(args[j]);
        start=i-1;
      }
    }
  }
  for (int j = 0; j < i - 2; j++)
    free(args[j]);
  exit(0);
}