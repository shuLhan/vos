#ifndef _VOS_TYPE_PROCESS_CREATE_H
#define	_VOS_TYPE_PROCESS_CREATE_H	1

#include "type/vos_TStmtMeta.h"
#include "type/vos_TBucket.h"
#include "type/vos_TStmt.h"

enum _cproc_status {
	CPROC_START	= 1,
	CPROC_BUCKETS_FULL,
	CPROC_DONE,
	CPROC_END
};

struct ProcCreate {
	pthread_t	tid;
	int		retval;
	int		status;
	struct StmtMeta	*in;
	struct Bucket	*buckets;
};

#endif
