#ifndef _VOS_STMTSET_H
#define	_VOS_STMTSET_H	1

#include <stdio.h>
#include <stdlib.h>
#include "vos_errno.h"
#include "type/vos_TStmt.h"

extern const char *_stmt_set[N_SET];

#define	set_get_var_idx(T) get_token_idx(_stmt_set, N_SET, T)

int stmtset_create(struct Stmt **stmt);
void stmtset_print(struct Stmt *stmt);
void stmtset_destroy(struct Stmt **stmt);

#endif
