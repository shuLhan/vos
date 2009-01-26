#include "op/vos_LL.h"

/**
 * @return:
 *	< 0	: success
 *	< E_MEM	: fail, out of memory
 */
int ll_add(struct LL **ll, unsigned int num, const char *str)
{
	int		s	= 0;
	struct LL	*wen	= 0;

	wen = (struct LL *) calloc(1, sizeof(struct LL));
	if (! wen)
		return E_MEM;

	s = str_raw_copy(str, &wen->str);
	if (s) {
		free(wen);
		return s;
	}

	wen->num = num;

	if (! (*ll))
		(*ll) = wen;
	else
		(*ll)->last->next = wen;

	(*ll)->last = wen;

	return 0;
}

void ll_link(struct LL **l, struct LL *r)
{
	if (! r)
		return;

	if (! (*l))
		(*l) = r;
	else
		(*l)->last->next = r;

	if (r->last) {
		(*l)->last = r->last;
		if ((*l)->last != r->last)
			r->last = 0;
	} else {
		while (r->next)
			r = r->next;
		(*l)->last = r;
	}
}

void ll_print(struct LL *ll)
{
	unsigned long n;

	while (ll) {
		n = ll->num;
		printf("%3lu ", n);
		while (ll && n == ll->num) {
			printf("%s ", ll->str);
			ll = ll->next;
		}
		printf("\n");
	}
}

void ll_destroy(struct LL **ll)
{
	struct LL *next = 0;

	while ((*ll)) {
		if ((*ll)->str)
			free((*ll)->str);
		next = (*ll)->next;
		free((*ll));
		(*ll) = next;
	}
}
