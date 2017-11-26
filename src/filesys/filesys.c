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

/*
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

*/

// MODEL SOLUTION

  block_sector_t inode_sector = 0;
struct dir *dir = get_dir(name);
char* file_name = get_filename(name);
bool success = false;
if (strcmp(file_name, ".") != 0 && strcmp(file_name, "..") != 0)
  {
    success = (dir != NULL
   && free_map_allocate (1, &inode_sector)
   && inode_create (inode_sector, initial_size, is_dir)
   && dir_add (dir, file_name, inode_sector));
  }
if (!success && inode_sector != 0)
  free_map_release (inode_sector, 1);
dir_close (dir);
free(file_name);

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

  char s[strlen(path) + 1];
  memcpy(s, path, strlen(path) + 1);

  char *save_ptr, *next_token = NULL, *token = strtok_r(s, "/", &save_ptr);
  struct dir* dir;
  if (s[0] == "/" || !thread_current()->current_dir)
    {
      dir = dir_open_root();
    }
  else
    {
      dir = dir_reopen(thread_current()->current_dir);
    }

  if (token)
    {
      next_token = strtok_r(NULL, "/", &save_ptr);
    }
  while (next_token != NULL)
    {
      if (strcmp(token, ".") != 0)
	{
	  struct inode *inode;

    printf(dir_parent_inode(dir));

	  if (strcmp(token, "..") == 0)
	    {
	      if (dir_parent_inode(dir) == NULL)
		{
		  return NULL;
		}
	    }
	  else
	    {
	      if (!dir_lookup(dir, token, &inode))
		{
		  return NULL;
		}
	    }
	  if (inode_is_dir(inode))
	    {
	      dir_close(dir);
	      dir = dir_open(inode);
	    }
	  else
	    {
	      inode_close(inode);
	    }
	}
      token = next_token;
      next_token = strtok_r(NULL, "/", &save_ptr);
    }
  return dir;

/*
// My code
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

    // printf("hey!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  while (next != NULL)
  {
    printf(next);


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
    printf(next);
  }

  return dir;

  */
}

/* Get filename from the given string */
char*
get_filename (const char* path_name)
{

  char copied_path[strlen(path_name) + 1];
  memcpy(copied_path, path_name, strlen(path_name) + 1);

  char *ptr, *next, *cur = "";
  next = strtok_r(copied_path, "/", &ptr);
  while( next != NULL)
  {
    cur = next;
    next = strtok_r(NULL, "/", &ptr);
  }
  char *filename = malloc(strlen(cur) + 1);

  memcpy(filename, cur, strlen(cur) + 1);
  return filename;

}

// MUST BE MODIFIED!!!!!!!!!!!!! NOT MINE
bool filesys_chdir (const char* name)
{
  struct dir* dir = get_dir(name);
  char* file_name = get_filename(name);
  struct inode *inode = NULL;

  if (dir != NULL)
    {
      if (strcmp(file_name, "..") == 0)
	{
	  if (!dir_parent_inode(dir))
	    {
	      free(file_name);
	      return false;
	    }
	}
      else if ((dir_is_root(dir) && strlen(file_name) == 0) ||
	  strcmp(file_name, ".") == 0)
	{
	  thread_current()->current_dir = dir;
	  free(file_name);
	  return true;
	}
      else
	{
	  dir_lookup (dir, file_name, &inode);
	}
    }

  dir_close (dir);
  free(file_name);

  dir = dir_open (inode);
  if (dir)
    {
      dir_close(thread_current()->current_dir);
      thread_current()->current_dir = dir;
      return true;
    }
  return false;
}
