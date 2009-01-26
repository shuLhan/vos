#include "op/vos_StmtCreate.h"

int stmtcreate_create(struct Stmt **create)
{
	(*create) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*create))
		return E_MEM;

	(*create)->out = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*create)->out) {
		free((*create));
		(*create) = 0;
		return E_MEM;
	}
	(*create)->type = STMT_CREATE;

	return 0;
}

void stmtcreate_print(struct Stmt *create)
{
	int		coma	= 0;
	struct StmtMeta	*in	= 0;

	if (! create)
		return;

	printf("CREATE %s \nFROM ", create->out->filename);

	in = create->in;
	while (in) {
		if (coma)
			printf(",\n");
		printf(" %s \n", in->filename);
		field_print(in->fields);
		coma++;
		in = in->next;
	}
	printf("\n");

	field_print(create->out->fields);

	if (create->out->alias)
		printf("AS %s;\n\n", create->out->alias);
	else
		printf(";\n\n");
}

void stmtcreate_destroy(struct Stmt **create)
{
	if (! (*create))
		return;

	stmtmeta_soft_destroy(&(*create)->in);
	stmtmeta_destroy(&(*create)->out);
	free((*create));
	(*create) = 0;
}
