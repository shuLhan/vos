#include "op/vos_StmtMeta.h"

/**
 * @return:
 *	< !0	: success
 *	< 0	: fail, filename not found
 */
struct StmtMeta * stmtmeta_search_filename(struct StmtMeta *meta,
						const char *str)
{
	int s;

	while (meta) {
		s = strcasecmp(meta->filename, str);
		if (s == 0)
			return meta;
		if (meta->alias) {
			s = strcasecmp(meta->alias, str);
			if (s == 0)
				return meta;
		}
		meta = meta->next;
	}

	return 0;
}

/**
 * @return:
 *	< 0 : success, fieldname found
 *	< E_PARSER_UNK_FIELDNAME : fail, fieldname not found
 *	< E_PARSER_AMB_FIELDNAME : fail, fieldname already declared
 */
int stmtmeta_search_field(struct StmtMeta *meta, const char *fldname,
				struct Field **fld_r)
{
	int		s;
	struct Field	*fld	= 0;

	while (meta) {
		fld = meta->fields;
		while (fld) {
			s = strcasecmp(fld->name, fldname);
			if (s == 0) {
				if ((*fld_r))
					return E_PARSER_AMB_FIELDNAME;
				else
					(*fld_r) = fld;
				break;
			}
			fld = fld->next;
		}
		meta = meta->next;
	}

	if (! (*fld_r))
		return E_PARSER_UNK_FIELDNAME;
	return 0;
}

int stmtmeta_soft_copy(struct StmtMeta *meta, struct StmtMeta **cp)
{
	int		s	= 0;
	struct Field	*fld	= 0;
	struct Field	*fcp	= 0;

	(*cp) = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*cp))
		return E_MEM;

	str_raw_copy(meta->filename, &(*cp)->filename);
	str_raw_copy(meta->alias, &(*cp)->alias);

	fld = meta->fields;
	while (fld) {
		s = field_soft_copy(fld, &fcp);
		if (s)
			goto err;

		field_add(&(*cp)->fields, fcp);
		fld = fld->next;
	}
err:
	if (s)
		stmtmeta_soft_destroy(cp);
	return s;
}

void stmtmeta_add(struct StmtMeta **meta, struct StmtMeta *n)
{
	if (! (*meta))
		(*meta) = n;
	else {
		struct StmtMeta *p = (*meta);
		while (p->next)
			p = p->next;
		p->next = n;
	}
}

void _stmtmeta_destroy(struct StmtMeta **meta, const int soft)
{
	if (! (*meta))
		return;

	if ((*meta)->filename)
		free((*meta)->filename);
	if ((*meta)->alias)
		free((*meta)->alias);
	if ((*meta)->fields)
		_field_destroy(&(*meta)->fields, soft);
	_stmtmeta_destroy(&(*meta)->next, soft);
	free((*meta));
	(*meta) = 0;
}
