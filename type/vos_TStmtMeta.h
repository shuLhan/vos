#ifndef _VOS_TYPE_STMTMETA_H
#define	_VOS_TYPE_STMTMETA_H	1

#include "type/vos_TField.h"

enum _stmtmeta_flag {
	JOIN_NORMAL		= 0,
	JOIN_OUTER		= 1,
	JOIN_ANTI		= 2,
	N_JOIN_FLAG		= 3,

	JOIN_UNSORTED		= 4,
	JOIN_SORTED		= 8,
	N_JOIN_SORT_FLAG	= 9,

	SORT_TMP		= 16
};

struct StmtMeta {
	int		flag;
	char		*filename;
	char		*alias;
	struct Field	*fields;
	struct StmtMeta	*next;
};

#endif
