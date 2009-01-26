#ifndef _VOS_TYPE_BUCKET_H
#define	_VOS_TYPE_BUCKET_H	1

#include "type/vos_TRecord.h"

struct Bucket {
	int		stat;
	struct Record	*cnt;
	struct Record	*p;
};

#endif
