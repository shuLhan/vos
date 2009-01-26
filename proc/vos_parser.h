#ifndef _VOS_PARSER_H
#define	_VOS_PARSER_H	1

#include "op/vos_LL.h"
#include "op/vos_File.h"
#include "op/vos_Stmt.h"

int vos_parsing(struct Stmt **stmt, const char *script);

#endif
