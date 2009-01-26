#ifndef _VOS_FILTER_H
#define	_VOS_FILTER_H	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type/vos_TFilter.h"

extern const char *_filter_op[N_FLTR_OP];
extern const char *_filter_rule[N_FLTR_RULE];
extern const void (*_filter_f[N_FLTR_OP]);

#define	op_get_idx(T)	get_token_idx(_filter_op,N_FLTR_OP,T)
#define	rule_get_idx(T)	get_token_idx(_filter_rule,N_FLTR_RULE,T)

int fltr_eq(const int rule, const char *v, const char *fltr);
int fltr_eqeq(const int rule, const char *v, const char *fltr);
int fltr_neq(const int rule, const char *v, const char *fltr);
int fltr_neqeq(const int rule, const char *v, const char *fltr);
int fltr_more(const int rule, const char *v, const char *fltr);
int fltr_less(const int rule, const char *v, const char *fltr);
int fltr_meq(const int rule, const char *v, const char *fltr);
int fltr_leq(const int rule, const char *v, const char *fltr);

#endif
