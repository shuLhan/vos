#ifndef _VOS_MERGE_H
#define	_VOS_MERGE_H	1

#include "type/vos_TMNode.h"
#include "op/vos_StmtSort.h"
#include "op/vos_LL.h"
#include "op/vos_Record.h"

int vos_sort_merge(struct Stmt *sort, struct LL *lsfile,
			struct Record **_all_rows, unsigned long all_n_row);

#endif
