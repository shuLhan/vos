#ifndef _VOS_PROCESS_CREATE_H
#define	_VOS_PROCESS_CREATE_H	1

#include <pthread.h>
#include "type/vos_TProcCreate.h"
#include "op/vos_Bucket.h"

#define	THREAD_TIME_WAIT	0.1

int vos_process_create(struct Stmt *create);

#endif
