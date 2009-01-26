#include "proc/vos_set.h"

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
	}
	return 0;
}
