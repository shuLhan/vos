#ifndef _VOS_STMTMETA_H
#define	_VOS_STMTMETA_H	1

#include "type/vos_TStmtMeta.h"
#include "op/vos_Field.h"

struct StmtMeta * stmtmeta_search_filename(struct StmtMeta *meta,
						const char *str);
int stmtmeta_search_field(struct StmtMeta *meta, const char *fldname,
				struct Field **fld_r);
int stmtmeta_soft_copy(struct StmtMeta *meta, struct StmtMeta **cp);
void stmtmeta_add(struct StmtMeta **meta, struct StmtMeta *n);
void _stmtmeta_destroy(struct StmtMeta **meta, const int soft);

#define	stmtmeta_destroy(M)		_stmtmeta_destroy(M,0)
#define	stmtmeta_soft_destroy(M)	_stmtmeta_destroy(M,1)

#endif
