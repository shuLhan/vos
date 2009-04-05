#include "proc/vos_sort.h"

/**
 * sort: sort record using mergesort
 * @desc:
 *	about mergesort. the key is in merge.
 *	merge case 1:
 *		1 2 3
 *		4 5 6
 *	merge case 2:
 *		4 5 6
 *		1 2 3
 *	merge case 3:
 *		1 3 5
 *		2 4 6
 */
static struct Record * sort(struct Record *rows, unsigned long n_row)
{
	int		s	= 0;
	unsigned long	i	= 1;
	unsigned long	nl	= 0;
	unsigned long	nr	= 0;
	struct Record	*l	= 0;
	struct Record	*r	= 0;
	struct Record	*tmp	= 0;

	if (n_row <= 1) {
		if (rows)
			rows->row_last = rows;
		return rows;
	}

	nl = n_row / 2;
	nr = n_row - nl;

	tmp	= rows->row_last;
	l	= rows;
	for (; i < nl; i++) 
		rows = rows->row_next;

	l->row_last		= rows;
	r			= rows->row_next;
	r->row_last		= tmp;
	l->row_last->row_next	= 0;

	if (nl > 1)
		l = sort(l, nl);
	if (nr > 1)
		r = sort(r, nr);

	/* merge case 1 */
	s = record_cmp(l->row_last, r);
	if (s <= 0) {
		l->row_last->row_next	= r;
		l->row_last		= r->row_last;
		r->row_last		= 0;
		return l;
	}
	/* merge case 2 : must be '>' not '>=' to keep 'stable' result */
	s = record_cmp(l, r->row_last);
	if (s > 0) {
		r->row_last->row_next	= l;
		r->row_last		= l->row_last;
		l->row_last		= 0;
		return r;
	}
	/* merge case 3 */
	rows	= 0;
	s	= record_cmp(l, r);
	if (s < 0) {
		rows		= l;
		rows->row_last	= rows;
		l		= l->row_next;
	} else if (s == 0) {
		rows		= l;
		l		= l->row_next;
		rows->row_next	= r;
		rows->row_last	= r;
		r		= r->row_next;
	} else {
		rows		= r;
		rows->row_last	= rows;
		r		= r->row_next;
	}

	while (l && r) {
		s = record_cmp(l, r);
		if (s < 0) {
			rows->row_last->row_next	= l;
			rows->row_last			= l;
			l				= l->row_next;
		} else if (s == 0) {
			rows->row_last->row_next		= l;
			l					= l->row_next;
			rows->row_last->row_next->row_next	= r;
			rows->row_last				= r;
			r					= r->row_next;
		} else {
			rows->row_last->row_next	= r;
			rows->row_last			= r;
			r				= r->row_next;
		}
	}

	if (l) {
		rows->row_last->row_next = l;
		while (l->row_next)
			l = l->row_next;
		rows->row_last = l;
	}
	if (r) {
		rows->row_last->row_next = r;
		while (r->row_next)
			r = r->row_next;
		rows->row_last = r;
	}

	return rows;
}

static int sort_write(struct ProcSort *psort, struct Record *rows)
{
	int		s		= 0;
	char		*rndm_name	= 0;
	struct String	*tmp		= 0;
	struct File	*Fout		= 0;

	str_create(&tmp);

	do {
		str_append(tmp, get_tmp_dir(1));

		/* get random file name */
		s = str_raw_randomize(VOS_SORT_TMP_FORMAT, &rndm_name);
		if (s)
			goto err;

		str_append(tmp, rndm_name);

		s = file_open(&Fout, tmp->buf, FOPEN_WO);
		if (s == 0)
			ll_add(&psort->lsout, psort->tid, tmp->buf);

		str_prune(tmp);
		free(rndm_name);
	} while (s == E_FILE_EXIST);

	s = record_write(rows, Fout, psort->sort->out->fields);

	file_write(Fout);
	file_close(&Fout);

err:
	str_destroy(&tmp);
	return s;
}

/**
 * @desc:
 *	for each thread, sort input file from pos_start to pos_end
 */
