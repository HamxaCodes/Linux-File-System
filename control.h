
#ifndef CONTROL
#define CONTROL

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define MAXFILESNUM 112
#define MAXFILENAME 20
#define NUMBITBLOCKS 4
#define NUMDIRBLOCKS 4
#define NUMFCBS 4
#define FATBLOCK_COUNT 256

typedef struct Super_Block
{
  int num_blocks;
  unsigned int free_fcb[MAXFILESNUM];
  char dummy[BLOCKSIZE - 452];
} SB;

void init_superblock(int disk_size)
{
    SB *superblock = (SB *)malloc(sizeof(SB));

    superblock->num_blocks = disk_size / BLOCKSIZE;

    int i;
    for (i = 0; i < MAXFILESNUM; ++i)
    {
        superblock->free_fcb[i] = 0;
    }

    write_block(superblock, 0);
    free(superblock);
}

typedef struct Directory_Entry
{
  char name[MAXFILENAME];
  int file_size;
  int first_block;
  char dummy[256 - sizeof(int) * 2 - sizeof(char) * MAXFILENAME];
} DENTRY;

typedef struct Directory_Block
{
  DENTRY entries[BLOCKSIZE / sizeof(DENTRY)];
} DB;

void init_dir_block()
{
  DB *cur_dir_block;

  cur_dir_block = (DB *)malloc(sizeof(DB));
  int i, j;

  for (j = 0; j < BLOCKSIZE / sizeof(DENTRY); ++j)
  {
    cur_dir_block->entries[j].name[0] = '\0';
    cur_dir_block->entries[j].file_size = 0;
    cur_dir_block->entries[j].first_block = 0;
  }

  for (i = 0; i < NUMDIRBLOCKS; i++)
  {
    write_block(cur_dir_block, i + 1 + NUMBITBLOCKS);
  }

  free(cur_dir_block);
}

void copy_dir_entry(DENTRY *dest, DENTRY *source)
{

  dest->file_size = source->file_size;
  dest->first_block = source->first_block;

  strcpy(dest->name, source->name);
}


void get_dir_entry(int n, DENTRY *entry)
{
  int dir_block_num = n / (BLOCKSIZE / sizeof(DB));
  int offset = n - dir_block_num * (BLOCKSIZE / sizeof(DB));

  DB *cur_dir_block;
  cur_dir_block = (DB *)malloc(sizeof(DB));

  read_block(cur_dir_block, dir_block_num + 1);

  copy_dir_entry(entry, cur_dir_block->entries + offset);

  free(cur_dir_block);
}

void set_dir_entry(int n, DENTRY *entry)
{
  int dir_block_num = n / (BLOCKSIZE / sizeof(DENTRY));
  int offset = n - dir_block_num * (BLOCKSIZE / sizeof(DENTRY));

  DB *cur_dir_block;
  cur_dir_block = (DB *)malloc(sizeof(DB));

  read_block(cur_dir_block, dir_block_num + 1);

  copy_dir_entry(cur_dir_block->entries + offset, entry);

  write_block(cur_dir_block, dir_block_num + 1);

  free(cur_dir_block);
}

int find_dir_entry(char *name, DENTRY *ret_entry)
{
  SB *sb;
  sb = (SB *)malloc(sizeof(SB));

  read_block(sb, 0);

  int i, j;
  DB *cur_dir_block;
  cur_dir_block = (DB *)malloc(sizeof(DB));
  for (i = 0; i < NUMDIRBLOCKS; i++)
  {
    read_block(cur_dir_block, i + 1);
    for (j = 0; j < BLOCKSIZE / sizeof(DENTRY); ++j)
    {
      if (sb->free_fcb[i * (BLOCKSIZE / sizeof(DENTRY)) + j] == 1 && (strcmp(name, cur_dir_block->entries[j].name) == 0))
      {
        copy_dir_entry(ret_entry, (cur_dir_block->entries + j));
        free(sb);
        free(cur_dir_block);
        return i * (BLOCKSIZE / sizeof(DENTRY)) + j;
      }
    }
  }
  free(sb);
  free(cur_dir_block);
  return -1;
}

int get_empty_dir()
{

  SB *sb;
  sb = (SB *)malloc(sizeof(SB));
  read_block(sb, 0);

  int i;

  for (i = 0; i < MAXFILESNUM; ++i)
  {
    if (sb->free_fcb[i] == 0)
    {
      free(sb);
      return i;
    }
  }

  free(sb);
  return -1;
}

void set_sb_dir(int k)
{
  SB *sb;
  sb = (SB *)malloc(sizeof(SB));
  read_block(sb, 0);
  sb->free_fcb[k] = 1;
  write_block(sb, 0);
  free(sb);
}


typedef struct Filecontrolblock
{
    int status;
} FCB;

typedef struct FCBblock
{
    FCB array[BLOCKSIZE / sizeof(FCB)];
} FCBBlock;


