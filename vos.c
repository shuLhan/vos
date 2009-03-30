#include "proc/vos_parser.h"
#include "proc/vos_set.h"
#include "proc/vos_load.h"
#include "proc/vos_sort.h"
#include "proc/vos_create.h"
#include "proc/vos_join.h"

enum _arg_todo {
	ARG_DONE	= 0,
	ARG_START,
	ARG_DEBUG_VALUE,
	ARG_VOS_SCRIPT
};

/**
 * @return:
 *	< 0..(n-1)	: success
 *	< -1		: fail
 */
int get_token_idx(const char **ls, const unsigned int n, const char *tok)
{
	int i = n - 1;
	int s = 0;

	if (! (*ls) || ! tok)
		return -1;

	for (; i >= 0; i--) {
		s = strcasecmp(ls[i], tok);
		if (s == 0)
			return i;
	}

	return i;
}

static int vos_init(int argc, char **argv)
{
	int i = 1;
	int s = ARG_START;

	_vos.debug = 0;
	while (i <= argc && s != ARG_DONE) {
		switch (s) {
		case ARG_START:
			s = strcmp(argv[i], "-h");
			if (s == 0)
				return E_VOS_PARAM;

			s = strcmp(argv[i], "-d");
			if (s == 0) {
				s = ARG_DEBUG_VALUE;
				i++;
			} else
				s = ARG_VOS_SCRIPT;
			break;

		case ARG_DEBUG_VALUE:
			_vos.debug = strtol(argv[i], 0, 0);
			s = ARG_VOS_SCRIPT;
			i++;
			break;

		case ARG_VOS_SCRIPT:
			_vos.script = argv[i];
			s = ARG_DONE;
			break;
		}
	}
	if (s != ARG_DONE)
		return E_VOS_PARAM;

	s = ll_add(&_vos.proc_tmp_dir, 1, VOS_DEF_PROC_TMP_DIR);
	if (s)
		return s;

	pthread_mutex_init(&_vos.proc_tmp_dir_lock, 0);
	_vos.p_proc_tmp_dir	= _vos.proc_tmp_dir;
	_vos.file_buf_size	= VOS_DEF_FILE_BUF_SIZE;
	_vos.proc_cmp_case	= VOS_DEF_PROC_CMP_CASE;
	_vos.proc_max		= VOS_DEF_PROC_MAX;
	_vos.proc_max_row	= VOS_DEF_PROC_MAX_ROW;

	srand(time((void *) 0));

	return 0;
}

static void vos_print()
{
	printf(" vos debug                : %d\n", _vos.debug);
	printf(" vos file buffer size     : %ld\n", _vos.file_buf_size);
	printf(" vos process compare case : %s\n", _vos.proc_cmp_case ?
							"not sensitive" :
							"sensitive");
	printf(" vos process maximum      : %d\n", _vos.proc_max);
	printf(" vos process maximum row  : %ld\n", _vos.proc_max_row);
	printf(" vos process temporary dir:\n");
	ll_print(_vos.proc_tmp_dir);
}

static int vos_process(struct Stmt *stmt)
{
	int s;

	while (stmt) {
		switch (stmt->type) {
		case STMT_SET:
			vos_process_set(stmt->set);

			if (_vos.debug & DBG_PARSER)
				vos_print();
			break;
		case STMT_LOAD:
			s = vos_process_load(stmt);
			if (s)
				return s;
			break;
		case STMT_SORT:
			s = vos_process_sort(stmt);
			if (s)
				return s;
			break;
		case STMT_CREATE:
			s = vos_process_create(stmt);
			if (s)
				return s;
			break;
		case STMT_JOIN:
			s = vos_process_join(stmt);
			if (s)
				return s;
			break;
		}
		stmt = stmt->next;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int		s;
	struct Stmt	*stmt	= 0;

	s = vos_init(argc - 1, argv);
	if (s)
		goto err;

	s = vos_parsing(&stmt, _vos.script);
	if (s)
		goto err;

	if (! (_vos.debug & DBG_SCRIPT))
		s = vos_process(stmt);

	if (! s)
		goto out;

err:
	switch (s) {
	case E_VOS_PARAM:
	case E_FILE_NOT_OPEN:
	case E_MEM:
		vos_error0(s);
		break;
	case E_FILE_OPEN:
	case E_FILE_EXIST:
	case E_FILE_NOT_EXIST:
	case E_FILE_SEEK:
	case E_FILE_READ:
	case E_FILE_WRITE:
		vos_error1(s, _vos.e_sparm0);
		break;
	case E_PARSER_INV_POS:
		vos_error1(s, _vos.e_nparm0);
		break;
	case E_PARSER_UNX_CHAR:
		vos_error2(s, _vos.e_nparm0, _vos.e_nparm1);
		break;
	case E_PARSER_UNX_TOKEN:
	case E_PARSER_UNK_TOKEN:
	case E_PARSER_UNK_FIELDNAME:
	case E_PARSER_INV_FIELDNAME:
	case E_PARSER_INV_VALUE:
	case E_PARSER_INV_STMT:
	case E_PARSER_AMB_FIELDNAME:
		vos_error2(s, _vos.e_nparm0, _vos.e_sparm0);
		break;
	case E_FILE_END:
		break;
	default:
		fprintf(stderr, "vos error number : %d\n", s);
	}

	if (_vos.e_sparm0)
		free(_vos.e_sparm0);
	if (_vos.e_sparm1)
		free(_vos.e_sparm1);
out:
	if (_vos.proc_tmp_dir)
		ll_destroy(&_vos.proc_tmp_dir);

	pthread_mutex_destroy(&_vos.proc_tmp_dir_lock);
	stmt_destroy(&stmt);

	return s;
}
