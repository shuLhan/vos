#ifndef _VOS_LL_H
#define	_VOS_LL_H	1

#include "type/vos_TLL.h"
#include "op/vos_String.h"

int ll_add(struct LL **tok, unsigned int num, const char *str);
void ll_link(struct LL **l, struct LL *r);
void ll_print(struct LL *tok);
void ll_destroy(struct LL **tok);

#endif
