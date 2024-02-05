#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void find(char *directory, char *filename) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(directory, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", directory);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", directory);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    fprintf(2, "wrong args: arg1 must be a directory\n");
    close(fd);
    return;
  }
  if(strlen(directory) + 1 + DIRSIZ + 1 > sizeof buf){
    printf("ls: path too long\n");
  }
  strcpy(buf, directory);
  p = buf+strlen(buf);
  *p++ = '/';
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }
    if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) 
      find(buf, filename);
    else if (strcmp(filename, p) == 0)
      printf("%s\n", buf);
  }
  close(fd);
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "usage: find directory filename\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}