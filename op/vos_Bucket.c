#include "op/vos_Bucket.h"

void bucket_print(struct Bucket *B, const int n)
{
	int	i;
	int	empty = 1;

	if (! B)
		return;

	for (i = 1; i < n; i++) {
		if (! B[i].cnt) {
			B[i].stat = 1;
			empty++;
		} else if (B[i].p == B[i].cnt) {
			B[i].stat = 1;
			empty++;
		} else {
			B[i].stat = 0;
			B[i].p = B[i].cnt;
		}
	}

	while (empty < n) {
		for (i = 1; i < n; i++) {
			if (! B[i].stat) {
				printf("(%2d) \"%s\"|", B[i].p->idx,
					B[i].p->v->buf);
				B[i].p = B[i].p->row_next;
				if (B[i].p == B[i].cnt->row_last || ! B[i].p) {
					empty++;
					B[i].stat = 1;
					B[i].p = 0;
					B[i].cnt->row_last = 0;
				}
			} else {
				printf("\"\"|");
			}
		}
		printf("\n");
	}
}

/**
 * @desc:
 *	move each field on Record 'R' into a bucket
 */
void record_to_bucket(struct Bucket *B, struct Record *R)
{
	struct Record *p = 0;

	if (! B)
		return;
	if (R)
		R->fld_last = 0;
	while (R) {
		p = R;
		if (! B[R->idx].cnt) {
			B[R->idx].cnt = R;
			B[R->idx].cnt->row_next = 0;
		} else {
			B[R->idx].cnt->row_last->row_next = R;
		}
		B[R->idx].cnt->row_last = R;
		R = R->fld_next;
		p->fld_next = 0;
	}
}

void bucket_empty(struct Bucket *B, const int n)
{
	int i;

	if (! B)
		return;
	for (i = 1; i < n; i++) {
		B[i].p = B[i].cnt;
		while (B[i].p) {
			str_prune(B[i].p->v);
			B[i].p = B[i].p->row_next;
		}
	}
}

void bucket_destroy(struct Bucket *B, const int n)
{
	int i;

	if (! B)
		return;
	for (i = 1; i < n; i++) {
		record_destroy(&B[i].cnt);
		B[i].p = 0;
	}
}

static int bucket_file_write(struct File *F)
{
	int		s	= 0;
	unsigned long	len	= 0;
	unsigned long	fidx	= F->idx;

	/* set F->idx to the last new line */
	while (F->idx > 0 && FCURC(F) != CH_NEWLINE)
		F->idx--;

	len	= fidx - F->idx;
	fidx	= F->idx;

	s = file_write(F);
	if (s)
		return s;

	F->buf = memmove(F->buf, &F->buf[fidx], len);
	F->idx = len;

	return 0;
}

static int record_write_once(struct Record *R, struct File *F,
				struct Field *frmt, int *start_p)
{
	int	s;
	long	len	= 0;
	long	len2	= 0;

	/* set start position */
	if (frmt->start_p) {
		if ((*start_p) > frmt->start_p) {
			while ((*start_p) > frmt->start_p) {
				FCURC(F) = ' ';
				F->idx--;
				(*start_p)--;
			}
		} else if ((*start_p) < frmt->start_p) {
			while ((*start_p) < frmt->start_p) {
				(*start_p)++;
				FCURC(F) = ' ';
				F->idx++;
				if (F->idx >= F->size) {
					s = bucket_file_write(F);
					if (s)
						return s;
				}
			}
		}
	}

	/* write left quote */
	if (frmt->left_q) {
		(*start_p)++;
		FCURC(F) = frmt->left_q;
		if (++F->idx >= F->size) {
			s = bucket_file_write(F);
			if (s)
				return s;
		}
	}

	/* write content of Record */
	len = R->v->idx;
	if (frmt->end_p) {
		/* + 1, because we start from zero */
		len2 = frmt->end_p - frmt->start_p + 1;
		if (frmt->left_q)
			len2--;
		if (frmt->right_q)
			len2--;
		if (len2 < len)
			len = len2;
	}
	if ((F->idx + len) >= F->size) {
		s = bucket_file_write(F);
		if (s)
			return s;
	}
	if (len > 0) {
		memcpy(&FCURC(F), R->v->buf, len);
		F->idx += len;
		(*start_p) += len;
		str_prune(R->v);
	}

	/* write right quote */
	if (frmt->right_q) {
		FCURC(F) = frmt->right_q;
		F->idx++;
		(*start_p)++;
		if (F->idx >= F->size) {
			s = bucket_file_write(F);
			if (s)
				return s;
		}
	}

	/* set end position */
	if (frmt->end_p) {
		while (len < len2) {
			FCURC(F) = ' ';
			F->idx++;
			len++;
			(*start_p)++;
			if (F->idx >= F->size) {
				s = bucket_file_write(F);
				if (s)
					return s;
			}
		}
	}
	/* write separator */
	if (frmt->sep) {
		FCURC(F) = frmt->sep;
		F->idx++;
		(*start_p)++;
		if (F->idx >= F->size) {
			s = bucket_file_write(F);
			if (s)
				return s;
		}
	}

	return 0;
}

int bucket_write(struct Bucket *B, const int n, struct File *F,
			struct Field *flds)
{
	int		s	= 0;
	int		i	= 0;
	int		empty	= 1;
	int		start_p	= 0;
	struct Field	*pfld	= 0;
	struct String	snon	= {0, 0, "\0"};
	struct Record	non	= {0, 0, &snon, 0, 0, 0, 0};

