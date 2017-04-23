/*
 * File-related system call implementations.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <copyinout.h>
#include <vfs.h>
#include <vnode.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>
#include <limits.h>

/*
 * open() - get the path with copyinstr, then use openfile_open and
 * filetable_place to do the real work.
 */
int
sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
{
	const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

	char *kpath;
	struct openfile *file;
	int result = 0;

	kpath = (char*)kmalloc(sizeof(char)*PATH_MAX)
	
	/* 
	 * Your implementation of system call open starts here.  
	 *
	 * Check the design document design/filesyscall.txt for the steps
	 
	(void) upath; // suppress compilation warning until code gets written
	(void) flags; // suppress compilation warning until code gets written
	(void) mode; // suppress compilation warning until code gets written
	(void) retval; // suppress compilation warning until code gets written
	(void) allflags; // suppress compilation warning until code gets written
	(void) kpath; // suppress compilation warning until code gets written
	(void) file; // suppress compilation warning until code gets written*/
	
	if (flags == allflags)
	{
		return EINVAL;
	}
	
	result = copyinstr(upath, kpath, PATH_MAX, NULL);
	
	if (result)
	{
		return EFAULT;
	}
	
	result = openfile_open(kpath, flags, mode, &file)
	
	if (result)
	{
		return EFAULT;
	}
	
	result = filetable_place(curproc->p_filetable, file, retval);
	
	if (result)
	{
		return EMFILE;
	}
	
	kfree(kpath);
	return result;
}

/*
 * read() - read data from a file
 */
int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
	int result = 0;

       /* 
        * Your implementation of system call read starts here.  
        *
        * Check the design document design/filesyscall.txt for the steps
        *
       (void) fd; // suppress compilation warning until code gets written
       (void) buf; // suppress compilation warning until code gets written
       (void) size; // suppress compilation warning until code gets written
       (void) retval; // suppress compilation warning until code gets written*/
	   
	struct openfile *file;
	struct iovec io;
	struct uio user;
	   
	char *buffer = (char*)kmalloc(size);
	result = filetable_get(cuproc->p_filetable, fd, &file);
	   
	if (result)
	{
		return result;   	
	}
	
	if (file->of_accmode == O_WRONLY)
	{
		return EPERM;
	}
	
	uio_knit(&io, &user, buffer, size, file->of_offset, UIO_READ);
	
	result = VOP_READ(file->of_vnode, &user);
	
	if (result)
	{
		return result;
	}
	
	result = copyout(buffer, (userptr_t)buf, size);
	
	if (result)
	{
		return result;
	}
	
	file->of_offset = user.uio_offset;
	filetable_put(curproc->p_filetable, fd, file);
	
	*retval = size - user.uio_resid;

    return result;
}

/*
 * write() - write data to a file
 */
 
 int
 sys_write(int fd, userptr_t buf, size_t size, int *retval)
 {
 	int result = 0;
	struct openfile *file;
	struct iovec io;
	struct uio user;
	size_t secondSize;
	char *buffer = (char*)kmalloc(size);
	int intSize = size;
	
	result = filetable_get(curproc->p_filetable, fd, &file);
	
	if (result)
	{
		return result;
	}
	
	if (file->of_accmode == O_RDONLY)
	{
		return EPERM;
	}
	
	copyinstr((userptr_t)buf, buffer, intSize, &secondSize);
	
	uio_knit(&io, &user, buffer, size, file->of_offset, UIO_WRITE);
	
	result = VOP_WRITE(file->of_vnode, &user);
	
	if (result)
	{
		return result;
	}
	
	file->of_offset = user.uio_offset;
	filetable_put(curproc->p_filetable, fd, file);
	*retval = size - user.uio_resid;
	
	kfree(buffer);
	
	return result;
 }

/*
 * close() - remove from the file table.
 */

int
sys_close(int fd)
{
	struct openfile *file = NULL;
	struct openfile *firstFile;
	
	if (filetable_okfd(curproc->p_filetable, fd))
	{
		filetable_placeat(curproc->p_filetable, file, fd, &firstFile);
		if (firsFile != NULL)
		{
			openfile_decref(oldfile);
		}
	}
	
	return 0;
}

/* 
* encrypt() - read and encrypt the data of a file
*/

int
sys_encrypt(const_userptr_t path, size_t size)
{
	int fd, ret;
	mode_t m = 0664;
	struct openfile *file;
	struct iovec io;
	struct uio user;
	int result = 0;
	unsigned long int mask;
	unsigned long int buffer;
	unsigned long int arr[size/4];
	size_t x, y = 0, z = 0;
	
	char *kpath = (char*)kmalloc(sizeof(char)*PATH_MAX);
	
	result = copyinstr(path, kpath, PATH_MAX, NULL);
	
	if (result)
	{
		result;
	}
	
	result = openfile_open(path, O_RDONLY, m, &file);
	
	if (result)
	{
		return result;
	}
	
	result = filetable_place(curproc->p_filetable, file, &ret);
	
	if (result)
	{
		return result;
	}
	
	fd = result;
	
	while (z < size)
	{
		if (file->of_accmode == O_WRONLY)
		{
			return result;
		}
		
		uio_kinit(&io, &user, &buffer, 4, file->of_offset, UIO_READ);
		
		result = VOP_READ(file->of_vnode, &user);
		
		if (result)
		{
			return result;
		}
		
		arr[y] = buffer;
		
		for (x = 0; x < 10; x++)
		{
			if ((arr[y] % 2) == 0)
			{
				mask = 0 << 31;
			}
			else
			{
				mask = 1 << 31;
			}
			
			arr[y] = arr[y] >> 1;
			arr[y] = arr[y] | mask;
		}
		
		y++;
		z = z + 4;
		file->of_offset = z;
	}
	
	z = 0;
	struct openfile *nextFile = NULL;
	struct openfile *firstFile;
	
	if (filetable_okfd(curproc->p_filetable, fd))
	{
		filetable_placeat(curproc->p_filetable, nextFile, fd, &firstFile);
	
		if (firstFile != NULL)
		{
			openfile_decref(firstFile);
		}
	}
	
	
	
	file = NULL;
	m = 0664;
	
	result = openfile_open(kpath, O_CREAT|O_WRONLY|O_TRUNC, m, &file);
	
	if (result)
	{
		return result;
	}
	
	result = filetable_place(curproc->p_filetable, file, &ret);
	
	if (result)
	{
		return result;
	}
	
	kfree(kpath);
	fd = result;
	
	while (z < y)
	{
		result = filetable_get(curproc->p_filetable, fd, &file);
		
		if (result)
		{
			return result;
		}
		
		if (file->of_accmode == O_RDONLY)
		{
			return EPERM;
		}
		
		uio_kinit(&io, &user, arr[z], 4, file->of_offset, UIO_WRITE);
		
		result = VOP_WRITE(file->of_vnode, &user);
		
		if (result)
		{
			return result;
		}
		
		file->of_offset = file->of_offset + 4;
		z++;
	}
	
	filetable_put(curproc->p_filetable, fd, file);
	
	
	
	nextFile = NULL;
	
	if (filetable_okfd(curproc->p_filetable, fd))
	{
		filetable_placeat(curproc->p_filetable, nextFile, fd, &firstFile);
		
		if (firstFile != NULL)
		{
			openfile_decref(firstFile);
		}
	}
	
	return 0;
}