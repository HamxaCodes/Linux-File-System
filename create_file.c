#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simplefs.h"

int main(int argc, char **argv)
{
  int ret;
  char vdiskname[200];
  char filename[200];

  if (argc != 3)
  {
    printf("usage: ./createfile  <vdiskname> <filename>\n");
    exit(0);
  }

  strcpy(vdiskname, argv[1]);
  strcpy(filename, argv[2]);

  printf("Creating File %s\n", filename);

  ret = sfs_mount(vdiskname);
  if (ret != 0)
  {
    printf("could not mount \n");
    exit(1);
  }

  ret = sfs_create(filename);

  if (ret == -1)
  {
    printf("could not be created\n");
  }
  else
  {
    printf("successfuly created\n");
  }

  ret = sfs_umount();
}