void init_fcb()
{

    FCBBlock *active_FCBBlock;
    active_FCBBlock = (FCBBlock *)malloc(sizeof(FCBBlock));

    int i, j;

    //Writing FCB's to disk

    for (j = 0; j < (BLOCKSIZE / sizeof(FCB)); j++)
    {
        active_FCBBlock->array[j].status = 0;
    }

    for (i = 0; i < NUMFCBS; i++)
    {
        write_block(active_FCBBlock, i + 1);
    }

    free(active_FCBBlock);
}

typedef struct FAT_Entry
{
  int is_full;
  int value;
} FATE;

void copy_fat_entry(FATE *dest, FATE *source)
{
  dest->is_full = source->is_full;
  dest->value = source->value;
}

unsigned int get_fat_entry_value(FATE *entry)
{
  unsigned int ret = entry->value;
  return ret;
}


typedef struct FAT_Block
{
  FATE array[BLOCKSIZE / sizeof(FATE)];
} FATB;

void init_fat_block()
{
  FATB *cur_fat_block;
  cur_fat_block = (FATB *)malloc(sizeof(FATB));

  int i, j;

  for (j = 0; j < BLOCKSIZE / sizeof(FATE); ++j)
  {

    if (j < FATBLOCK_COUNT + NUMDIRBLOCKS + 1)
      cur_fat_block->array[j].is_full = 1;
    else
    {
      cur_fat_block->array[j].is_full = 0;
    }
    cur_fat_block->array[j].value = 0;
  }

  write_block(cur_fat_block, 1 + NUMDIRBLOCKS);


  for (j = 0; j < BLOCKSIZE / sizeof(FATE); j++)
  {
    cur_fat_block->array[j].is_full = 0;
    cur_fat_block->array[j].value = 0;
  }

  for (i = 1; i < FATBLOCK_COUNT; i++)
  {
    write_block(cur_fat_block, i + 1 + NUMDIRBLOCKS);
  }

  free(cur_fat_block);
}

void get_fat_entry(int n, FATE *entry)
{
  int fat_block_num = n / (BLOCKSIZE / sizeof(FATE));
  int offset = n - fat_block_num * (BLOCKSIZE / sizeof(FATE));

  FATB *cur_fat_block;
  cur_fat_block = (FATB *)malloc(sizeof(FATB));

  read_block(cur_fat_block, fat_block_num + 1 + MAXFILESNUM * sizeof(DENTRY) / BLOCKSIZE);

  copy_fat_entry(entry, cur_fat_block->array + offset);

  free(cur_fat_block);
}

void write_fat_entry(int n, FATE *entry)
{
  int fat_block_num = n / (BLOCKSIZE / sizeof(FATE));
  int offset = n - fat_block_num * (BLOCKSIZE / sizeof(FATE));

  FATB *cur_fat_block;
  cur_fat_block = (FATB *)malloc(sizeof(FATB));

  read_block(cur_fat_block, fat_block_num + 1 + MAXFILESNUM * sizeof(DENTRY) / BLOCKSIZE);

  copy_fat_entry(cur_fat_block->array + offset, entry);

  write_block(cur_fat_block, fat_block_num + 1 + MAXFILESNUM * sizeof(DENTRY) / BLOCKSIZE);

  free(cur_fat_block);
}

int get_block_from_pos(DENTRY *fbc, int pos)
{

  int i;
  FATE *fe;
  fe = (FATE *)malloc(sizeof(FATE));

  int cur_block = fbc->first_block;

  for (i = 0; i < pos / BLOCKSIZE; i++)
  {
    get_fat_entry(cur_block, fe);
    cur_block = get_fat_entry_value(fe);
  }

  free(fe);
  return cur_block;
}

int get_next_fat(int cur_block)
{

  FATE *fe;
  fe = (FATE *)malloc(sizeof(FATE));
  get_fat_entry(cur_block, fe);
  cur_block = get_fat_entry_value(fe);

  free(fe);
  return cur_block;
}

int check_enough_blocks(int req_blocks)
{

  if (req_blocks <= 0)
    return 1;

  int i, j;
  int n_empty = 0;

  FATB *fb;
  fb = (FATB *)malloc(sizeof(FATB));

  for (i = 0; i < FATBLOCK_COUNT; i++)
  {
    read_block(fb, i + 1 + NUMDIRBLOCKS);

    for (j = 0; j < BLOCKSIZE / sizeof(FATE); ++j)
    {

      if (!fb->array[j].is_full)
      {
        n_empty++;

        if (n_empty == req_blocks)
        {
          free(fb);
          return 1;
        }
      }
    }
  }

  free(fb);
  return 0;
}

