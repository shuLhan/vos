#ifndef _VOS_STMT_CREATE_H
#define	_VOS_STMT_CREATE_H	1

#include "type/vos_TStmt.h"
#include "op/vos_StmtMeta.h"

int stmtcreate_create(struct Stmt **create);
void stmtcreate_print(struct Stmt *create);
void stmtcreate_destroy(struct Stmt **create);

#endif
