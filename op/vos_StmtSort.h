#ifndef _VOS_STMTSORT_H
#define	_VOS_STMTSORT_H	1

#include "type/vos_TStmt.h"
#include "op/vos_File.h"
#include "op/vos_StmtMeta.h"

#define	sort_get_idx(T)	get_token_idx(_fflag_sort, N_FFLAG_SORT, T)

int stmtsort_create(struct Stmt **sort);
int stmtsort_init_output(struct Stmt *sort);
void stmtsort_print(struct Stmt *sort);
void stmtsort_destroy(struct Stmt **sort);

#endif
