#include "op/vos_Record.h"

/**
 * @return:
 *	< 0	: success
 *	< E_MEM	: fail
 */
int record_new(struct Record **R, struct Field *fld, struct String *str)
{
	(*R) = (struct Record *) calloc(1, sizeof(struct Record));
	if (! (*R))
		return E_MEM;

	(*R)->v		= str;
	(*R)->idx	= fld->idx;
	(*R)->flag	= fld->flag;

	return 0;
}

/**
 * @return:
 *	< -1	: l < r
 *	< 0	: l == r
 *	< 1	: l > r
 */
int _record_cmp(struct Record *l, struct Record *r, const int idx)
{
	int		s	= 0;
	struct Record	*pl	= l;
	struct Record	*pr	= r;

	if (! l && ! r)
		return 0;
	if (! l)
		return -1;
	if (! r)
		return 1;

	/* search for field index to compare */
	while (l && (l->idx != idx))
		l = l->fld_next;
	while (r && (r->idx != idx))
		r = r->fld_next;

	if (!l || !r)
		return 0;

	if (_vos.proc_cmp_case == CMP_CASE_SENSITIVE)
		s = strcmp(l->v->buf, r->v->buf);
	else
		s = strcasecmp(l->v->buf, r->v->buf);

	if (s == 0) {
		s = _record_cmp(pl, pr, idx + 1);
	} else {
		if (l->flag == FFLAG_SORT_DESC) {
			if (s < 0)
				return 1;
			if (s > 0)
				return -1;
		}
	}
	return s;
}

void record_add_field(struct Record **R, struct Record *Rfld)
{
	if (! Rfld)
		return;

	if (! (*R))
		(*R) = Rfld;
	else
		(*R)->fld_last->fld_next = Rfld;

	(*R)->fld_last = Rfld;
}

void record_add_row(struct Record **R, struct Record *row)
{
	if (! row)
		return;

	if (! (*R))
		(*R) = row;
	else
		(*R)->row_last->row_next = row;

	(*R)->row_last = row;
}

void record_prune(struct Record *R)
{
	while (R) {
		if (R->v)
			str_prune(R->v);
		R = R->fld_next;
	}
}

void record_destroy(struct Record **R)
{
	struct Record *Rfld = 0;
	struct Record *tmp = 0;

	while ((*R)) {
		Rfld = (*R)->fld_next;
		while (Rfld) {
			if (Rfld->v) {
				str_destroy(&Rfld->v);
			}
			tmp = Rfld->fld_next;
			free(Rfld);
			Rfld = tmp;
		}

		if ((*R)->v) {
			str_destroy(&(*R)->v);
			(*R)->v = 0;
		}
		tmp = (*R)->row_next;
		free((*R));
		(*R) = tmp;
	}

	(*R) = 0;
}

void record_print(struct Record *R)
{
	while (R) {
		printf("%s|", R->v->buf);
		R = R->fld_next;
	}
	printf("\n");
}

int record_write(struct Record *R, struct File *F, struct Field *_frmt)
{
	int		s	= 0;
	int		start_p	= 0;
	int		len	= 0;
	int		len2	= 0;
	struct Record	*Rfld	= 0;
	struct Field	*frmt	= 0;

	while (R) {
		start_p	= F->idx;
		Rfld	= R;
		frmt	= _frmt;
		while (Rfld) {
			/* set start position */
			if (frmt->start_p) {
				len = start_p + frmt->start_p;
				if (len < F->idx) {
					while (F->idx > len) {
						FCURC(F) = ' ';
						F->idx--;
					}
				} else {
					F->idx = start_p + len;
					if (F->idx >= F->size) {
						s = file_write(F);
						if (s)
							return s;
					}
				}
			}

			/* write left quote */
			if (frmt->left_q) {
				FCURC(F) = frmt->left_q;
				if (++F->idx >= F->size) {
					s = file_write(F);
					if (s)
						return s;
				}
			}

			/* write content of Record */
			len = Rfld->v->idx;
			if (frmt->end_p) {
				len2 = frmt->end_p - frmt->start_p + 1;
				if (frmt->left_q)
					len2--;
				if (frmt->right_q)
					len2--;
				if (len2 < len)
					len = len2;
			}
			if ((F->idx + len) >= F->size) {
				s = file_write(F);
				if (s)
					return s;
			}
			if (len > 0) {
				memcpy(&FCURC(F), Rfld->v->buf, len);
				F->idx += len;
				str_prune(Rfld->v);
			}

			/* write right quote */
			if (frmt->right_q) {
				FCURC(F) = frmt->right_q;
				F->idx++;
				if (F->idx >= F->size) {
					s = file_write(F);
					if (s)
						return s;
				}
			}

			/* set end position */
			if (frmt->end_p) {
				while (len <= len2) {
					FCURC(F) = ' ';
					F->idx++;
					len++;
					if (F->idx >= F->size) {
						s = file_write(F);
						if (s)
							return s;
					}
				}
			}

			/* write separator */
			if (frmt->sep) {
				FCURC(F) = frmt->sep;
				F->idx++;
				if (F->idx >= F->size) {
					s = file_write(F);
					if (s)
						return s;
				}
			}

			frmt	= frmt->next;
			Rfld	= Rfld->fld_next;
		}
		if (F->idx >= F->size) {
			s = file_write(F);
			if (s)
				goto err;
		}
		FCURC(F) = CH_NEWLINE;
		F->idx++;

		R = R->row_next;
	}
err:
	return s;
}

