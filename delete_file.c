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
    printf("usage: ./deletefile  <vdiskname> <filename>\n");
    exit(0);
  }

  strcpy(vdiskname, argv[1]);
  strcpy(filename, argv[2]);

  printf("Deleting File %s\n", filename);

  ret = sfs_mount(vdiskname);
  if (ret != 0)
  {
    printf("could not mount \n");
    exit(1);
  }

  ret = sfs_delete(filename);

  if (ret == -1)
  {
    printf("could not be deleted\n");
  }
  else
  {
    printf("successfuly deleted\n");
  }

  ret = sfs_umount();
}
