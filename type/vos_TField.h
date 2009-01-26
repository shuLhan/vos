#ifndef _VOS_TYPE_FIELD_H
#define	_VOS_TYPE_FIELD_H	1

enum _field_type_idx {
	FT_STRING	= 0,
	FT_NUMBER,
	FT_DATETIME,
	N_FIELD_TYPE
};

enum _fflag_sort_idx {
	FFLAG_SORT_ASC	= 1,
	FFLAG_SORT_DESC	= 2,
	N_FFLAG_SORT
};

enum _fflag_idx {
	FFLAG_CREATE	= 4,
	FFLAG_FILTER	= 8,
	FFLAG_JOIN	= 16,
	N_FFLAG
};

struct Field {
	int		idx;
	int		flag;
	int		type;
	int		left_q;
	int		right_q;
	int		start_p;
	int		end_p;
	int		sep;
	int		fltr_idx;
	int		fltr_rule;
	int		(*fop)(const int, const void *, const void *);
	char		*name;
	char		*date_format;
	char		*fltr_v;
	struct Field	*next;
};

#endif
