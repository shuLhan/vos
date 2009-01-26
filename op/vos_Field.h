#ifndef _VOS_FIELD_H
#define	_VOS_FIELD_H	1

#include <strings.h>
#include "type/vos_TField.h"
#include "op/vos_String.h"
#include "op/vos_Filter.h"
#include "vos.h"

extern const char *_field_type[N_FIELD_TYPE];
extern const char *_fflag_sort[N_FFLAG_SORT];

#define	field_get_type_idx(T)	get_token_idx(_field_type, N_FIELD_TYPE, T)

int field_soft_copy(struct Field *fld, struct Field **cp);
void field_add(struct Field **fields, struct Field *f);
void field_print(struct Field *fields);
void _field_destroy(struct Field **fields, const int soft);

#define	field_destroy(F)	_field_destroy((F),0)
#define	field_soft_destroy(F)	_field_destroy((F),1)

#endif
