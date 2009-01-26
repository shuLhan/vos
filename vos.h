#ifndef _VOS_H
#define	_VOS_H	1

#include "type/vos_TVos.h"

#define	VOS_DEF_DATE_FORMAT	"YYYY/MM/DD"
#define	VOS_DEF_FILE_BUF_SIZE	8192
#define	VOS_DEF_PROC_CMP_CASE	CMP_CASE_SENSITIVE
#define	VOS_DEF_PROC_MAX	2
#define	VOS_DEF_PROC_MAX_ROW	100000
#define	VOS_SORT_OUT_FORMAT	"sort.XXXXXXXX"
#define	VOS_SORT_TMP_FORMAT	"tmp.sort.XXXXXXXX"
#define	VOS_JOIN_OUT_FORMAT	"join.XXXXXXXX"

struct Vos _vos;

int get_token_idx(const char **ls, const unsigned int n, const char *tok);

#endif
