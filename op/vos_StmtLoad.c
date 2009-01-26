#include "op/vos_StmtLoad.h"

int stmtload_create(struct Stmt **load)
{
	(*load) = (struct Stmt *) calloc(1, sizeof(struct Stmt));
	if (! (*load))
		return E_MEM;

	(*load)->in = (struct StmtMeta *) calloc(1, sizeof(struct StmtMeta));
	if (! (*load)->in) {
		free((*load));
		(*load) = 0;
		return E_MEM;
	}
	(*load)->type = STMT_LOAD;
	return 0;
}

void stmtload_print(struct Stmt *load)
{
	if (! load && !load->in)
		return;

	printf("LOAD \"%s\" \n", load->in->filename);
	field_print(load->in->fields);
	if (load->in->alias)
		printf("AS %s;\n\n", load->in->alias);
	else
		printf(";\n\n");
}

void stmtload_destroy(struct Stmt **load)
{
	if (! (*load))
		return;

	if ((*load)->in)
		stmtmeta_destroy(&(*load)->in);
	free((*load));
	(*load) = 0;
}
