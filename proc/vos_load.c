#include "vos_load.h"

/**
 * @desc:
 *	check if file to `load' is exist.
 * @return:
 *	< 0 : success.
 *	< E_FILE_NOT_EXIST : fail.
 */
int vos_process_load(struct Stmt *load)
{
	int fd = 0;

	fd = open(load->in->filename, O_RDONLY);
	if (fd < 0)
		return E_FILE_NOT_EXIST;

	close(fd);
	return 0;
}
