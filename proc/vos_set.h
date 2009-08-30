#ifndef _VOS_SET_H
#define	_VOS_SET_H	1

#include <unistd.h>
#include "vos.h"
#include "op/vos_LL.h"
#include "op/vos_StmtSet.h"

int set_proc_tmp_dir_value(char *tmp);
int vos_process_set(struct StmtSet *set);

#endif