void write_blocks(DENTRY *fbc, void *buf, int size, int req_blocks)
{

  int i, j;

  if (size <= 0)
    return;

  int n_empty = 0;

  FATE *fe;
  fe = (FATE *)malloc(sizeof(FATE));

  int cur_block, empty_block;

  if (fbc->file_size % BLOCKSIZE == 0)
  {

    FATB *fb;
    fb = (FATB *)malloc(sizeof(FATB));
    int done = 0;

    for (i = 0; i < FATBLOCK_COUNT && !done; i++)
    {
      read_block(fb, i + 1 + NUMDIRBLOCKS);

      for (j = 0; j < BLOCKSIZE / sizeof(FATE) && !done; ++j)
      {

        if (!fb->array[j].is_full)
        {
          empty_block = i * (BLOCKSIZE / sizeof(FATE)) + j;
          done = 1;
        }
      }
    }
    free(fb);
    req_blocks--;

    if (fbc->file_size == 0)
    {
      fbc->first_block = empty_block;
      cur_block = fbc->first_block;
    }
    else
    {
      int last_block = get_block_from_pos(fbc, fbc->file_size - 1);

      FATE *fe;
      fe = (FATE *)malloc(sizeof(FATE));

      get_fat_entry(last_block, fe);
      fe->value = empty_block;
      write_fat_entry(last_block, fe);
      cur_block = empty_block;

      free(fe);
    }

    get_fat_entry(empty_block, fe);
    fe->is_full = 1;
    write_fat_entry(empty_block, fe);
  }
  else
  {
    cur_block = get_block_from_pos(fbc, fbc->file_size);
  }


  int offset = fbc->file_size % BLOCKSIZE;

  char *content = (char *)malloc(BLOCKSIZE);

  if (req_blocks == 0)
  {
    read_block(content, cur_block);
    strncpy(content + offset, (char *)buf, size);
    write_block(content, cur_block);
  }
  else
  {

    int *empty_blocks = (int *)malloc(sizeof(int) * req_blocks);

    FATB *fb;
    fb = (FATB *)malloc(sizeof(FATB));
    int done = 0;

    for (i = 0; i < FATBLOCK_COUNT && !done; i++)
    {
      read_block(fb, i + 1 + NUMDIRBLOCKS);

      for (j = 0; j < BLOCKSIZE / sizeof(FATE) && !done; ++j)
      {

        if (!fb->array[j].is_full)
        {

          empty_blocks[n_empty] = i * (BLOCKSIZE / sizeof(FATE)) + j;
          n_empty++;

          if (n_empty == req_blocks)
          {
            done = 1;
          }
        }
      }
    }
    free(fb);

    read_block(content, cur_block);
    strncpy(content + offset, (char *)buf, BLOCKSIZE - offset);

    buf = (char *)buf + (BLOCKSIZE - offset);
    size -= BLOCKSIZE - offset;

    write_block(content, cur_block);

    get_fat_entry(cur_block, fe);
    fe->value = empty_blocks[0];
    fe->is_full = 1;
    write_fat_entry(cur_block, fe);

    for (i = 0; i < n_empty; i++)
    {

      cur_block = empty_blocks[i];

      read_block(content, cur_block);
      strncpy(content, (char *)buf, BLOCKSIZE > size ? size : BLOCKSIZE);
      buf = (char *)buf + (BLOCKSIZE > size ? size : BLOCKSIZE);
      size -= (BLOCKSIZE > size ? size : BLOCKSIZE);
      write_block(content, cur_block);

      get_fat_entry(cur_block, fe);

      if (i == n_empty - 1)
        fe->value = -1;
      else
      {
        fe->value = empty_blocks[i + 1];
      }

      fe->is_full = 1;
      write_fat_entry(cur_block, fe);
    }
    free(empty_blocks);
  }

  free(fe);
  free(content);
}

DENTRY otp[MAXFILESNUM];
int modes[MAXFILESNUM];
int valid_otp[MAXFILESNUM];
int last_pos[MAXFILESNUM];
int entry_pos[MAXFILESNUM];

int add_otp_file(DENTRY *fcb, int mode, int ep)
{

  int i;
  int flag = 0;
  int fp;
  for (i = 0; i < MAXFILESNUM && !flag; i++)
  {
    if (valid_otp[i] == 0)
    {
      flag = 1;
      fp = i;
    }
  }
  if (flag == 0)
    return -1;

  copy_dir_entry(otp + fp, fcb);

  modes[fp] = mode;
  valid_otp[fp] = 1;
  entry_pos[fp] = ep;

  return fp;
}

void remove_otp_file(int fd)
{

  if (fd >= MAXFILESNUM || fd < 0)
    return;
  valid_otp[fd] = 0;
}

int check_possible_read(int fd, int n)
{
  if (otp[fd].file_size >= last_pos[fd] + n)
    return 1;

  return 0;
}

void advance_last_pos(int fd, int n)
{
  if (check_possible_read(fd, n))
    last_pos[fd] += n;
}

typedef struct Bitmap
{
    int isUsed;
} BMP;

typedef struct bitmapBlock
{
    BMP array[BLOCKSIZE / sizeof(BMP)];
} BMPblock;

void init_bitmap(int disk_size)
{}

#endif