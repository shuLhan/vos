#ifndef _VOS_FILE_H
#define	_VOS_FILE_H	1

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "type/vos_TFile.h"
#include "vos.h"
#include "op/vos_String.h"

#define	FILE_DEF_SET	' '

#define	FCURC(F)	F->buf[F->idx]
#define	FCURP(F)	(F->pos + F->idx)

int file_open(struct File **F, const char *f, int flag);
int file_read(struct File *F);
int file_write(struct File *F);
int file_fetch_until(struct File *F, struct String *str, int c);
int file_skip_until(struct File *F, int c);
int file_skip_space(struct File *F);
void file_close(struct File **F);

int file_raw_get_size(const char *file, unsigned long *fsize);
int file_raw_is_exist(const char *file);

#endif
