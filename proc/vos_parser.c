#include "proc/vos_parser.h"

enum parsing_set_todo {
	PSET_DONE	= 0,
	PSET_START,
	PSET_VAR,
	PSET_VALUE,
	PSET_END
};

enum parsing_load_todo {
	PLOAD_DONE	= 0,
	PLOAD_START,
	PLOAD_NAME,
	PLOAD_FIELD_BEGIN,
	PLOAD_FIELD_END,
	PLOAD_AS,
	PLOAD_AS_NAME,
	PLOAD_END
};

enum parsing_field_todo {
	PFIELD_DONE	= 0,
	PFIELD_START,
	PFIELD_LEFTQ,
	PFIELD_PARENT_NAME,
	PFIELD_PARENT_SEP,
	PFIELD_NAME,
	PFIELD_RIGHTQ,
	PFIELD_STARTP,
	PFIELD_ENDP,
	PFIELD_TYPE,
	PFIELD_DATE_FORMAT,
	PFIELD_SEP,
	PFIELD_END
};

enum parsing_sort_todo {
	PSORT_DONE	= 0,
	PSORT_START,
	PSORT_INPUT,
	PSORT_BY,
	PSORT_FIELD,
	PSORT_ORDER,
	PSORT_FIELD_SEP,
	PSORT_INTO,
	PSORT_AS,
	PSORT_AS_VALUE,
	PSORT_END
};

enum parsing_create_todo {
	PCREATE_DONE	= 0,
	PCREATE_START,
	PCREATE_FILENAME,
	PCREATE_FROM,
	PCREATE_FROM_VALUE,
	PCREATE_FROM_SEP,
	PCREATE_FIELD_BEGIN,
	PCREATE_FIELD_END,
	PCREATE_FILTER,
	PCREATE_FILTER_BEGIN,
	PCREATE_FILTER_END,
	PCREATE_AS,
	PCREATE_AS_NAME,
	PCREATE_END
};

enum parsing_filter_todo {
	PFLTR_DONE	= 0,
	PFLTR_START,
	PFLTR_RULE,
	PFLTR_PARENT_NAME,
	PFLTR_PARENT_SEP,
	PFLTR_FIELD_NAME,
	PFLTR_OP,
	PFLTR_VALUE,
	PFLTR_END
};

enum parsing_join_todo {
	PJOIN_DONE	= 0,
	PJOIN_START,
	PJOIN_INPUT,
	PJOIN_INPUT_JOIN_FLAG,
	PJOIN_INPUT_SORT_FLAG,
	PJOIN_INPUT_SEP,
	PJOIN_INTO,
	PJOIN_FIELD_BEGIN,
	PJOIN_FIELD_START,
	PJOIN_FIELD_L_PARENT_NAME,
	PJOIN_FIELD_R_PARENT_NAME,
	PJOIN_FIELD_L_PARENT_SEP,
	PJOIN_FIELD_R_PARENT_SEP,
	PJOIN_FIELD_L_NAME,
	PJOIN_FIELD_R_NAME,
	PJOIN_FIELD_OP,
	PJOIN_FIELD_SEP,
	PJOIN_FIELD_END,
	PJOIN_AS,
	PJOIN_AS_NAME,
	PJOIN_END
};

static int stmt_get_output_from(struct StmtMeta **smeta, struct Stmt *stmt,
				const char *name)
{
	int		s;
	struct Stmt	*p	= 0;

	p = stmt_find_by_name(stmt, name);
	if (! p)
		return E_PARSER_INV_VALUE;

	switch (p->type) {
	case STMT_LOAD:
		s = stmtmeta_soft_copy(p->in, smeta);
		break;
	case STMT_SORT:
		s = stmtmeta_soft_copy(p->out, smeta);
		break;
	case STMT_CREATE:
		s = stmtmeta_soft_copy(p->out, smeta);
		break;
	case STMT_JOIN:
		s = stmtmeta_soft_copy(p->out, smeta);
		break;
	default:
		s = E_PARSER_INV_STMT;
	}
	return s;
}

