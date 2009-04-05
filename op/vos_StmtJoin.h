#ifndef _VOS_STMTJOIN_H
#define	_VOS_STMTJOIN_H	1

#include "type/vos_TStmt.h"
#include "op/vos_StmtMeta.h"
#include "op/vos_File.h"

extern const char *_join_flag[N_JOIN_FLAG];
extern const char *_join_sort_flag[N_JOIN_SORT_FLAG];

#define	join_get_flag(T)	get_token_idx(_join_flag, N_JOIN_FLAG, T)
#define	join_get_sort_flag(T)	get_token_idx(_join_sort_flag, N_JOIN_SORT_FLAG, T)

int stmtjoin_create(struct Stmt **join);
int stmtjoin_init_output(struct Stmt *join);
void stmtjoin_print(struct Stmt *join);
void stmtjoin_destroy(struct Stmt **join);

#endif