	for (i = 1; i < n; i++) {
		if (! B[i].cnt) {
			B[i].stat = 1;
			empty++;
		} else if (B[i].p == B[i].cnt) { /* p does not move */
			B[i].stat = 1;
			empty++;
		} else {
			B[i].stat = 0;
			B[i].p = B[i].cnt;
		}
	}

	while (empty < n) {
		pfld = flds;
		for (i = 1; i < n; i++) {
			if (! B[i].stat) {
				s = record_write_once(B[i].p, F, pfld,
							&start_p);
				if (s)
					goto err;

				B[i].p = B[i].p->row_next;
				if (B[i].p == B[i].cnt->row_last
				|| (! B[i].p)) {
					empty++;
					B[i].stat		= 1;
					B[i].p			= B[i].cnt;
					B[i].cnt->row_last	= 0;
				}
			} else {
				s = record_write_once(&non, F, pfld,
							&start_p);
			}
			pfld = pfld->next;
		}
		if (F->idx >= F->size) {
			s = bucket_file_write(F);
			if (s)
				goto err;
		}
		FCURC(F) = CH_NEWLINE;
		F->idx++;
		start_p	= 0;
	}
	file_write(F);
err:
	return s;
}

/**
 * @return:
 *	< -1	: rejected
 *	< 0	: success, accepted
 *	< > 0	: fail
 */
int bucket_read_filtered(struct Bucket *B, struct File *F, struct Field *_fld)
{	
	int		len	= 0;
	int		flen	= 0;
	int		s	= 0;
	int		reject	= 0;
	long int	start_p	= 0;
	struct String	*str	= 0;
	struct Field	*fld	= _fld;

	if (F->idx >= F->size) {
		s = file_read(F);
		if (s)
			return s;
	}

	while (fld) {
		if (fld->flag && ! reject)
			str = B[fld->idx].p->v;

		/* where do we start ? */
		if (fld->start_p) {
			if (start_p < fld->start_p) {
				len	= fld->start_p - start_p;
				start_p	+= len;
				flen	= F->idx + len;
				if (flen > F->size) {
					len = flen - F->size;
					s = file_read(F);
					if (s)
						goto err;
				} 
				F->idx += len;
			}
			if (fld->left_q) {
				if (FCURC(F) == fld->left_q) {
					F->idx++;
					start_p++;

					if (F->idx >= F->size) {
						s = file_read(F);
						if (s)
							goto err;
					}
				}
			}
		} else if (fld->left_q) {
			if (FCURC(F) == fld->left_q) {
				F->idx++;
				start_p++;
				if (F->idx >= F->size) {
					s = file_read(F);
					if (s)
						goto err;
				}
			} else
				goto err;
		}

		if (fld->end_p) {
			while (start_p <= fld->end_p) {
				if (fld->flag && ! reject)
					str_append_c(str, FCURC(F));

				F->idx++;
				start_p++;

				if (F->idx >= F->size) {
					s = file_read(F);
					if (s)
						goto err;
				}
			}
		} else if (fld->right_q) {
			if (fld->flag && ! reject) {
				s = file_fetch_until(F, str, fld->right_q);
				start_p += str->idx + 1;
			} else {
				s = file_skip_until(F, fld->right_q);
			}
			if (s)
				goto err;

			F->idx++;
			if (F->idx >= F->size) {
				s = file_read(F);
				if (s)
					goto err;
			}

			if (fld->sep) {
				while (FCURC(F) != fld->sep) {
					F->idx++;
					start_p++;
					if (F->idx >= F->size) {
						s = file_read(F);
						if (s)
							goto err;
					}
				}
				F->idx++;
				start_p++;

				if (F->idx >= F->size) {
					s = file_read(F);
					if (s)
						goto err;
				}
			}
		} else if (fld->sep) {
			if (fld->flag && !reject) {
				s = file_fetch_until(F, str, fld->sep);
				start_p += str->idx + 1;
			} else
				s = file_skip_until(F, fld->sep);
			if (s)
				goto err;

			F->idx++;
			if (F->idx >= F->size) {
				s = file_read(F);
				if (s)
					goto err;
			}
		} else {
			if (fld->flag && !reject)
				s = file_fetch_until(F, str, CH_NEWLINE);
			else
				s = file_skip_until(F, CH_NEWLINE);
			if (s)
				goto err;
		}

		if ((fld->flag & FFLAG_FILTER) && !reject) {
			s = fld->fop(fld->fltr_rule, str->buf,
						fld->fltr_v);
			/* reject this record */
			if (s == 0) {
				reject = 1;
				break;
				/* prune later */
			}
		}

		fld = fld->next;
	}
	if (FCURC(F) != CH_NEWLINE) {
		s = file_skip_until(F, CH_NEWLINE);
	}
	F->idx++;

	fld = _fld;
	if (reject) {
		/* since we don't know in which field cause the row being
		 * rejected, we just reset all field content here */
		s = -1;
		while (fld) {
			str_prune(B[fld->idx].p->v);
			fld = fld->next;
		}
	} else {
		s = 0;
		/* move `p' to the next row in the bucket */
		while (fld) {
			if (B[fld->idx].p)
				B[fld->idx].p = B[fld->idx].p->row_next;
			fld = fld->next;
		}
	}
err:
	return s;
}
