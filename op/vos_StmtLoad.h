#ifndef _VOS_STMTLOAD_H
#define	_VOS_STMTLOAD_H	1

#include "type/vos_TStmt.h"
#include "op/vos_StmtMeta.h"

int stmtload_create(struct Stmt **load);
void stmtload_destroy(struct Stmt **load);
void stmtload_print(struct Stmt *load);

#endif
