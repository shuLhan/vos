#ifndef	_VOS_TYPE_PROCESS_SORT_H
#define	_VOS_TYPE_PROCESS_SORT_H	1

#include "type/vos_TLL.h"
#include "type/vos_TStmt.h"

struct ProcSort {
	pthread_t		tid;
	int			retval;
	unsigned long		pos_start;
	unsigned long		pos_end;
	unsigned long		n_row;
	struct Record		*rows;
	const struct Stmt	*sort;
	struct LL		*lsout;
};

#endif