static void *sort_process(void *parm)
{
	int		s	= 0;
	unsigned long	n_row	= 0;
	struct File	*F	= 0;
	struct Record	*R	= 0;
	struct ProcSort	*psort	= 0;
	struct Record	*rows	= 0;

	psort = (struct ProcSort *) parm;

	if (_vos.debug & DBG_SORT) {
		printf("(%lu) sort pos: %ld to %ld\n", psort->tid,
			psort->pos_start, psort->pos_end);
	}

	s = file_open(&F, psort->sort->in->filename, FOPEN_RO);
	if (s)
		goto err;

	F->pos = psort->pos_start;
	if (psort->pos_start != 0) {
		s = lseek(F->d, psort->pos_start, SEEK_SET);
		if (s < 0) {
			s = E_FILE_SEEK;
			str_raw_copy(F->name, &_vos.e_sparm0);
			goto err;
		}

		s = file_read(F);
		if (s)
			goto err;

		/* skip one row */
		s = file_skip_until(F, CH_NEWLINE);
		if (s)
			goto err;
		F->idx++;
	} else {
		s = file_read(F);
		if (s)
			goto err;
	}

	if (_vos.debug & DBG_SORT) {
		printf("(%lu) pos start : %ld\n", psort->tid, FCURP(F));
	}

	/* phase 1: create & fill rows */
	while (FCURP(F) < psort->pos_end) {
		s = record_read(&R, F, psort->sort->in->fields);
		if (! R)
			break;

		record_add_row(&psort->rows, R);
		psort->n_row++;
		if (psort->n_row >= _vos.proc_max_row)
			break;
	}

	if (_vos.debug & DBG_SORT) {
		printf("(%lu) pos end   : %ld\n", psort->tid, FCURP(F));
	}

	/* check if error, but not end of file */
	if (s && s != E_FILE_END)
		goto err;

	/* sort rows */
	psort->rows = sort(psort->rows, psort->n_row);

	/* dump rows */
	s = sort_write(psort, psort->rows);
	if (s)
		goto err;

	if (psort->n_row < _vos.proc_max_row || s == E_FILE_END)
		goto out;

	/* phase 2: only use already-created-rows */
	R			= psort->rows;
	psort->rows		= R->row_next;
	psort->rows->row_last	= R->row_last;
	R->row_next		= 0;
	R->row_last		= 0;
	n_row			= 0;
	while (FCURP(F) < psort->pos_end) {
		s = record_read2(R, F, psort->sort->in->fields);
		if (s) {
			if (s == E_FILE_END)
				break;
			goto err;
		}

		record_add_row(&rows, R);
		n_row++;
		if (n_row >= _vos.proc_max_row) {
			rows = sort(rows, n_row);

			s = sort_write(psort, rows);
			if (s)
				goto err;

			psort->rows	= rows;
			rows		= 0;
			n_row		= 0;
		}

		R		= psort->rows;
		psort->rows	= R->row_next;
		if (psort->rows) {
			psort->rows->row_last	= R->row_last;
			R->row_next		= 0;
			R->row_last		= 0;
		}
	}

	if (n_row) {
		if (_vos.debug & DBG_SORT) {
			printf(" (%lu) : F->pos %lu, F->idx %lu, "\
				"F->size %lu\n", psort->tid, F->pos, F->idx,
				F->size);
			printf(" (%lu) : %lu\n", psort->tid, n_row);
			printf(" (%lu) : last row : ", psort->tid);
			record_print(rows->row_last);
		}

		rows = sort(rows, n_row);

		s = sort_write(psort, rows);
		if (s)
			goto err;

		R->row_next = rows;
		psort->rows->row_last->row_next	= R;
		psort->rows->row_last		= rows->row_last;
		psort->rows->row_last->row_next	= 0;
		rows->row_last			= 0;
	}

	if (_vos.debug & DBG_SORT) {
		printf(" (%lu) psort first row : %p\n", psort->tid, psort->rows);
		printf(" (%lu) psort last row  : %p\n", psort->tid, psort->rows->row_last);
		printf(" (%lu) counting rows ...\n", psort->tid);
		n_row = 0;
		rows = psort->rows;
		while (rows->row_next) {
			n_row++;
			rows = rows->row_next;
		}
		n_row++;
		printf(" (%lu) number of rows  : %lu\n", psort->tid, n_row);
		printf(" (%lu) last row        : %p\n", psort->tid, rows);
	}
out:
	if (_vos.debug & DBG_SORT)
		ll_print(psort->lsout);
	s = 0;
err:
	file_close(&F);
	psort->retval = s;

	return (void *) s;
}

int vos_process_sort(struct Stmt *sort)
{
	int		i		= 0;
	int		s		= 0;
	unsigned long	fsize		= 0;
	unsigned long	esize		= 0;
	unsigned long	all_n_row	= 0;
	struct ProcSort	*sort_proc	= 0;
	struct LL	*lsout		= 0;
	struct LL	*ll		= 0;
	struct Record	*all_rows	= 0;
	struct Record	*cnt_rows	= 0;

	s = file_raw_get_size(sort->in->filename, &fsize);
	if (s)
		goto err;

	esize = fsize / _vos.proc_max;

	sort_proc = (struct ProcSort *) calloc(_vos.proc_max,
						sizeof(struct ProcSort));
	if (! sort_proc) {
		s = E_MEM;
		goto err;
	}

	/* create thread for sort */
	for (i = 0; i < _vos.proc_max; i++) {
		if (i == 0)
			sort_proc[i].pos_start = i * esize;
		else
			sort_proc[i].pos_start = (i * esize) - 1;

		if ((i + 1) == _vos.proc_max)
			sort_proc[i].pos_end = fsize;
		else
			sort_proc[i].pos_end = sort_proc[i].pos_start + esize;

		sort_proc[i].n_row	= 0;
		sort_proc[i].rows	= 0;
		sort_proc[i].sort	= sort;

		pthread_create(&sort_proc[i].tid, 0, sort_process,
				(void *) &sort_proc[i]);
	}

	for (i = 0; i < _vos.proc_max; i++) {
		pthread_join(sort_proc[i].tid, 0);

		if (sort_proc[i].retval && !s)
			s = sort_proc[i].retval;
		else {
			ll_link(&lsout, sort_proc[i].lsout);
			if (! all_rows)
				all_rows = sort_proc[i].rows;
			else {
				all_rows->row_last->row_next =
							sort_proc[i].rows;
				all_rows->row_last = sort_proc[i].rows->row_last;
			}
			all_n_row += sort_proc[i].n_row;
		}
	}

	if (! s) {
		if (_vos.debug & DBG_SORT) {
			esize = 0;
			cnt_rows = all_rows;
			printf(" all-rows first row : %p\n", all_rows);
			printf(" all-rows last row  : %p\n", all_rows->row_last);
			while (cnt_rows) {
				esize++;
				cnt_rows = cnt_rows->row_next;
				if (cnt_rows == all_rows->row_last)
					printf(" last row found in : %lu\n", esize);
			}
			printf(" count all rows : %lu\n", esize);
		}
		/* use already created rows for merge */
		s = vos_sort_merge(sort, lsout, &all_rows, all_n_row);
	}
err:
	/* remove all temporary file */
	ll = lsout;
	while (ll) {
		unlink(ll->str);
		ll = ll->next;
	}
	ll_destroy(&lsout);
	record_destroy(&all_rows);
	free(sort_proc);
	return s;
}
