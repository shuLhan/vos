#include "op/vos_StmtSort.h"

int stmtsort_create(struct Stmt **sort)
{
	(*sort) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*sort))
		return E_MEM;

	(*sort)->out = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*sort)->out) {
		free((*sort)->in);
		free((*sort));
		(*sort) = 0;
		return E_MEM;
	}
	(*sort)->type = STMT_SORT;
	return 0;
}

int stmtsort_init_output(struct Stmt *sort)
{
	int		s		= E_MEM;
	struct Field	*fld_in		= 0;
	struct Field	*fld_out	= 0;

	if (! sort->out) {
		sort->out = (struct StmtMeta *) calloc(1,
						sizeof(struct StmtMeta));
		if (! sort->out)
			return E_MEM;
	}

	if (! sort->out->filename) {
		do {
			s = str_raw_randomize(VOS_SORT_OUT_FORMAT,
							&sort->out->filename);
			if (s)
				return s;

			s = file_raw_is_exist(sort->out->filename);
			if (s)
				free(sort->out->filename);
		} while (s);
	} else {
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
		if ((*sort)->out->filename)
			free((*sort)->out->filename);
		if ((*sort)->out->alias)
			free((*sort)->out->alias);
		field_soft_destroy(&(*sort)->out->fields);
		free((*sort)->out);
	}
	free((*sort));
	(*sort) = 0;
}
