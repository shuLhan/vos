#ifndef _VOS_TYPE_FILTER_H
#define	_VOS_TYPE_FILTER_H	1

enum _filter_op_idx {
	OP_EQUAL = 0,	/* not case sensitive */
	OP_TRUE_EQUAL,	/* case sensitive */
	OP_NOT_EQUAL,
	OP_NOT_TRUE_EQUAL,
	OP_MORE,
	OP_LESS,
	OP_MORE_EQUAL,
	OP_LESS_EQUAL,
	N_FLTR_OP
};

enum _filter_rule_idx {
	FLTR_REJECT = 0,
	FLTR_ACCEPT = 1,
	N_FLTR_RULE
};

#endif
