#ifndef _VOS_TYPE_MERGE_NODE_H
#define	_VOS_TYPE_MERGE_NODE_H	1

#include "type/vos_TFile.h"
#include "type/vos_TRecord.h"

struct MNode {
	int		level;
	struct Record	*row;
	struct File	*file;
	struct MNode	*left;
	struct MNode	*right;
};

#endif
