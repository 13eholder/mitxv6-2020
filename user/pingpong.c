#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(2,"usage: pingpong\n");
    exit(1);
  }
  int p[2];
  int p1[2];
  char buf[2];
  pipe(p);
  pipe(p1);
  if (fork() == 0) {
    close(p[1]);
    close(p1[0]);
    read(p[0], buf, 1);
    close(p[0]);
    fprintf(1, "%d: received ping\n",getpid());
    write(p1[1], buf, 1);
    close(p[1]);
  } else {
    close(p[0]);
    close(p1[1]);
    write(p[1], "z", 1);
    close(p[1]);
    read(p1[0], buf, 1);
    close(p1[0]);
    fprintf(1, "%d: received pong\n", getpid());
  }
  exit(0);
}