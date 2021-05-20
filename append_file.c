#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simplefs.h"

int main(int argc, char **argv)
{
  int ret;
  int fd;
  char vdiskname[200];
  char filename[200];
  char command[100];

  system(command);
  

  if (argc != 3)
  {
    printf("usage: ./appendfile  <vdiskname> <filename>\n");
    exit(0);
  }

  strcpy(vdiskname, argv[1]);
  strcpy(filename, argv[2]);

  printf("Reading File %s\n", filename);

  ret = sfs_mount(vdiskname);
  if (ret != 0)
  {
    printf("could not mount \n");
    exit(1);
  }

  fd = sfs_open(filename, MODE_APPEND);

  if (fd == -1)
  {
    printf("could not open file :(\n");
    exit(1);
  }

  int n;
  printf("Enter number of chars you want to append:");
  scanf("%d", &n);

  char *append = (char *)malloc(5000);
  printf("Enter append content: \n");
  scanf("%s", append);

  printf("append content recieved : \n%s\n", append);

  ret = sfs_append(fd, append, n);

  if (ret == -1)
  {
    printf("could not be appended\n");
  }
  else
  {
    printf("successfuly appended\n");
  }

  free(append);

  sfs_close(fd);
  ret = sfs_umount();
}
