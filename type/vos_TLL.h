#ifndef _VOS_TYPE_LL_H
#define	_VOS_TYPE_LL_H	1

struct LL {
	unsigned long	num;
	char		*str;
	struct LL	*next;
	struct LL	*last;
};

#endif
