#include "user/user.h"

void prime(int p[2]) {
  close(p[1]);
  int mod;
  if (read(p[0], &mod, 4) == 0) {
    close(p[0]);
    exit(0);
  }
  fprintf(1,"prime %d\n",mod);
  int p1[2];
  pipe(p1);
  if (fork() == 0) {
    prime(p1);
  } else {
    close(p1[0]);    
    int num;
    while (read(p[0],&num,4)) {
      if (num % mod != 0) {
          write(p1[1],&num,4);
        }
    }
    close(p[0]);
    close(p1[1]);
    wait(0);
  }
  exit(0);
}

int main() {
  int p[2];
  pipe(p);
  if (fork()==0) {
    prime(p);
  } else {
    close(p[0]);
    for (int i = 2; i <= 35; i++) {
      write(p[1],&i,4);
    }
    close(p[1]);
    wait(0);
  }
  exit(0);
}