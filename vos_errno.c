#include "vos_errno.h"

const char *vos_errmsg[N_VOS_ERRNO] = {
	"\0",
	"Usage: vos <vos script>\n\0",

	"line %d: unexpected character '%c'.\n\0",
	"line %d: unexpected token '%s'.\n\0",
	"line %d: unknown token '%s'.\n\0",
	"line %d: unknown field name '%s'.\n\0",		/* 5 */
	"line %d: invalid field name '%s'.\n\0",
	"line %d: invalid position value.\n\0",
	"line %d: invalid format or value '%s'.\n\0",
	"line %d: invalid statement '%s'.\n\0",
	"line %d: duplicate/ambigous field name '%s'.\n\0",	/* 10 */

	"error: cannot open file '%s'.\n\0",
	"error: file '%s' exist.\n\0",
	"error: file '%s' is not exist.\n\0",
	"error: file not open\n\0",
	"error: cannot seek file '%s'.\n\0",			/* 15 */
	"error: cannot read file.\n\0",
	"error: errot at writing to file.\n\0",
	"error: end of file.\n\0",
	"error: converting string to number.\n\0",
	"error: out of memory.\n\0"				/* 20 */
};
