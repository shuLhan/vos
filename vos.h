#ifndef _VOS_H
#define	_VOS_H	1

#include "type/vos_TVos.h"

#define	VOS_DEF_DEBUG_VALUE	0
#define	VOS_DEF_DATE_FORMAT	"YYYY/MM/DD"
#define	VOS_DEF_FILE_BUF_SIZE	8192
#define	VOS_DEF_PROC_CMP_CASE	CMP_CASE_SENSITIVE
#define	VOS_DEF_PROC_MAX	2
#define	VOS_DEF_PROC_MAX_ROW	100000
#define	VOS_DEF_PROC_TMP_DIR	"/tmp/"

#define	VOS_ENV_DEBUG		"VOS_DEBUG"
#define	VOS_ENV_TMP_DIR		"VOS_TMP_DIR"
#define	VOS_ENV_FILE_BUF_SIZE	"VOS_FILE_BUFFER_SIZE"
#define	VOS_ENV_PROC_CMP_CASE	"VOS_COMPARE_CASE"
#define	VOS_ENV_PROC_MAX	"VOS_PROCESS_MAX"
#define	VOS_ENV_PROC_MAX_ROW	"VOS_PROCESS_MAX_ROW"

#define	VOS_SORT_OUT_FORMAT	"sort.XXXXXXXX"
#define	VOS_SORT_TMP_FORMAT	"tmp.sort.XXXXXXXX"
#define	VOS_JOIN_OUT_FORMAT	"join.XXXXXXXX"

struct Vos _vos;

int get_token_idx(const char **ls, const unsigned int n, const char *tok);
char *get_tmp_dir(const int lock);
void sys_getenv_num(unsigned long *env_value, const char *env_name,
			unsigned long def_value);

#endif
