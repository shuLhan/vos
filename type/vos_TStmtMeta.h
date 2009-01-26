#ifndef _VOS_TYPE_STMTMETA_H
#define	_VOS_TYPE_STMTMETA_H	1

#include "type/vos_TField.h"

struct StmtMeta {
	int		flag;
	char		*filename;
	char		*alias;
	struct Field	*fields;
	struct StmtMeta	*next;
};

#endif