/**
 * @desc: touch if you dare!
 *	priority in reading a record:
 *		- start_p > left_q
 *		- end_p > right_q > sep
 * @return:
 *	< 0	: success
 *	< !0	: fail. and R will be NULL
 */
int record_read(struct Record **R, struct File *F, struct Field *fld)
{
	int		s	= 0;
	int		len	= 0;
	int		flen	= 0;
	long int	start_p	= 0;
	struct String	*str	= 0;
	struct Record	*Rfld	= 0;

	if (F->idx >= F->size) {
		s = file_read(F);
		if (s)
			return s;
	}

	(*R) = 0;
	while (fld) {
		/* where do we start ? */
		if (fld->start_p) {
			if (start_p < fld->start_p) {
				len	= fld->start_p - start_p;
				start_p	+= len;
				flen	= F->idx + len;
				if (flen >= F->size) {
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

		/* create string for field data */
		s = str_create(&str);
		if (s)
			return s;

		if (fld->end_p) {
			while (start_p <= fld->end_p) {
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
			s = file_fetch_until(F, str, fld->right_q);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;

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
			s = file_fetch_until(F, str, fld->sep);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;

			if (F->idx >= F->size) {
				s = file_read(F);
				if (s)
					goto err;
			}
		} else {
			s = file_fetch_until(F, str, CH_NEWLINE);
			if (s)
				goto err;
		}

		s = record_new(&Rfld, fld, str);
		if (s)
			goto err;

		record_add_field(R, Rfld);
		str	= 0;
		Rfld	= 0;
		fld	= fld->next;
	}
	/* go to next row */
	if (FCURC(F) != CH_NEWLINE) {
		s = file_fetch_until(F, str, CH_NEWLINE);
	}
	F->idx++;
	return 0;
err:
	str_destroy(&str);
	if (s && s != E_FILE_END)
		record_destroy(R);
	return s;
}

/**
 * @return:
 *	< 0	: success
 *	< > 0	: fail
 */
int record_read2(struct Record *R, struct File *F, struct Field *fld)
{
	int		s	= 0;
	int		len	= 0;
	int		flen	= 0;
	long int	start_p	= 0;
	struct String	*str	= 0;

	if (F->idx >= F->size) {
		s = file_read(F);
		if (s)
			return s;
	}

	while (fld) {
		str = R->v;
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
			s = file_fetch_until(F, str, fld->right_q);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;

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
			s = file_fetch_until(F, str, fld->sep);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;
	
			if (F->idx >= F->size) {
				s = file_read(F);
				if (s)
					goto err;
			}
		} else {
			s = file_fetch_until(F, str, CH_NEWLINE);
			if (s)
				goto err;
		}

		fld = fld->next;
		R = R->fld_next;
	}
	/* go to next row */
	if (FCURC(F) != CH_NEWLINE) {
		file_fetch_until(F, str, CH_NEWLINE);
	}
	F->idx++;
	s = 0;
err:
	return s;
}

int record_read_filtered(struct Record **R, struct File *F, struct Field *fld)
{	
	int		len	= 0;
	int		flen	= 0;
	int		s	= 0;
	int		reject	= 0;
	long int	start_p	= 0;
	struct String	*str	= 0;
	struct Record	*Rfld	= 0;

	if (F->idx >= F->size) {
		s = file_read(F);
		if (s)
			return s;
	}

	(*R) = 0;
	while (fld) {
		/* where do we start ? */
		if (fld->start_p) {
			if (start_p < fld->start_p) {
				len	= fld->start_p - start_p;
				start_p	+= len;
				flen	= F->idx + len;
				if (flen >= F->size) {
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

		if (! str) {
			s = str_create(&str);
			if (s)
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
			if (fld->flag && ! reject)
				s = file_fetch_until(F, str, fld->right_q);
			else
				s = file_skip_until(F, fld->right_q);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;

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
			if (fld->flag && !reject)
				s = file_fetch_until(F, str, fld->sep);
			else
				s = file_skip_until(F, fld->sep);
			if (s)
				goto err;

			F->idx++;
			start_p += str->idx + 1;

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
				str_prune(str);
			}
		}

		if (fld->flag & FFLAG_CREATE) {
			if (! reject) {
				s = record_new(&Rfld, fld, str);
				if (s)
					goto err;

				record_add_field(R, Rfld);
				str	= 0;
				Rfld	= 0;
			}
		} else
			str_prune(str);

		fld = fld->next;
	}
	/* go to next row */
	if (FCURC(F) != CH_NEWLINE) {
		s = file_fetch_until(F, str, CH_NEWLINE);
	}
	F->idx++;
	s = 0;
err:
	str_destroy(&str);
	if ((s && s != E_FILE_END) || reject)
		record_destroy(R);
	return s;
}
