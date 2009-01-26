/***
 * @desc: filter behavior: return 1 if true or 0 if false.
 *
 *	if v operation with fltr is true
 *		return rule
 *	return !rule
 *
 * the return value is checked by filter caller,
 * i.e by vos_Record.record_read()
 *
 *	if (fltr(rule, v, fltr))
 *		accept this record
 *	else
 *		reject this record
 */
#include "op/vos_Filter.h"

const char *_filter_op[N_FLTR_OP] = {
	"=\0",
	"==\0",
	"!=\0",
	"!==\0",
	">\0",
	"<\0",
	">=\0",
	"<=\0"
};

const char *_filter_rule[N_FLTR_RULE] = {
	"REJECT\0",
	"ACCEPT\0"
};

const void (*_filter_f[N_FLTR_OP]) = {
	fltr_eq,
	fltr_eqeq,
	fltr_neq,
	fltr_neqeq,
	fltr_more,
	fltr_less,
	fltr_meq,
	fltr_leq
};

/**
 * @desc:
 *	case not sensitive
 * @return:
 *	< 1	: true
 *	< 0	: false
 */
int fltr_eq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s == 0)
		return rule;
	return !rule;
}

/**
 * @desc:
 *	case sensitive
 * @return:
 *	< 1	: true
 *	< 0	: false
 */
int fltr_eqeq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcmp(v, fltr);
	if (0 == s)
		return rule;
	return !rule;
}

int fltr_neq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s != 0)
		return rule;
	return !rule;
}

int fltr_neqeq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcmp(v, fltr);
	if (s != 0)
		return rule;
	return !rule;
}

int fltr_more(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s > 0)
		return rule;
	return !rule;
}

int fltr_less(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s < 0)
		return rule;
	return !rule;
}

int fltr_meq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s >= 0)
		return rule;
	return !rule;
}

int fltr_leq(const int rule, const char *v, const char *fltr)
{
	int s;
	s = strcasecmp(v, fltr);
	if (s < 0)
		return rule;
	return !rule;
}
