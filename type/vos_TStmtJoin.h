#ifndef _VOS_TYPE_STMTJOIN_H
#define	_VOS_TYPE_STMTJOIN_H	1

enum _stmt_join_flag {
	JOIN_NORMAL	= 0,
	JOIN_OUTER	= 1,
	JOIN_ANTI	= 2,
	N_JOIN_FLAG
};

enum _stmt_join_sort_flag {
	JOIN_UNSORTED	= 4,
	JOIN_SORTED	= 8,
	N_JOIN_SORT_FLAG
};

#endif
