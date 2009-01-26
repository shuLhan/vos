#ifndef _VOS_ERRNO_H
#define	_VOS_ERRNO_H	1

#include <errno.h>

enum vos_errno {
	E_VOS_PARAM		= 1,
/* parser module error number */
	E_PARSER_UNX_CHAR,
	E_PARSER_UNX_TOKEN,
	E_PARSER_UNK_TOKEN,
	E_PARSER_UNK_FIELDNAME,
	E_PARSER_INV_FIELDNAME,
	E_PARSER_INV_POS,
	E_PARSER_INV_VALUE,
	E_PARSER_INV_STMT,
	E_PARSER_AMB_FIELDNAME,	/* 10 */
/* error number from system */
	E_FILE_OPEN,
	E_FILE_EXIST,
	E_FILE_NOT_EXIST,
	E_FILE_NOT_OPEN,
	E_FILE_SEEK,		/* 15 */
	E_FILE_READ,
	E_FILE_WRITE,
	E_FILE_END,
	E_STRTONUM,
	E_MEM,
	N_VOS_ERRNO
};
extern const char *vos_errmsg[N_VOS_ERRNO];

#define	vos_error0(N)		fprintf(stderr,vos_errmsg[N])
#define	vos_error1(N,V)		fprintf(stderr,vos_errmsg[N],V)
#define	vos_error2(N,V,W)	fprintf(stderr,vos_errmsg[N],V,W)

#endif
