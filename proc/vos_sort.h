#ifndef _VOS_SORT_H
#define	_VOS_SORT_H	1

#include <pthread.h>
#include "type/vos_TProcSort.h"
#include "proc/vos_sort_merge.h"

int vos_process_sort(struct Stmt *sort);

#endif