static int parsing_SET(struct Stmt **stmt, struct LL **ptok)
{
	int s = PSET_START;

	while ((*ptok) && s != PSET_DONE) {
		switch (s) {
		case PSET_START:
			s = stmtset_create(stmt);
			if (s)
				return s;
			s = PSET_VAR;
			break;
		case PSET_VAR:
			s = set_get_var_idx((*ptok)->str);
			if (s < 0) {
				s = E_PARSER_UNK_TOKEN;
				goto err;
			}

			(*stmt)->set->var_idx = s;

			if (s == SET_PROC_CMP_CASE_SENSITIVE
			||  s == SET_PROC_CMP_CASE_NOTSENSITIVE)
				s = PSET_END;
			else
				s = PSET_VALUE;
			break;
		case PSET_VALUE:
			(*stmt)->set->value	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PSET_END;
			break;
		case PSET_END:
			if ((*ptok)->str[0] != ';') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = PSET_DONE;
			break;
		}
		(*ptok)	= (*ptok)->next;
	}
	if (s != PSET_DONE) {
		s = E_PARSER_INV_STMT;
err:
		stmtset_destroy(stmt);
	}
	return s;
}

/**
 * @desc:
 *	if mref not null, that mean we parsing field for create statement.
 */
static int parsing_FIELD(struct StmtMeta *mref, struct StmtMeta *mout,
				struct LL **ptok)
{
	int		idx		= 1;
	int		s		= 0;
	int		todo_next	= 0;
	struct LL	*prevtok	= 0;
	struct Field	*f		= 0;
	struct Field	*fsearch	= 0;
	struct StmtMeta	*pmeta		= 0;
	struct String	*fname		= 0;

	if (mref) {
		s = str_create(&fname);
		if (s)
			goto err;
	}

