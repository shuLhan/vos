#ifndef _VOS_BUCKET_H
#define	_VOS_BUCKET_H	1

#include "type/vos_TBucket.h"
#include "op/vos_Record.h"

void bucket_print(struct Bucket *B, const int n);
void record_to_bucket(struct Bucket *B, struct Record *R);
void bucket_empty(struct Bucket *B, const int n);
void bucket_destroy(struct Bucket *B, const int n);

int bucket_write(struct Bucket *B, const int n, struct File *F,
			struct Field *flds);
int bucket_read_filtered(struct Bucket *B, struct File *F,
				struct Field *fld);

#endif
