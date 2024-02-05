#include "user/user.h"
#include "kernel/types.h"


int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(2, "usage: sleep time \n");
    exit(1);
  }
  int time = atoi(argv[1]);
  sleep(time);
  exit(0);
}