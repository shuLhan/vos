#include "proc/vos_set.h"

static int set_proc_tmp_dir_value(char *tmp)
{
	int		idx	= 0;
	int		s	= 0;
	struct String	*path	= 0;

	s = str_create(&path);
	if (s)
		return s;

	if (*tmp == ':') {
		tmp++;
		idx = _vos.proc_tmp_dir->last->num;
	} else
		ll_destroy(&_vos.proc_tmp_dir);

	while (*tmp) {
		/* skip '"' */
		tmp++;

		while(*tmp && *tmp != '"') {
			s = str_append_c(path, *tmp);
			if (s)
				return s;
			tmp++;
		}

		/* does user have write access ? */
		s = access(path->buf, W_OK);
		if (s == 0) {
			idx++;
			s = ll_add(&_vos.proc_tmp_dir, idx, path->buf);
			if (s)
				return s;
		}

		/* skip '"' */
		tmp++;

		/* skip ':' */
		if (*tmp && *tmp == ':')
			tmp++;
		else
			break;

		str_prune(path);
	}

	_vos.p_proc_tmp_dir = _vos.proc_tmp_dir;

	str_destroy(&path);

	return 0;
}

int vos_process_set(struct StmtSet *set)
{
	switch (set->var_idx) {
	case SET_FILE_BUFFER_SIZE:
		_vos.file_buf_size = strtoul(set->value, 0, 0);
		if (! _vos.file_buf_size)
			_vos.file_buf_size = VOS_DEF_FILE_BUF_SIZE;
		break;

	case SET_PROC_CMP_CASE_SENSITIVE:
		_vos.proc_cmp_case = CMP_CASE_SENSITIVE;
		break;

	case SET_PROC_CMP_CASE_NOTSENSITIVE:
		_vos.proc_cmp_case = CMP_CASE_NOTSENSITIVE;
		break;

	case SET_PROC_MAX:
		_vos.proc_max = strtol(set->value, 0, 0);
		if (! _vos.proc_max)
			_vos.proc_max = VOS_DEF_PROC_MAX;
		break;

	case SET_PROC_MAX_ROW:
		_vos.proc_max_row = strtoul(set->value, 0, 0);
		if (! _vos.proc_max_row)
			_vos.proc_max_row = VOS_DEF_PROC_MAX_ROW;
		break;

	case SET_PROC_TMP_DIR:
		set_proc_tmp_dir_value(set->value);
		break;
	}

	return 0;
}
