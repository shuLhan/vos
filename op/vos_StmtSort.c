#include "op/vos_StmtSort.h"

int stmtsort_create(struct Stmt **sort)
{
	(*sort) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*sort))
		return E_MEM;

	(*sort)->out = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*sort)->out) {
		free((*sort));
		(*sort) = 0;
		return E_MEM;
	}
	(*sort)->type = STMT_SORT;
	return 0;
}

int stmtsort_init(struct Stmt *sort)
{
	int		s		= 0;
	struct Field	*fld_in		= 0;
	struct Field	*fld_out	= 0;

	if (! sort->out) {
		sort->out = (struct StmtMeta *) calloc(1,
						sizeof(struct StmtMeta));
		if (! sort->out)
			return E_MEM;
	}

	if (sort->out->filename) {
		/* check if file output is exist */
		s = file_raw_is_exist(sort->out->filename);
		if (s) {
			str_raw_copy(sort->out->filename, &_vos.e_sparm0);
			return E_FILE_EXIST;
		}
	}

	if (! sort->out->alias) {
		s = str_raw_copy(sort->in->alias, &sort->out->alias);
		if (s)
			return s;
	}

	sort->out->flag = sort->in->flag;

	fld_in = sort->in->fields;
	while (fld_in) {
		fld_out = (struct Field *) calloc(1, sizeof(struct Field));
		if (! fld_out)
			return E_MEM;

		fld_out->idx		= fld_in->idx;
		fld_out->flag		= fld_in->flag;
		fld_out->type		= fld_in->type;
		if (fld_in->next)
			fld_out->sep	= '|';
		fld_out->name		= fld_in->name;
		fld_out->date_format	= fld_in->date_format;

		field_add(&sort->out->fields, fld_out);
		fld_in = fld_in->next;
	}

	return 0;
}

/**
 * @stmtsort_init_output: get temporary file name for sort output.
 *
 * @return:
 *	< 0	: success.
 *	< !0	: fail.
 */
int stmtsort_init_output(struct Stmt *sort)
{
	int		s;
	char		*rndm_name	= 0;
	struct String	*tmp		= 0;

	/* filename already declared by INTO statement */
	if (sort->out->filename)
		return 0;

	str_create(&tmp);

	do {
		str_append(tmp, get_tmp_dir(0));

		s = str_raw_randomize(VOS_SORT_OUT_FORMAT, &rndm_name);
		if (s)
			goto err;

		str_append(tmp, rndm_name);

		s = file_raw_is_exist(tmp->buf);
		if (s)
			str_prune(tmp);

		free(rndm_name);
	} while (s);

	sort->out->flag		|= SORT_TMP;
	sort->out->filename	= tmp->buf;
	tmp->buf		= 0;
err:
	str_destroy(&tmp);
	return s;
}

void stmtsort_print(struct Stmt *sort)
{
	if (! sort)
		return;

	printf("SORT %s ", sort->in->alias ? sort->in->alias
						: sort->in->filename);
	field_print(sort->in->fields);
	printf(" BY \n");
	field_print(sort->out->fields);
	printf(";\n\n");
}

void stmtsort_destroy(struct Stmt **sort)
{
	if (! (*sort))
		return;

	stmtmeta_soft_destroy(&(*sort)->in);
	if ((*sort)->out) {
		if ((*sort)->out->filename) {
			if ((*sort)->out->flag & SORT_TMP)
				unlink((*sort)->out->filename);
			free((*sort)->out->filename);
		}
		if ((*sort)->out->alias)
			free((*sort)->out->alias);
		field_soft_destroy(&(*sort)->out->fields);
		free((*sort)->out);
	}
	free((*sort));
	(*sort) = 0;
}
