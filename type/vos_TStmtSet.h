#ifndef _VOS_TYPE_STMTSET_H
#define	_VOS_TYPE_STMTSET_H	1

enum _set_idx {
	SET_FILE_BUFFER_SIZE	= 0,
	SET_PROC_CMP_CASE_SENSITIVE,
	SET_PROC_CMP_CASE_NOTSENSITIVE,
	SET_PROC_MAX,
	SET_PROC_MAX_ROW,
	N_SET
};

struct StmtSet {
	unsigned short	var_idx;
	char		*value;
};

#endif
