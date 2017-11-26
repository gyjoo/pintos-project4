#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"
#include "threads/malloc.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size, bool is_dir) 
{
  block_sector_t inode_sector = 0;
  struct dir *dir = get_dir(name);
  char* filename = get_filename(name);
  
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, is_dir)
                  && dir_add (dir, name, inode_sector));

  if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
    success = false;

  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);

  dir_close (dir);
  free(filename);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  struct dir *dir = dir_open_root ();
  struct inode *inode = NULL;

  if (dir != NULL)
    dir_lookup (dir, name, &inode);
  dir_close (dir);

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  struct dir *dir = dir_open_root ();
  bool success = dir != NULL && dir_remove (dir, name);
  dir_close (dir); 

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}

/* Get directory path */
struct dir* 
get_dir (const char* path) 
{
  struct dir* dir;
  int length = strlen(path);
  char copied_path[length + 1];
  memcpy(copied_path, path, length + 1);

  if (copied_path[0] == "/" || !thread_current()->current_dir)
    dir = dir_open_root();
  else 
    dir = dir_reopen(thread_current()->current_dir);

  char *ptr, *next = NULL, *cur = strtok_r(copied_path, "/", &ptr);
  if (cur)
    next = strtok_r(NULL, "/", &ptr);

  while (next != NULL)
  {
    struct inode* inode;
    if (strcmp(cur, ".") == 0) continue;
    else if(strcmp(cur, "..") == 0)
    {
      inode = dir_parent_inode(dir);
      if(inode == NULL) 
        return NULL;
    }
    else if(dir_lookup(dir, next, &inode) == false)
      return NULL;

    if(inode_is_dir(inode))
    {
      dir_close(dir);
      dir = dir_open(inode);
    }
    else
    {
      inode_close(inode);
    }

    cur = next;
    next = strtok_r(NULL, "/", &ptr);
  }

  return dir;
}

/* Get filename from the given string */
char* 
get_filename (const char* path)
{
  char copied_path[strlen(path) + 1];
  memcpy(copied_path, path, strlen(path) + 1);

  char *ptr, *next, *cur = "";
  next = strtok_r(copied_path, "/", &ptr);
  
  while (next != NULL)
  {
    cur = next;
  }
  
  char *filename = malloc(strlen(cur) + 1);
  
  memcpy(filename, cur, strlen(cur) + 1);
  return filename;
}
