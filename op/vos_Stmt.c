#include "op/vos_Stmt.h"

const char *_stmt_type[N_STMT_TYPE] = {
	"SET\0",
	"LOAD\0",
	"CREATE\0",
	"SORT\0",
	"JOIN\0"
};

void stmt_add(struct Stmt **stmt, struct Stmt *new_stmt)
{
	if (! (*stmt)) {
		(*stmt)		= new_stmt;
		(*stmt)->next	= 0;
		(*stmt)->prev	= 0;
	} else {
		struct Stmt *p = (*stmt);

		while (p->next)
			p = p->next;

		p->next		= new_stmt;
		new_stmt->prev	= p;
	}
	(*stmt)->last = new_stmt;
}

struct Stmt * stmt_find_by_name(struct Stmt *p, const char *name)
{
	while (p) {
		switch (p->type) {
		case STMT_LOAD:
		case STMT_SORT:
			if (strcasecmp(p->in->filename, name) == 0)
				return p;

			if (strcasecmp(p->in->alias, name) == 0)
				return p;
			break;

		case STMT_CREATE:
		case STMT_JOIN:
			if (strcasecmp(p->out->filename, name) == 0)
				return p;

			if (strcasecmp(p->out->alias, name) == 0)
				return p;
			break;
		}
		p = p->prev;
	}

	return 0;
}

/**
 * @desc: update the filename in 'smeta' to point to sort output.
 *
 *	some statement use sort output as input, since sort output some time
 *	is not defined in script (without INTO clause) then we need to update
 *	the filename before processing.
 *
 * @return:
 *	< 0			: success.
 *	< E_FILE_NOT_EXIST	: fail, could not find file alias in 'stmt'.
 */
int stmt_update_meta(struct Stmt *stmt, struct StmtMeta *smeta)
{
	struct Stmt *p;

	if (smeta->filename)
		return 0;

	p = stmt_find_by_name(stmt, smeta->alias);
	if (! p) {
		str_raw_copy(smeta->alias, &_vos.e_sparm0);
		return E_FILE_NOT_EXIST;
	}

	if (p->type == STMT_SORT) {
		str_raw_copy(p->out->filename, &smeta->filename);
	}

	return 0;
}

void stmt_print(struct Stmt *stmt)
{
	while (stmt) {
		switch (stmt->type) {
		case STMT_SET:
			stmtset_print(stmt);
			break;
		case STMT_LOAD:
			stmtload_print(stmt);
			break;
		case STMT_SORT:
			stmtsort_print(stmt);
			break;
		case STMT_CREATE:
			stmtcreate_print(stmt);
			break;
		case STMT_JOIN:
			stmtjoin_print(stmt);
			break;
		}
		stmt = stmt->next;
	}
}

void stmt_destroy(struct Stmt **stmt)
{
	struct Stmt	*p	= (*stmt);
	struct Stmt	*pnext	= 0;

	while (p) {
		pnext = p->next;

		switch (p->type) {
		case STMT_SET:
			stmtset_destroy(&p);
			break;
		case STMT_LOAD:
			stmtload_destroy(&p);
			break;
		case STMT_SORT:
			stmtsort_destroy(&p);
			break;
		case STMT_CREATE:
			stmtcreate_destroy(&p);
			break;
		case STMT_JOIN:
			stmtjoin_destroy(&p);
			break;
		}
		p = pnext;
	}
	(*stmt) = 0;
}
