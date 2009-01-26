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

struct Stmt * stmt_find_by_name(struct Stmt *stmt, const char *name)
{
	int		s	= 0;
	struct Stmt	*p	= stmt->last;

	while (p) {
		switch (p->type) {
		case STMT_LOAD:
			s = strcasecmp(p->in->filename, name);
			if (s == 0)
				return p;

			if (p->in->alias) {
				s = strcasecmp(p->in->alias, name);
				if (s == 0)
					return p;
			}
			break;

		case STMT_SORT:
			s = strcasecmp(p->in->filename, name);
			if (s == 0)
				return p;

			if (p->in->alias) {
				s = strcasecmp(p->in->alias, name);
				if (s == 0)
					return p;
			}
			break;

		case STMT_CREATE:
			s = strcasecmp(p->out->filename, name);
			if (s == 0)
				return p;

			if (p->out->alias) {
				s = strcasecmp(p->out->alias, name);
				if (s == 0)
					return p;
			}
			break;

		case STMT_JOIN:
			s = strcasecmp(p->out->filename, name);
			if (s == 0)
				return p;

			if (p->out->alias) {
				s = strcasecmp(p->out->alias, name);
				if (s == 0)
					return p;
			}
			break;
		}
		p = p->prev;
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
