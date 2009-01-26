#include "op/vos_StmtSet.h"

const char *_stmt_set[N_SET] = {
	"FILE_BUFFER_SIZE\0",
	"PROCESS_COMPARE_CASE_SENSITIVE\0",
	"PROCESS_COMPARE_CASE_NOTSENSITIVE\0",
	"PROCESS_MAX\0",
	"PROCESS_MAX_ROW\0"
};

int stmtset_create(struct Stmt **stmt)
{
	(*stmt) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*stmt))
		return E_MEM;

	(*stmt)->set = (struct StmtSet *) calloc(1, sizeof(struct StmtSet));
	if (! (*stmt)->set) {
		free((*stmt));
		return E_MEM;
	}
	(*stmt)->type = STMT_SET;
	return 0;
}

void stmtset_print(struct Stmt *stmt)
{
	if (stmt && stmt->set)
		printf("SET %s %s;\n\n", _stmt_set[stmt->set->var_idx],
			stmt->set->value ? stmt->set->value : "\0");
}

void stmtset_destroy(struct Stmt **stmt)
{
	if (! (*stmt))
		return;

	if ((*stmt)->set) {
		if ((*stmt)->set->value)
			free((*stmt)->set->value);
		free((*stmt)->set);
		free((*stmt));
		(*stmt) = 0;
	}
}
