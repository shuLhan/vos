#include "op/vos_Field.h"

const char *_field_type[N_FIELD_TYPE] = {
	"STRING\0",
	"NUMBER\0",
	"DATETIME\0"
};

const char *_fflag_sort[N_FFLAG_SORT] = {
	"\0",
	"ASC\0",
	"DESC\0"
};

/**
 * @desc:
 *	create new Field object 'cp' by copying properties of field 'fld',
 *	instead of allocating new field name and date_format, this new Field
 *	object only reference their value from 'fld'.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int field_soft_copy(struct Field *fld, struct Field **cp)
{
	(*cp) = (struct Field *) calloc(1, sizeof(struct Field));
	if (! (*cp))
		return E_MEM;

	(*cp)->idx		= 0;
	(*cp)->flag		= 0;
	(*cp)->type		= fld->type;
	(*cp)->left_q		= fld->left_q;
	(*cp)->right_q		= fld->right_q;
	(*cp)->start_p		= fld->start_p;
	(*cp)->end_p		= fld->end_p;
	(*cp)->sep		= fld->sep;
	(*cp)->date_format	= fld->date_format;
	(*cp)->name		= fld->name;
	(*cp)->next		= 0;

	return 0;
}

void field_add(struct Field **fields, struct Field *f)
{
	struct Field *p = 0;

	if (! (*fields)) {
		(*fields) = f;
	} else {
		p = (*fields);
		while (p->next)
			p = p->next;
		p->next = f;
	}
}

void field_print(struct Field *fields)
{
	printf("(\n");
	while (fields) {
		printf("\t[%2d] '%c' : %s : '%c' : %3d : %3d : '%c' : %s : %s "\
			"(%2d)\n",
			fields->idx, fields->left_q,
			fields->name ? fields->name : "\0",
			fields->right_q, fields->start_p, fields->end_p,
			fields->sep, _field_type[fields->type],
			fields->date_format ? fields->date_format : "\0",
			fields->flag);
		if (fields->fltr_idx) {
			printf("\tFILTER %d: %s (%p) %s\n", fields->fltr_idx,
				_filter_rule[fields->fltr_rule],
				fields->fop,
				fields->fltr_v ? fields->fltr_v : "\0");
		}
		fields = fields->next;
	}
	printf(") ");
}

void _field_destroy(struct Field **field, const int soft)
{
	struct Field *next = 0;

	while ((*field)) {
		if (! soft) {
			if ((*field)->date_format)
				free((*field)->date_format);
			if ((*field)->name)
				free((*field)->name);
		}
		if ((*field)->fltr_v)
			free((*field)->fltr_v);
		next = (*field)->next;
		free((*field));
		(*field) = next;
	}
}
