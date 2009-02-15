#include "op/vos_File.h"

/**
 * @desc:
 *	FOPEN_RO : file not create if not exist, instead return an error
 *	FOPEN_WO : file will be create if not exist, and error if exist
 * @return:
 *	< 0			: success
 *	< E_MEM			: fail, out of memory
 *	< E_FILE_OPEN		: fail, cannot open file 'f'
 *	< E_FILE_EXIST		: fail, fail 'f' is exist
 *	< E_FILE_NOT_EXIST	: fail, file 'f' is not exit
 */
int file_open(struct File **F, const char *f, int flag)
{
	int s = 0;

	(*F) = (struct File *) calloc(1, sizeof(struct File));
	if (! (*F))
		return E_MEM;

	switch (flag) {
	case FOPEN_RO:
		(*F)->d = open(f, O_RDONLY);
		break;
	case FOPEN_WO:
		(*F)->size = _vos.file_buf_size;
		(*F)->d	= open(f, O_WRONLY | O_CREAT | O_EXCL,
				S_IRUSR | S_IWUSR);
		break;
	}

	if ((*F)->d < 0) {
		str_raw_copy(f, &_vos.e_sparm0);

		switch (errno) {
		case ENOENT:
			return E_FILE_NOT_EXIST;
		case EEXIST:
			return E_FILE_EXIST;
		default:
			return E_FILE_OPEN;
		}
	}

	(*F)->name	= f;
	(*F)->buf	= (char *) calloc(_vos.file_buf_size, sizeof(char));
	if (! (*F)->buf) {
		s = E_MEM;
		file_close(F);
	}

	return s;
}

/**
 * @desc:
 *	at first read, File size is zero.
 * @return:
 *	< 0			: success
 *	< E_FILE_END		: end of file
 *	< E_FILE_NOT_OPEN	: fail, file is not open
 *	< E_FILE_READ		: fail, error at reading file
 */
int file_read(struct File *F)
{
	if (! F)
		return E_FILE_NOT_OPEN;

	if (F->size) {
		F->buf = memset(F->buf, FILE_DEF_SET, F->size);
		F->pos	+= F->size;
		F->idx	= 0;
	}

	F->size = read(F->d, F->buf, _vos.file_buf_size);
	if (F->size <= 0) {
		if (F->size == 0)
			return E_FILE_END;

		str_raw_copy(F->name, &_vos.e_sparm0);
		return E_FILE_READ;
	}
	return 0;
}

/**
 * @desc:
 *	write contents of File buffer.
 * @return:
 *	< 0			: success.
 *	< E_FILE_NOT_OPEN	: fail, file is not open.
 *	< E_FILE_WRITE		: fail, error at writing to file.
 */
int file_write(struct File *F)
{
	int s = 0;

	if (! F)
		return E_FILE_NOT_OPEN;
	if (! F->idx)
		return 0;

	s = write(F->d, F->buf, F->idx);
	if (s < 0) {
		str_raw_copy(F->name, &_vos.e_sparm0);
		return E_FILE_WRITE;
	}

	F->buf = memset(F->buf, FILE_DEF_SET, F->idx);
	F->pos += F->idx;
	F->idx = 0;

	return 0;
}

/**
 * @desc:
 *	copy File buffer, from current File index, into String 'str' until
 *	character 'c' is found in File buffer.
 * @return:
 *	< 0	: success
 *	< !0	: fail
 */
int file_fetch_until(struct File *F, struct String *str, int c)
{
	int s = 0;

	while (F->idx < F->size && FCURC(F) != c) {
		s = str_append_c(str, FCURC(F));
		if (s)
			return s;
		F->idx++;
	}
	if (F->idx >= F->size) {
		s = file_read(F);
		if (s == 0)
			s = file_fetch_until(F, str, c);
	}
	return s;
}

/**
 * @desc:
 *	move forward File index until character 'c' is found in buffer.
 * @return:
 *	< 0	: success
 *	< !0	: fail
 */
int file_skip_until(struct File *F, int c)
{
	int s = 0;

	while (F->idx < F->size && FCURC(F) != c)
		F->idx++;
	if (F->idx >= F->size) {
		s = file_read(F);
		if (s == 0)
			s = file_skip_until(F, c);
	}
	return s;
}

/**
 * @desc:
 *	skip white-space character in File buffer.
 * @return:
 *	< 0	: success
 *	< !0	: fail
 */
int file_skip_space(struct File *F)
{
	int s = 0;

	while (F->idx < F->size && isspace(FCURC(F)))
		F->idx++;
	if (F->idx >= F->size) {
		s = file_read(F);
		if (s == 0)
			s = file_skip_space(F);
	}
	return s;
}

void file_close(struct File **F)
{
	if ((*F)) {
		if ((*F)->buf)
			free((*F)->buf);
		if ((*F)->d > 0)
			close((*F)->d);
		free((*F));
		(*F) = 0;
	}
}

/**
 * @desc:
 *	get size of 'file'.
 * @return:
 *	< 0		: success
 *	< E_FILE_OPEN	: fail, cannot open file
 *	< E_FILE_SEEK	: fail, cannot seek file
 */
int file_raw_get_size(const char *file, unsigned long *fsize)
{
	int fd = 0;

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		str_raw_copy(file, &_vos.e_sparm0);
		return E_FILE_OPEN;
	}

	*fsize = lseek(fd, 0, SEEK_END);
	if (*fsize < 0) {
		str_raw_copy(file, &_vos.e_sparm0);
		return E_FILE_SEEK;
	}

	close(fd);

	return 0;
}

/**
 * @desc:
 *	check if 'file' is exist.
 * @return:
 *	< 1	: if 'file' is exist
 *	< 0	: if 'file' does not exist
 *	< -1	: error at opening file
 */
int file_raw_is_exist(const char *file)
{
	int s	= 0;
	int fd	= 0;

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		if (errno != ENOENT) {
			perror(file);
			s = -1;
		}
	} else {
		close(fd);
		s = 1;
	}

	return s;
}
