#ifndef _VOS_STMT_H
#define	_VOS_STMT_H	1

#include "op/vos_StmtSet.h"
#include "op/vos_StmtLoad.h"
#include "vos_StmtSort.h"
#include "vos_StmtCreate.h"
#include "vos_StmtJoin.h"

extern const char *_stmt_type[N_STMT_TYPE];

#define	stmt_get_type_idx(T)	get_token_idx(_stmt_type, N_STMT_TYPE, T)

void stmt_add(struct Stmt **stmt, struct Stmt *new_stmt);
struct Stmt * stmt_find_by_name(struct Stmt *stmt, const char *name);
int stmt_update_meta(struct Stmt *stmt, struct StmtMeta *smeta);
void stmt_print(struct Stmt *stmt);
void stmt_destroy(struct Stmt **stmt);

#endif
