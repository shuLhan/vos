#ifndef _VOS_TYPE_STMT_H
#define	_VOS_TYPE_STMT_H	1

#include "type/vos_TStmtSet.h"
#include "type/vos_TStmtMeta.h"

enum _stmt_type_idx {
	STMT_SET = 0,
	STMT_LOAD,
	STMT_CREATE,
	STMT_SORT,
	STMT_JOIN,
	N_STMT_TYPE
};

struct Stmt {
	unsigned short	type;
	struct StmtSet	*set;
	struct StmtMeta	*in;
	struct StmtMeta	*out;
	struct Stmt	*next;
	struct Stmt	*prev;
	struct Stmt	*last;
};

#endif
