#include "proc/vos_parser.h"
#include "proc/vos_set.h"
#include "proc/vos_load.h"
#include "proc/vos_sort.h"
#include "proc/vos_create.h"
#include "proc/vos_join.h"

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

/**
 * @desc: get one of the temporary directory from the list of temporary
 *	directories.
 *
 * @return:
 *	< tmp_dir : a path to random temporary directory.
 */
char *get_tmp_dir(const int lock)
{
	int	s;
	char	*tmp_dir = 0;

	if (_vos.proc_max > 1 && lock) {
		do {
			s = pthread_mutex_trylock(&_vos.proc_tmp_dir_lock);
		} while (s);
	}

	tmp_dir = _vos.p_proc_tmp_dir->str;

	/* to the next temporary dir */
	_vos.p_proc_tmp_dir = _vos.p_proc_tmp_dir->next;
	if (! _vos.p_proc_tmp_dir)
		_vos.p_proc_tmp_dir = _vos.proc_tmp_dir;

	if (_vos.proc_max > 1 && lock) {
		pthread_mutex_unlock(&_vos.proc_tmp_dir_lock);
	}

	return tmp_dir;
}

void sys_getenv_num(unsigned long *env_value, const char *env_name,
			unsigned long def_value)
{
	const char *env = getenv(env_name);

	if (env)
		*env_value = strtol(env, 0, 0);
	else
		*env_value = def_value;
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

/**
 * @return:
 *	< 0		: success.
 *	< E_VOS_PARAM	: fail.
 */
static int vos_init(int argc, char **argv)
{
	int	s;
	char	*env;

	if (argc < 2 || argc > 2)
		return E_VOS_PARAM;

	pthread_mutex_init(&_vos.proc_tmp_dir_lock, 0);

	/* get Vos variable value from system environment */
	sys_getenv_num((unsigned long *) &_vos.debug, VOS_ENV_DEBUG,
						VOS_DEF_DEBUG_VALUE);

	sys_getenv_num(&_vos.file_buf_size, VOS_ENV_FILE_BUF_SIZE,
						VOS_DEF_FILE_BUF_SIZE);

	sys_getenv_num((unsigned long *) &_vos.proc_cmp_case,
						VOS_ENV_PROC_CMP_CASE,
						VOS_DEF_PROC_CMP_CASE);

	sys_getenv_num((unsigned long *) &_vos.proc_max, VOS_ENV_PROC_MAX,
						VOS_DEF_PROC_MAX);

	sys_getenv_num(&_vos.proc_max_row, VOS_ENV_PROC_MAX_ROW,
						VOS_DEF_PROC_MAX_ROW);

	/* get temporary dir from system environment */
	env = getenv(VOS_ENV_TMP_DIR);
	if (env)
		set_proc_tmp_dir_value(env);
	else {
		s = ll_add(&_vos.proc_tmp_dir, 1, VOS_DEF_PROC_TMP_DIR);
		if (s)
			return s;
	}

	_vos.p_proc_tmp_dir = _vos.proc_tmp_dir;

	_vos.script = argv[1];

	srand(time((void *) 0));

	if (_vos.debug)
		vos_print();

	return 0;
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

	s = vos_init(argc, argv);
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