	s = PFIELD_START;
	while ((*ptok) && s != PFIELD_DONE) {
		switch (s) {
		case PFIELD_START:
			f = (struct Field *) calloc(1, sizeof(struct Field));
			if (! f) {
				s = E_MEM;
				goto err;
			}
			s = PFIELD_LEFTQ;
			break;

		case PFIELD_LEFTQ:
			switch ((*ptok)->str[0]) {
			case '\'':
				f->left_q	= (*ptok)->str[1];
				s		= PFIELD_SEP;
				todo_next	= PFIELD_PARENT_NAME;
				break;
			case ':':
				s = PFIELD_PARENT_NAME;
				break;
			default:
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			break;

		case PFIELD_SEP:
			switch ((*ptok)->str[0]) {
			case ':':
				s = todo_next;
				break;
			case ',':
				if (todo_next == PFIELD_TYPE) {
					s = PFIELD_END;
					continue;
				}
				/* if todo_next != PFIELD_TYPE that's mean
				 * there is an error in statement so skip it
				 * to 'default' */
			case ')':
				if (todo_next == PFIELD_TYPE) {
					s = PFIELD_END;
					continue;
				}
			default:
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			break;

		case PFIELD_PARENT_NAME:
			if (mref) {
				prevtok	= (*ptok);
				s	= PFIELD_PARENT_SEP;
				break;
			}
			s = PFIELD_NAME;
			continue;

		case PFIELD_PARENT_SEP:
			if ((*ptok)->str[0] == '.') {
				pmeta = stmtmeta_search_filename(mref,
								prevtok->str);
				if (! pmeta) {
					(*ptok) = prevtok;
					s = E_PARSER_INV_VALUE;
					goto err;
				}
				s = PFIELD_NAME;
				break;
			}
			/* continue to below */
			pmeta = mref;
			(*ptok) = prevtok;

		case PFIELD_NAME:
			if (mref) {
				/* search field */
				s = stmtmeta_search_field(pmeta, (*ptok)->str,
								&fsearch);
				if (s == E_PARSER_UNK_FIELDNAME)
					goto err;

				/* create field name "x.field" */
				if (pmeta->alias) {
					str_append(fname, pmeta->alias);
					str_append(fname, ".");
				}
				str_append(fname, fsearch->name);
				s = str_raw_copy(fname->buf, &f->name);
				if (s)
					goto err;
				str_prune(fname);
			} else {
				/* check if field already declared */
				s = stmtmeta_search_field(mout, (*ptok)->str,
								&fsearch);
				if (s != E_PARSER_UNK_FIELDNAME) {
					s = E_PARSER_AMB_FIELDNAME;
					goto err;
				}

				f->name		= (*ptok)->str;
				(*ptok)->str	= 0;
			}
			s		= PFIELD_SEP;
			todo_next	= PFIELD_RIGHTQ;
			break;

		case PFIELD_RIGHTQ:
			switch ((*ptok)->str[0]) {
			case '\'':
				f->right_q	= (*ptok)->str[1];
				s		= PFIELD_SEP;
				todo_next	= PFIELD_STARTP;
				break;
			case ':':
				s = PFIELD_STARTP;
				break;
			default:
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			break;

		case PFIELD_STARTP:
			if ((*ptok)->str[0] == ':')
				s = PFIELD_ENDP;
			else if (isdigit((*ptok)->str[0])) {
				f->start_p	= strtol((*ptok)->str, 0, 0);
				s		= PFIELD_SEP;
				todo_next	= PFIELD_ENDP;
			} else {
				s = E_PARSER_INV_VALUE;
				goto err;
			}
			break;

		case PFIELD_ENDP:
			if (isdigit((*ptok)->str[0])) {
				f->end_p = strtol((*ptok)->str, 0, 0);
				if (f->end_p < f->start_p) {
					s = E_PARSER_INV_POS;
					goto err;
				}
			} else if ((*ptok)->str[0] == '\'') {
				f->sep = (*ptok)->str[1];
			} else {
				s		= PFIELD_SEP;
				todo_next	= PFIELD_TYPE;
				continue;
			}

			s		= PFIELD_SEP;
			todo_next	= PFIELD_TYPE;
			break;

		case PFIELD_TYPE:
			s = field_get_type_idx((*ptok)->str);
			if (s < 0) {
				s = E_PARSER_INV_VALUE;
				goto err;
			}
			f->type = s;
			if (s == FT_DATETIME)
				s = PFIELD_DATE_FORMAT;
			else
				s = PFIELD_END;
			break;

		case PFIELD_DATE_FORMAT:
			if (isalpha((*ptok)->str[0])) {
				f->date_format	= (*ptok)->str;
				(*ptok)->str	= 0;
				s		= PFIELD_END;
				break;
			} 

		case PFIELD_END:
			if (mref) {
				fsearch->idx	= idx++;
				fsearch->flag	|= FFLAG_CREATE;
				pmeta		= 0;
			}
			fsearch	= 0;

			switch ((*ptok)->str[0]) {
			case ',':
				s = PFIELD_START;
				field_add(&mout->fields, f);
				break;
			case ')':
				s = PFIELD_DONE;
				field_add(&mout->fields, f);
				break;
			default:
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			continue;
		}
		(*ptok)	= (*ptok)->next;
	}

	if (s != PFIELD_DONE) {
		s = E_PARSER_INV_STMT;
err:
		if (f)
			field_destroy(&f);
		if (mout->fields)
			field_destroy(&mout->fields);
	}
	if (fname)
		str_destroy(&fname);
	return s;
}

static int parsing_LOAD(struct Stmt **load, struct LL **ptok)
{
	int s = PLOAD_START;

	while ((*ptok) && s != PLOAD_DONE) {
		switch (s) {
		case PLOAD_START:
			s = stmtload_create(load);
			if (s)
				return s;
			s = PLOAD_NAME;
			break;

		case PLOAD_NAME:
			(*load)->in->filename	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PLOAD_FIELD_BEGIN;
			break;

		case PLOAD_FIELD_BEGIN:
			if ((*ptok)->str[0] != '(') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = parsing_FIELD(0, (*load)->in, ptok);
			if (s)
				goto err;

		case PLOAD_FIELD_END:
			if ((*ptok)->str[0] != ')') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = PLOAD_AS;
			break;

		case PLOAD_AS:
			s = strcasecmp((*ptok)->str, "AS");
			if (s == 0) {
				s = PLOAD_AS_NAME;
				break;
			}
		case PLOAD_END:
			if ((*ptok)->str[0] != ';') {
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			s = PLOAD_DONE;
			break;

		case PLOAD_AS_NAME:
			(*load)->in->alias	= (*ptok)->str;
			(*ptok)->str		= 0;
			s		= PLOAD_END;
			break;
		}
		(*ptok)	= (*ptok)->next;
	}

	if (s != PLOAD_DONE) {
		s = E_PARSER_INV_STMT;
err:
		stmtload_destroy(load);
	}
	return s;
}

static int parsing_SORT(struct Stmt *stmt, struct Stmt **sort,
			struct LL **ptok)
{
	int			idx		= 1;
	int			s		= PSORT_START;
	struct StmtMeta		*smeta		= 0;
	struct Field		*fsearch	= 0;

	while ((*ptok) && s != PSORT_DONE) {
		switch (s) {
		case PSORT_START:
			s = stmtsort_create(sort);
			if (s)
				goto err;
			s = PSORT_INPUT;
			break;

		case PSORT_INPUT:
			s = stmt_get_output_from(&smeta, stmt, (*ptok)->str);
			if (s)
				goto err;

			stmtmeta_add(&(*sort)->in, smeta);
			s = PSORT_BY;
			break;

		case PSORT_BY:
			s = strcasecmp((*ptok)->str, "BY");
			if (s) {
				s = E_PARSER_INV_STMT;
				goto err;
			}
			s = PSORT_FIELD;
			break;

		case PSORT_FIELD:
			s = stmtmeta_search_field(smeta, (*ptok)->str,
							&fsearch);
			if (s == E_PARSER_UNK_FIELDNAME)
				goto err;

			s = PSORT_ORDER;
			break;

		case PSORT_ORDER:
			s = sort_get_idx((*ptok)->str);
			if (s > 0) {
				fsearch->flag	= s;
				s		= PSORT_FIELD_SEP;
				break;
			} else
				fsearch->flag = FFLAG_SORT_ASC;
			/* go below */

		case PSORT_FIELD_SEP:
			fsearch->idx	= idx++;
			fsearch		= 0;
			if ((*ptok)->str[0] == ',') {
				s = PSORT_FIELD;
				break;
			}

			s = strcasecmp((*ptok)->str, "INTO");
			if (s == 0) {
				s = PSORT_INTO;
				break;
			}
			/* go below */

		case PSORT_AS:
			s = strcasecmp((*ptok)->str, "AS");
			if (s == 0) {
				s = PSORT_AS_VALUE;
				break;
			}
			/* go below */

		case PSORT_END:
			if ((*ptok)->str[0] == ';') {
				s = PSORT_DONE;
			} else {
				s = E_PARSER_UNK_TOKEN;
				goto err;
			}
			break;

		case PSORT_INTO:
			(*sort)->out->filename	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PSORT_AS;
			break;

		case PSORT_AS_VALUE:
			(*sort)->out->alias	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PSORT_END;
			break;
		}
		(*ptok)	= (*ptok)->next;
	}

	if (s == PSORT_DONE)
		s = stmtsort_init_output((*sort));
	else
		s = E_PARSER_INV_STMT;
err:
	if (s) {
		stmtsort_destroy(sort);
	}
	return s;
}

/**
 * @return:
 *	< 0	: success
 *	< !0	: fail
 */
static int parsing_FILTER(struct StmtMeta *mref, struct LL **ptok)
{
	int		s		= PFLTR_START;
	int		fltr_rule	= 0;
	int		fltr_idx	= 1;
	struct LL	*prevtok	= 0;
	struct StmtMeta	*pmeta		= 0;
	struct Field	*fsearch	= 0;

	while ((*ptok) && s != PFLTR_DONE) {
		switch (s) {
		case PFLTR_START: /* needed, so we can check next ptok */
			s = PFLTR_RULE;
			break;

		case PFLTR_RULE:
			fltr_rule = rule_get_idx((*ptok)->str);
			if (fltr_rule < 0) {
				s = E_PARSER_INV_STMT;
				goto err;
			}
			s = PFLTR_PARENT_NAME;
			break;

		case PFLTR_PARENT_NAME:
			prevtok = (*ptok);
			s	= PFLTR_PARENT_SEP;
			break;

		case PFLTR_PARENT_SEP:
			if ((*ptok)->str[0] == '.') {
				pmeta = stmtmeta_search_filename(mref,
								prevtok->str);
				if (! pmeta) {
					s = E_PARSER_INV_VALUE;
					goto err;
				}
				s = PFLTR_FIELD_NAME;
				break;
			} else {
				s = stmtmeta_search_field(mref, prevtok->str,
								&fsearch);
				if (s == E_PARSER_UNK_FIELDNAME)
					goto err;

				s = PFLTR_OP;
				continue;
			}

		case PFLTR_FIELD_NAME:
			s = stmtmeta_search_field(pmeta, (*ptok)->str,
							&fsearch);
			if (s == E_PARSER_UNK_FIELDNAME)
				goto err;
			s = PFLTR_OP;
			break;

		case PFLTR_OP:
			s = op_get_idx((*ptok)->str);
			if (s < 0) {
				s = E_PARSER_UNK_TOKEN;
				goto err;
			}
			fsearch->fop	= _filter_f[s];
			s		= PFLTR_VALUE;
			break;

		case PFLTR_VALUE:
			fsearch->fltr_v	= (*ptok)->str;
			(*ptok)->str	= 0;
			s		= PFLTR_END;
			break;

		case PFLTR_END:
			fsearch->flag		|= FFLAG_FILTER;
			fsearch->fltr_idx	= fltr_idx++;
			fsearch->fltr_rule	= fltr_rule;
			fsearch			= 0;
			pmeta			= 0;

			if ((*ptok)->str[0] == ')') {
				s = PFLTR_DONE;
				continue;
			}
			if ((*ptok)->str[0] == ',') {
				s = PFLTR_START;
				continue;
			}

			s = E_PARSER_UNX_TOKEN;
			goto err;
		}
		(*ptok)	= (*ptok)->next;
	}

	if (s != PFLTR_DONE)
		s = E_PARSER_INV_STMT;
err:
	return s;
}

static int parsing_CREATE(struct Stmt *stmt, struct Stmt **create,
				struct LL **ptok)
{
	int		s	= PCREATE_START;
	struct StmtMeta	*smeta	= 0;

	while ((*ptok) && s != PCREATE_DONE) {
		switch (s) {
		case PCREATE_START:
			s = stmtcreate_create(create);
			if (s)
				return s;
			s = PCREATE_FILENAME;
			break;

		case PCREATE_FILENAME:
			(*create)->out->filename	= (*ptok)->str;
			(*ptok)->str			= 0;
			s				= PCREATE_FROM;
			break;

		case PCREATE_FROM:
			s = strcasecmp((*ptok)->str, "FROM");
			if (s) {
				s = E_PARSER_INV_STMT;
				goto err;
			}
			s = PCREATE_FROM_VALUE;
			break;

		case PCREATE_FROM_VALUE:
			s = stmt_get_output_from(&smeta, stmt, (*ptok)->str);
			if (s)
				goto err;

			stmtmeta_add(&(*create)->in, smeta);
			smeta	= 0;
			s	= PCREATE_FROM_SEP;
			break;

		case PCREATE_FROM_SEP:
			if ((*ptok)->str[0] == ',') {
				s = PCREATE_FROM_VALUE;
				break;
			}

		case PCREATE_FIELD_BEGIN:
			if ((*ptok)->str[0] != '(') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = parsing_FIELD((*create)->in, (*create)->out, ptok);
			if (s)
				goto err;

		case PCREATE_FIELD_END:
			if ((*ptok)->str[0] != ')') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = PCREATE_FILTER;
			break;

		case PCREATE_FILTER:
			s = strcasecmp((*ptok)->str, "FILTER");
			if (s == 0) {
				s = PCREATE_FILTER_BEGIN;
				break;
			}

		case PCREATE_AS:
			s = strcasecmp((*ptok)->str, "AS");
			if (s == 0) {
				s = PCREATE_AS_NAME;
				break;
			}

		case PCREATE_END:
			if ((*ptok)->str[0] != ';') {
				s = E_PARSER_UNX_TOKEN;
				goto err;
			}
			s = PCREATE_DONE;
			break;

		case PCREATE_FILTER_BEGIN:
			if ((*ptok)->str[0] != '(') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = parsing_FILTER((*create)->in, ptok);
			if (s)
				goto err;

		case PCREATE_FILTER_END:
			if ((*ptok)->str[0] != ')') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= (*ptok)->num;
				_vos.e_nparm1	= (*ptok)->str[0];
				goto err;
			}
			s = PCREATE_AS;
			break;

		case PCREATE_AS_NAME:
			(*create)->out->alias	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PCREATE_END;
			break;
		}
		(*ptok)	= (*ptok)->next;
	}

	if (s != PCREATE_DONE) {
		s = E_PARSER_INV_STMT;
err:
		stmtcreate_destroy(create);
	}
	return s;

}

static int parsing_JOIN_where(struct StmtMeta *mref, struct LL **ptok)
{
	int		s		= 0;
	int		todo		= PJOIN_FIELD_START;
	int		join_idx	= 1;
	struct LL	*prevtok	= 0;
	struct StmtMeta	*pmeta		= 0;
	struct Field	*fsearch	= 0;

	while ((*ptok) && todo != PJOIN_FIELD_END) {
		switch (todo) {
		case PJOIN_FIELD_START:
			todo = PJOIN_FIELD_L_PARENT_NAME;
			break;

		case PJOIN_FIELD_L_PARENT_NAME:
		case PJOIN_FIELD_R_PARENT_NAME:
			prevtok = (*ptok);
			if (todo == PJOIN_FIELD_L_PARENT_NAME)
				todo = PJOIN_FIELD_L_PARENT_SEP;
			else
				todo = PJOIN_FIELD_R_PARENT_SEP;
			break;

		case PJOIN_FIELD_L_PARENT_SEP:
		case PJOIN_FIELD_R_PARENT_SEP:
			if ((*ptok)->str[0] == '.') {
				pmeta = stmtmeta_search_filename(mref,
								prevtok->str);
				if (! pmeta) {
					s = E_PARSER_INV_VALUE;
					goto err;
				}
				if (todo == PJOIN_FIELD_L_PARENT_SEP)
					todo = PJOIN_FIELD_L_NAME;
				else
					todo = PJOIN_FIELD_R_NAME;
			} else {
				s = stmtmeta_search_field(mref, prevtok->str,
								&fsearch);
				if (s == E_PARSER_UNK_FIELDNAME)
					goto err;

				if (todo == PJOIN_FIELD_L_PARENT_SEP)
					todo = PJOIN_FIELD_OP;
				else
					todo = PJOIN_FIELD_SEP;
				continue;
			}
			break;

		case PJOIN_FIELD_L_NAME:
		case PJOIN_FIELD_R_NAME:
			s = stmtmeta_search_field(pmeta, (*ptok)->str,
							&fsearch);
			if (s == E_PARSER_UNK_FIELDNAME)
				goto err;
			if (todo == PJOIN_FIELD_L_NAME)
				todo = PJOIN_FIELD_OP;
			else
				todo = PJOIN_FIELD_SEP;
			break;

		case PJOIN_FIELD_OP:
			s = op_get_idx((*ptok)->str);
			if (s < OP_EQUAL || s > OP_TRUE_EQUAL) {
				s = E_PARSER_INV_VALUE;
				goto err;
			}
			fsearch->idx	= join_idx;
			fsearch->flag	|= FFLAG_JOIN;
			fsearch->fop	= _filter_f[s];
			fsearch		= 0;
			pmeta		= 0;
			todo		= PJOIN_FIELD_R_PARENT_NAME;
			break;

		case PJOIN_FIELD_SEP:
			fsearch->idx	|= join_idx;
			fsearch->flag	|= FFLAG_JOIN;
			join_idx	= join_idx << 1;
			fsearch		= 0;
			pmeta		= 0;
			if ((*ptok)->str[0] == ',') {
				todo		= PJOIN_FIELD_START;
				continue;
			}
			if ((*ptok)->str[0] == ')') {
				todo = PJOIN_FIELD_END;
				continue;
			}
			s = E_PARSER_UNK_TOKEN;
			goto err;
		}
		(*ptok) = (*ptok)->next;
	}
	return 0;
err:
	return s;
}

static int parsing_JOIN(struct Stmt *stmt, struct Stmt **join,
			struct LL **ptok)
{
	int			s	= PJOIN_START;
	int			n_input	= 1;
	struct StmtMeta		*smeta	= 0;

	while ((*ptok) && s != PJOIN_DONE) {
		switch (s) {
		case PJOIN_START:
			s = stmtjoin_create(join);
			if (s)
				return s;

			s = PJOIN_INPUT;
			break;

		case PJOIN_INPUT:
			if (n_input > 2) {
				s = E_PARSER_INV_STMT;
				goto err;
			}

			s = stmt_get_output_from(&smeta, stmt, (*ptok)->str);
			if (s)
				goto err;

			stmtmeta_add(&(*join)->in, smeta);
			s = PJOIN_INPUT_JOIN_FLAG;
			break;

		case PJOIN_INPUT_JOIN_FLAG:
			s = join_get_flag((*ptok)->str);
			if (s > 0) {
				smeta->flag |= s;
				s = PJOIN_INPUT_SORT_FLAG;
				break;
			}

		case PJOIN_INPUT_SORT_FLAG:
			s = join_get_sort_flag((*ptok)->str);
			if (s > 0) {
				smeta->flag |= s;
				s = PJOIN_INPUT_SEP;
				break;
			} else {
				smeta->flag |= JOIN_UNSORTED;
			}

		case PJOIN_INPUT_SEP:
			if ((*ptok)->str[0] == ',') {
				n_input++;
				s	= PJOIN_INPUT;
				smeta	= 0;
				break;
			}

			s = strcasecmp((*ptok)->str, "INTO");
			if (s == 0) {
				s = PJOIN_INTO;
				break;
			}

		case PJOIN_FIELD_BEGIN:
			if ((*ptok)->str[0] == '(') {
				if (n_input != 2) {
					s = E_PARSER_INV_STMT;
					goto err;
				}

				s = parsing_JOIN_where((*join)->in, ptok);
				if (s)
					goto err;

				s = PJOIN_FIELD_END;
				continue;
			}
			s = E_PARSER_UNK_TOKEN;
			goto err;

		case PJOIN_FIELD_END:
			if ((*ptok)->str[0] == ')') {
				s = PJOIN_AS;
				break;
			}
			s = E_PARSER_UNK_TOKEN;
			goto err;

		case PJOIN_AS:
			s = strcasecmp((*ptok)->str, "AS");
			if (s == 0) {
				s = PJOIN_AS_NAME;
				break;
			}

		case PJOIN_END:
			if ((*ptok)->str[0] == ';') {
				s = PJOIN_DONE;
				break;
			}
			s = E_PARSER_UNK_TOKEN;
			goto err;

		case PJOIN_INTO:
			(*join)->out->filename	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PJOIN_FIELD_BEGIN;
			break;

		case PJOIN_AS_NAME:
			(*join)->out->alias	= (*ptok)->str;
			(*ptok)->str		= 0;
			s			= PJOIN_END;
			break;
		}
		(*ptok) = (*ptok)->next;
	}
	if (s == PJOIN_DONE) {
		s = stmtjoin_init_output((*join));
	} else
		s = E_PARSER_INV_STMT;
err:
	if (s)
		stmtjoin_destroy(join);

	return s;
}

/**
 * @return:
 *	< 0	: success
 *	< -1	: fail
 */
static int parsing_token(struct Stmt **stmt, struct LL **ptok)
{
	int			s		= 0;
	int			stmt_type	= 0;
	struct Stmt		*new_stmt	= 0;

	while ((*ptok)) {
		stmt_type = stmt_get_type_idx((*ptok)->str);
		if (stmt_type < 0)
			return E_PARSER_UNK_TOKEN;

		switch (stmt_type) {
		case STMT_SET:
			s = parsing_SET(&new_stmt, ptok);
			break;
		case STMT_LOAD:
			s = parsing_LOAD(&new_stmt, ptok);
			break;
		case STMT_SORT:
			s = parsing_SORT((*stmt), &new_stmt, ptok);
			break;
		case STMT_CREATE:
			s = parsing_CREATE((*stmt), &new_stmt, ptok);
			break;
		case STMT_JOIN:
			s = parsing_JOIN((*stmt), &new_stmt, ptok);
			break;
		default:
			return E_PARSER_UNX_TOKEN;
		}
		if (s)
			goto err;
		stmt_add(stmt, new_stmt);
	}
	s = 0;
err:
	return s;
}

int vos_parsing(struct Stmt **stmt, const char *script)
{
	int		s	= 0;
	unsigned int	line_no	= 1;
	struct String	*str	= 0;
	struct File	*F	= 0;
	struct LL	*ls_tok	= 0;
	struct LL	*ptok	= 0;

	s = file_open(&F, script, FOPEN_RO);
	if (s)
		goto err;

	s = file_read(F);
	if (s)
		goto err;

	s = str_create(&str);
	if (s)
		goto err;

	while (F->idx < F->size) {
		switch (FCURC(F)) {
		case CH_NEWLINE:
			line_no++;
		case  ' ': case '\b': case '\f': case '\r': case '\t':
		case '\v':
			break;

		case '#':
			s = file_skip_until(F, CH_NEWLINE);
			if (s)
				goto err;

			line_no++;
			break;

		case '(': case ')': case ':': case ',': case ';': case '.':
		case '+': case '-':
			s = str_append_c(str, FCURC(F));
			if (s)
				goto err;

			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			break;

		case '!': case '=': case '<': case '>':
			do {
				s = str_append_c(str, FCURC(F));
				if (s)
					goto err;
				F->idx++;
				if (F->idx >= F->size) {
					s = file_read(F);
					if (s)
						break;
				}
			} while (FCURC(F) == '=');

			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			continue;

		case '[':
			F->idx++;
			file_fetch_until(F, str, ']');
			if (FCURC(F) != ']') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= line_no;
				_vos.e_nparm1	= FCURC(F);
				goto err;
			}
			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			break;
		case '\"':
			F->idx++;
			file_fetch_until(F, str, '\"');
			if (FCURC(F) != '\"') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= line_no;
				_vos.e_nparm1	= FCURC(F);
				goto err;
			}
			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			break;
		case '\'':
			s = str_append_c(str, FCURC(F));
			if (s)
				goto err;

			F->idx++;
			if (FCURC(F) == '\\')
				F->idx++;

			s = str_append_c(str, FCURC(F));
			if (s)
				goto err;

			F->idx++;
			if (FCURC(F) != '\'') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= line_no;
				_vos.e_nparm1	= FCURC(F);
				goto err;
			}

			s = str_append_c(str, FCURC(F));
			if (s)
				goto err;

			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			break;
		default:
			if (! isalnum(FCURC(F)) && FCURC(F) != '_') {
				s		= E_PARSER_UNX_CHAR;
				_vos.e_nparm0	= line_no;
				_vos.e_nparm1	= FCURC(F);
				goto err;
			}
			while (isalnum(FCURC(F)) || FCURC(F) == '_') {
				s = str_append_c(str, FCURC(F));
				if (s)
					goto err;
				F->idx++;
			}
			s = ll_add(&ls_tok, line_no, str->buf);
			if (s)
				goto err;

			str_prune(str);
			continue;
		}
		F->idx++;
	}

	if (_vos.debug & DBG_PARSER) {
		ll_print(ls_tok);
	}

	ptok	= ls_tok;
	s	= parsing_token(stmt, &ptok);
	if (s)
		goto err;

	goto out;
err:
	switch (s) {
	case E_PARSER_UNK_TOKEN:
	case E_PARSER_UNX_TOKEN:
	case E_PARSER_UNK_FIELDNAME:
	case E_PARSER_INV_FIELDNAME:
	case E_PARSER_INV_VALUE:
	case E_PARSER_INV_STMT:
	case E_PARSER_AMB_FIELDNAME:
		_vos.e_nparm0	= ptok->num;
		_vos.e_sparm0	= ptok->str;
		ptok->str	= 0;
		break;
	case E_PARSER_INV_POS:
		_vos.e_nparm0	= ptok->num;
		break;
	}
out:
	if (_vos.debug & DBG_PARSER) {
		stmt_print((*stmt));
	}
	ll_destroy(&ls_tok);
	str_destroy(&str);
	file_close(&F);

	return s;
}
