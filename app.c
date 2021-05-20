#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simplefs.h"

int main(int argc, char **argv)
{
    int ret;
    int fd1, fd2, fd;
    int i;
    char c;
    char buffer[1024];
    char buffer2[8] = {50, 50, 50, 50, 50, 50, 50, 50};
    int size;
    char vdiskname[200];

    printf("started\n");

    if (argc != 2)
    {
        printf("usage: app  <vdiskname>\n");
        exit(0);
    }
    strcpy(vdiskname, argv[1]);

    ret = sfs_mount(vdiskname);
    if (ret != 0)
    {
        printf("could not mount \n");
        exit(1);
    }

    printf("creating files\n");
    sfs_create("file1.bin");
    sfs_create("file2.bin");
    sfs_create("file3.bin");

    fd1 = sfs_open("file1.bin", MODE_APPEND);
    fd2 = sfs_open("file2.bin", MODE_APPEND);
    for (i = 0; i < 10000; ++i)
    {
        buffer[0] = (char)65;
        sfs_append(fd1, (void *)buffer, 1);
    }

    printf("succesfully appended 1st file\n");

    for (i = 0; i < 10000; ++i)
    {
        buffer[0] = (char)65;
        buffer[1] = (char)66;
        buffer[2] = (char)67;
        buffer[3] = (char)68;
        sfs_append(fd2, (void *)buffer, 4);
    }

    printf("succesfully appended 2nd File\n");

    sfs_close(fd1);
    sfs_close(fd2);

    printf("succesfully closed 1st and 2nd file\n");
    
    fd = sfs_open("file3.bin", MODE_APPEND);
    for (i = 0; i < 10000; ++i)
    {
        memcpy(buffer, buffer2, 8); // just to show memcpy
        sfs_append(fd, (void *)buffer, 8);
    }
    sfs_close(fd);

    printf("succesfully appended 3rd File\n");

    printf("Now reading the third file...\n");

    fd = sfs_open("file3.bin", MODE_READ);
    size = sfs_getsize(fd);
    for (i = 0; i < size; ++i)
    {
        sfs_read(fd, (void *)buffer, 1);
        c = (char)buffer[0];
        printf("%d ", c); //Comment this to prevent printing at output
        c = c + 1;
    }

    printf("\nSuccesfully read third file\n");

    sfs_close(fd);
    ret = sfs_umount();
}
