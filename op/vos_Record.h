#ifndef	_VOS_RECORD_H
#define	_VOS_RECORD_H	1

#include "type/vos_TRecord.h"
#include "op/vos_File.h"
#include "op/vos_Field.h"

int record_new(struct Record **R, struct Field *fld, struct String *str);
int _record_cmp(struct Record *l, struct Record *r, const int idx);
#define	record_cmp(L,R)	_record_cmp(L,R,1)

void record_add_field(struct Record **R, struct Record *Rfld);
void record_add_row(struct Record **R, struct Record *row);
void record_prune(struct Record *R);
void record_destroy(struct Record **R);
void record_print(struct Record *R);

int record_write(struct Record *R, struct File *F, struct Field *_fld);
int record_read(struct Record **R, struct File *F, struct Field *fld);
int record_read2(struct Record *R, struct File *F, struct Field *fld);
int record_read_filtered(struct Record **R, struct File *F,
				struct Field *fld);

#endif
