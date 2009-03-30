#ifndef _VOS_T_H
#define	_VOS_T_H

#include <pthread.h>
#include "vos_TLL.h"

/**
 * @desc:
 *	- DBG_SCRIPT	: don't process the script
 *				use to check if script is valid or not
 *	- DBG_PARSER	: debug parsing process and execute statement in
 *				the script
 *	- DBG_SORT	: debug sort process
 *	- DBG_CREATE	: debug create process
 *	- DBG_JOIN	: debug join process
 *	- DBG_ALL	: debug all process
 */
enum _vos_debug {
	DBG_SCRIPT	= 1,
	DBG_PARSER	= 2,
	DBG_SORT	= 4,
	DBG_CREATE	= 8,
	DBG_JOIN	= 16,
	DBG_ALL		= 30
};

enum _vos_cmp_case {
	CMP_CASE_SENSITIVE	= 0,
	CMP_CASE_NOTSENSITIVE
};

struct Vos {
	int		debug;		/* debug parameter */
	int		e_nparm0;	/* first error parameter for number */
	int		e_nparm1;	/* second error parameter for number */
	int		proc_cmp_case;	/* use compare case sensitive ? */
	int		proc_max;	/* maximum process for sort */
	unsigned long	file_buf_size;	/* size of buffer for file */
	unsigned long	proc_max_row;	/* maximum row for each process */
	pthread_mutex_t	proc_tmp_dir_lock;
	char		*script;	/* vos script */
	char		*e_sparm0;	/* first error parameter (string) */
	char		*e_sparm1;	/* second error parameter (string) */
	struct LL	*proc_tmp_dir;	/* process temporary directories */
	struct LL	*p_proc_tmp_dir;
};

#endif
