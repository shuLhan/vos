#ifndef _VOS_TYPE_FILE_H
#define	_VOS_TYPE_FILE_H	1

enum _file_open_flag {
	FOPEN_RO	= 1,
	FOPEN_WO	= 2
};

/**
 * @desc:
 *	d	- descriptor
 *	idx	- index of buffer
 *	buf	- content of file
 *	size	- size of buf
 *	pos	- pointer to last file position from last read/write.
 *		to get the current file position: pos + idx
 */
struct File {
	int		d;
	long int	idx;
	long int	size;
	unsigned long	pos;
	const char	*name;
	char		*buf;
};

#endif
