#include "proc/vos_create.h"

static void * create_process(void *parm)
{
	int			s;
	unsigned long		n_row	= 0;
	struct ProcCreate	*cproc	= 0;
	struct File		*F	= 0;
	struct Record		*R	= 0;
	struct Field		*fld	= 0;

	cproc = (struct ProcCreate *) parm;
	s = file_open(&F, cproc->in->filename, FOPEN_RO);
	if (s)
		goto err;

	s = file_read(F);
	if (s)
		goto err;

	/* phase 1: create & fill bucket */
	do {
		s = record_read_filtered(&R, F, cproc->in->fields);
		if (R) {
			record_to_bucket(cproc->buckets, R);
			n_row++;
			R = 0;
		}
	} while (s == 0 && n_row < _vos.proc_max_row);
	if (s && s != E_FILE_END)
		goto err;

	/* set row last to mark the end of row */
	fld = cproc->in->fields;
	while (fld) {
		if (fld->idx) {
			if (cproc->buckets[fld->idx].cnt) {
				cproc->buckets[fld->idx].cnt->row_last =
					cproc->buckets[fld->idx].cnt->row_last->row_next;
			}
		}
		fld = fld->next;
	}

	if (_vos.debug & DBG_CREATE) {
		printf("child: bucket full\n");
	}

	cproc->status = CPROC_BUCKETS_FULL;
	while (cproc->status == CPROC_BUCKETS_FULL)
		sleep(THREAD_TIME_WAIT);

	if (cproc->status != CPROC_START)
		goto out;
	if (n_row < _vos.proc_max_row)
		goto out;

	if (_vos.debug & DBG_CREATE) {
		printf("child: bucket empty\n");
	}

	/* phase 2:
	 * this is where bucket is already filled with size == _vos.proc_max_row
	 * but empty. so we just need to fill the empty row, no need to
	 * malloc/create it again.
	 */
	n_row = 0;
	do {
		s = bucket_read_filtered(cproc->buckets, F,
						cproc->in->fields);
		if (s == 0) {
			n_row++;
		} else if (s > 0 && s != E_FILE_END)
			goto err;

		if (n_row >= _vos.proc_max_row) {
			fld = cproc->in->fields;
			while (fld) {
				if (fld->idx) {
					cproc->buckets[fld->idx].cnt->row_last
						= cproc->buckets[fld->idx].p;
				}
				fld = fld->next;
			}

			if (_vos.debug & DBG_CREATE) {
				printf("child: bucket full\n");
			}

			cproc->status = CPROC_BUCKETS_FULL;
			while (cproc->status == CPROC_BUCKETS_FULL)
				sleep(THREAD_TIME_WAIT);

			if (cproc->status != CPROC_START)
				goto out;

			if (_vos.debug & DBG_CREATE) {
				printf("child: bucket empty\n");
			}

			n_row = 0;
		}
	} while (s <= 0);

	if (n_row) {
		fld = cproc->in->fields;
		while (fld) {
			if (fld->idx) {
				cproc->buckets[fld->idx].cnt->row_last =
					cproc->buckets[fld->idx].p;
			}
			fld = fld->next;
		}
	}
out:
	s = 0;
err:
	switch (s) {
	case E_FILE_OPEN:
	case E_FILE_NOT_EXIST:
		vos_error1(s, cproc->in->filename);
		break;
	case E_FILE_NOT_OPEN:
	case E_FILE_READ:
		vos_error0(s);
		break;
	}
	cproc->status = CPROC_DONE;
	file_close(&F);

	cproc->retval = s;
	return 0; 
}

int vos_process_create(struct Stmt *create)
{
	int			i		= 0;
	int			s		= 0;
	int			n_in		= 0;
	int			n_done		= 0;
	int			n_bucket	= 1;
	struct File		*F		= 0;
	struct StmtMeta		*pin		= create->in;
	struct ProcCreate	*cproc		= 0;
	struct Bucket		*buckets	= 0;
	struct Field		*fld		= 0;

	/* how many input ? */
	while (pin) {
		n_in++;
		pin = pin->next;
	}

	/* how many field output (bucket) ? */
	fld = create->out->fields;
	while (fld) {
		n_bucket++;
		fld = fld->next;
	}

	s = file_open(&F, create->out->filename, FOPEN_WO);
	if (s)
		goto err;

	cproc = (struct ProcCreate *) calloc(n_in, sizeof(struct ProcCreate));
	if (! cproc) {
		s = E_MEM;
		goto err;
	}

	buckets = (struct Bucket *) calloc(n_bucket, sizeof(struct Bucket));
	if (! buckets) {
		s = E_MEM;
		goto err;
	}

	pin = create->in;
	for (i = 0; i < n_in; i++) {
		stmt_update_meta(create->prev, pin);

		cproc[i].in		= pin;
		cproc[i].status		= CPROC_START;
		cproc[i].buckets	= buckets;

		pthread_create(&cproc[i].tid, 0, create_process,
				(void *) &cproc[i]);
		pin = pin->next;
	}

	/* phase 1: create & fill bucket */
	for (i = 0; i < n_in; i++) {
		/* wait until child fill the buckets */
		while (cproc[i].status == CPROC_START)
			sleep(THREAD_TIME_WAIT);

		if (cproc[i].status == CPROC_DONE) {
			pthread_join(cproc[i].tid, 0);
			if (cproc[i].retval) {
				s = cproc[i].retval;
				goto err;
			}

			n_done++;
			cproc[i].status	= CPROC_END;
		}
	}

	if (_vos.debug & DBG_CREATE) {
		printf("parent: writing bucket\n");
	}
	s = bucket_write(buckets, n_bucket, F, create->out->fields);
	if (s)
		goto err;

	/* phase 2: fill the buckets */
	while (n_done < n_in) {
		for (i = 0; i < n_in; i++) {
			/* restart threads */
			if (cproc[i].status == CPROC_BUCKETS_FULL)
				cproc[i].status = CPROC_START;
		}

		for (i = 0; i < n_in; i++) {
			/* wait until child fill the buckets */
			while (cproc[i].status == CPROC_START)
				sleep(THREAD_TIME_WAIT);

			if (cproc[i].status == CPROC_DONE) {
				pthread_join(cproc[i].tid, 0);

				/* catch error here */
				if (cproc[i].retval) {
					s = cproc[i].retval;
					goto err;
				}

				n_done++;
				cproc[i].status	= CPROC_END;
			}
		}

		if (_vos.debug & DBG_CREATE) {
			printf("parent: writing bucket\n");
		}
		s = bucket_write(buckets, n_bucket, F, create->out->fields);
		if (s)
			goto err;
	}

err:
	if (s && cproc) {
		/* cancel all thread */
		for (i = 0; i < n_in; i++) {
			if (cproc[i].status == CPROC_BUCKETS_FULL) {
				cproc[i].status	= CPROC_DONE;
				pthread_join(cproc[i].tid,
						(void *) &cproc[i].retval);
			}
		}
	}

	bucket_destroy(buckets, n_bucket);

	if (buckets)
		free(buckets);
	if (cproc)
		free(cproc);
	if (F)
		file_close(&F);
	return s;
}
