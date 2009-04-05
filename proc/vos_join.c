#include "proc/vos_join.h"

static int join_do_sort(struct Stmt *join)
{
	int		s	= 0;
	struct Stmt	*sort	= 0;
	struct StmtMeta	*inr	= join->in->next;

	if (join->in->flag & JOIN_UNSORTED) {
		s = stmtsort_create(&sort);
		if (s)
			return s;

		join->in->next	= 0;
		sort->in	= join->in;

		s = stmtsort_init(sort);
		if (s)
			goto err;

		s = vos_process_sort(sort);
		if (s)
			goto err;

		join->in	= sort->out;
		join->in->next	= inr;
		sort->out	= 0;

		stmtmeta_soft_destroy(&sort->in);
	}

	if (inr->flag & JOIN_UNSORTED) {
		if (! sort) {
			s = stmtsort_create(&sort);
			if (s)
				return s;
		}

		sort->in = join->in->next;

		s = stmtsort_init(sort);
		if (s)
			goto err;

		s = vos_process_sort(sort);
		if (s)
			goto err;

		join->in->next	= sort->out;
		sort->out	= 0;
	}
err:
	if (sort)
		stmtsort_destroy(&sort);
	return s;
}

int vos_process_join(struct Stmt *join)
{
	int		s;
	struct Field	*field	= 0;
	struct File	*F	= 0;
	struct File	*FL	= 0;
	struct File	*FR	= 0;
	struct Record	*rowl	= 0;
	struct Record	*rowr	= 0;
	struct Record	*emptyl	= 0;
	struct Record	*emptyr	= 0;
	struct Record	*empty	= 0;
	struct StmtMeta	*inl	= join->in;
	struct StmtMeta	*inr	= join->in->next;
	struct String	*str	= 0;

	/* prepare for empty fields */
	if (inl->flag & JOIN_OUTER
	||  inl->flag & JOIN_ANTI
	||  inr->flag & JOIN_ANTI) {
		field = inl->fields;
		while (field) {
			s = str_create(&str);
			if (s)
				goto err;

			s = record_new(&empty, field, str);
			if (s)
				goto err;

			record_add_field(&emptyr, empty);
			empty	= 0;
			str	= 0;
			field	= field->next;
		}
	}
	if (inr->flag & JOIN_OUTER
	||  inr->flag & JOIN_ANTI
	||  inl->flag & JOIN_ANTI) {
		field = inr->fields;
		while (field) {
			s = str_create(&str);
			if (s)
				goto err;

			s = record_new(&empty, field, str);
			if (s)
				goto err;

			record_add_field(&emptyl, empty);
			empty	= 0;
			str	= 0;
			field	= field->next;
		}
	}

	s = join_do_sort(join);
	if (s)
		goto err;

	inl = join->in;
	inr = join->in->next;

	s = file_open(&F, join->out->filename, FOPEN_WO);
	if (s)
		goto err;

	s = file_open(&FL, inl->filename, FOPEN_RO);
	if (s)
		goto err;

	s = file_open(&FR, inr->filename, FOPEN_RO);
	if (s)
		goto err;

	s = file_read(FL);
	if (s)
		goto err;

	s = file_read(FR);
	if (s)
		goto err;

	s = record_read(&rowl, FL, inl->fields);
	s = record_read(&rowr, FR, inr->fields);
	while (s == 0) {
		s = record_cmp(rowl, rowr);
		if (s < 0) {
			/* x+ || x- */
			if (inl->flag & JOIN_OUTER
			||  inl->flag & JOIN_ANTI) {
				rowl->fld_last->fld_next = emptyr;
				record_write(rowl, F, join->out->fields);
				rowl->fld_last->fld_next = 0;
			} else {
				record_prune(rowl);
			}
			s = record_read2(rowl, FL, inl->fields);
			if (s)
				break;
		} else if (s == 0) {
			/* x- && y- */
			if ((inl->flag & JOIN_ANTI)
			&&  (inr->flag & JOIN_ANTI)) {
				record_prune(rowl);
				record_prune(rowr);
			} else if (inl->flag & JOIN_ANTI) {
				emptyl->fld_last->fld_next = rowr;
				record_write(emptyl, F, join->out->fields);
				emptyl->fld_last->fld_next = 0;
				record_prune(rowl);
			} else if (inr->flag & JOIN_ANTI) {
				rowl->fld_last->fld_next = emptyr;
				record_write(rowl, F, join->out->fields);
				rowl->fld_last->fld_next = 0;
				record_prune(rowr);
			} else {
				rowl->fld_last->fld_next = rowr;
				record_write(rowl, F, join->out->fields);
				rowl->fld_last->fld_next = 0;
			}

			s = record_read2(rowl, FL, inl->fields);
			if (s)
				break;
			s = record_read2(rowr, FR, inr->fields);
			if (s)
				break;
		} else {
			/* y+ || y- */
			if (inr->flag & JOIN_OUTER
			||  inr->flag & JOIN_ANTI) {
				emptyl->fld_last->fld_next = rowr;
				record_write(emptyl, F, join->out->fields);
				emptyl->fld_last->fld_next = 0;
			} else {
				record_prune(rowr);
			}

			s = record_read2(rowr, FR, inr->fields);
			if (s)
				break;
		}
	}

	if (FL->size
	&& ((inl->flag & JOIN_OUTER) || inl->flag & JOIN_ANTI)) {
		while (FL->size) {
			rowl->fld_last->fld_next = emptyr;
			record_write(rowl, F, join->out->fields);
			rowl->fld_last->fld_next = 0;

			s = record_read2(rowl, FL, inl->fields);
		}
	}

	if (FR->size
	&& ((inr->flag & JOIN_OUTER) || inr->flag & JOIN_ANTI)) {
		while (FR->size) {
			emptyl->fld_last->fld_next = rowr;
			record_write(emptyl, F, join->out->fields);
			emptyl->fld_last->fld_next = 0;

			s = record_read2(rowr, FR, inr->fields);
		}
	}
	file_write(F);
	s = 0;
err:
	record_destroy(&rowl);
	record_destroy(&rowr);
	file_close(&FR);
	file_close(&FL);
	file_close(&F);
	record_destroy(&emptyl);
	record_destroy(&emptyr);

	/* remove temporary sort file */
	if (inl->flag & JOIN_UNSORTED) {
		unlink(inl->filename);
	}
	if (inr->flag & JOIN_UNSORTED) {
		unlink(inr->filename);
	}

	return s;
}
