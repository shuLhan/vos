#ifndef _VOS_TYPE_RECORD_H
#define	_VOS_TYPE_RECORD_H	1

#include "type/vos_TString.h"

struct Record {
	int		idx;
	int		flag;
	struct String	*v;
	struct Record	*fld_next;
	struct Record	*fld_last;
	struct Record	*row_next;
	struct Record	*row_last;
};

#endif
