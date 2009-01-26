#include "op/vos_StmtJoin.h"

const char *_join_flag[N_JOIN_FLAG] = {
	"\0",
	"+\0",
	"-\0"
};

const char *_join_sort_flag[N_JOIN_SORT_FLAG] = {
	"\0",
	"\0",
	"\0",
	"\0",
	"UNSORTED\0",
	"\0",
	"\0",
	"\0",
	"SORTED\0"
};

int stmtjoin_create(struct Stmt **join)
{
	(*join) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*join))
		return E_MEM;

	(*join)->out = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*join)->out) {
		free((*join));
		(*join) = 0;
		return E_MEM;
	}
	(*join)->type = STMT_JOIN;
	return 0;
}

int stmtjoin_init_output(struct Stmt *join)
{
	int		s;
	struct Field	*field		= 0;
	struct Field	*fld_new	= 0;
	struct String	*str		= 0;
	struct StmtMeta	*inr		= 0;

	if (! join)
		return 0;

	if (! join->out->filename) {
		do {
			s = str_raw_randomize(VOS_JOIN_OUT_FORMAT,
						&join->out->filename);
			if (s)
				return s;

			s = file_raw_is_exist(join->out->filename);
			if (s)
				free(join->out->filename);
		} while (s);
	} else {
		s = file_raw_is_exist(join->out->filename);
		if (s) {
			str_raw_copy(join->out->filename, &_vos.e_sparm0);
			return E_FILE_EXIST;
		}
	}

	s = str_create(&str);
	if (s)
		return s;

	field = join->in->fields;
	while (field) {
		fld_new = (struct Field *) calloc(1, sizeof(struct Field));
		if (! fld_new) {
			s = E_MEM;
			goto err;
		}

		fld_new->type		= field->type;
		fld_new->sep		= '|';
		fld_new->date_format	= field->date_format;

		if (join->in->alias) {
			str_append(str, join->in->alias);
			str_append(str, ".");
		}
		str_append(str, field->name);
		str_detach(str, &fld_new->name);

		field_add(&join->out->fields, fld_new);
		field = field->next;
	}

	inr	= join->in->next;
	field	= inr->fields;
	while (field) {
		fld_new = (struct Field *) calloc(1, sizeof(struct Field));
		if (! fld_new) {
			s = E_MEM;
			goto err;
		}

		fld_new->type		= field->type;
		if (field->next)
			fld_new->sep	= '|';
		fld_new->date_format	= field->date_format;

		if (inr->alias) {
			str_append(str, inr->alias);
			str_append(str, ".");
		}
		str_append(str, field->name);
		str_detach(str, &fld_new->name);

		field_add(&join->out->fields, fld_new);
		field = field->next;
	}
	s = 0;
err:
	str_destroy(&str);
	return s;
}

void stmtjoin_print(struct Stmt *join)
{
	printf("JOIN %s (%2d) ", join->in->alias, join->in->flag);
	field_print(join->in->fields);

	printf("WITH %s (%2d) ", join->in->next->alias, join->in->next->flag);
	field_print(join->in->next->fields);

	printf("INTO %s ", join->out->filename);
	field_print(join->out->fields);

	if (join->out->alias)
		printf(" AS %s ;\n", join->out->alias);
	else
		printf(";\n");
}

void stmtjoin_destroy(struct Stmt **join)
{
	struct StmtMeta *inl = 0; 
	struct StmtMeta *inr = 0;

	if (! (*join))
		return;

	if ((*join)->in)
		inl = (*join)->in;
	if (inl)
		inr = inl->next;
	
	if (inr) {
		if (inr->flag & JOIN_UNSORTED) {
			if (inr->filename)
				free(inr->filename);
			if (inr->alias)
				free(inr->alias);
			field_soft_destroy(&inr->fields);
			free(inr);
		} else
			stmtmeta_soft_destroy(&(*join)->in->next);
		(*join)->in->next = 0;
	}

	if (inl) {
		if (inl->flag & JOIN_UNSORTED) {
			if (inl->filename)
				free(inl->filename);
			if (inl->alias)
				free(inl->alias);
			field_soft_destroy(&inl->fields);
			free(inl);
		} else
			stmtmeta_soft_destroy(&(*join)->in);
	}

	stmtmeta_destroy(&(*join)->out);
	free((*join));
	(*join) = 0;
}
