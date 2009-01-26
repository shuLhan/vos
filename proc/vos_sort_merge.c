#include "proc/vos_sort_merge.h"

static int mnode_new(struct MNode **node, const int level, const char *file)
{
	int s = 0;

	(*node) = (struct MNode *) calloc(1, sizeof(struct MNode));
	if (! (*node))
		return E_MEM;

	s = file_open(&(*node)->file, file, FOPEN_RO);
	if (s)
		goto err;

	s = file_read((*node)->file);
	if (s)
		goto err;

	(*node)->level = level;
	s = 0;
err:
	if (s && (*node)) {
		if ((*node)->file)
			file_close(&(*node)->file);
		free((*node));
	}
	return s;
}

static void mnode_destroy(struct MNode **node)
{
	if (! (*node))
		return;
	if ((*node)->file)
		file_close(&(*node)->file);
	if ((*node)->row)
		record_destroy(&(*node)->row);
	mnode_destroy(&(*node)->left);
	mnode_destroy(&(*node)->right);
	free((*node));
	(*node) = 0;
}

static void mnode_print(struct MNode *root)
{
	if (root->left)
		mnode_print(root->left);
	record_print(root->row);
	if (root->right)
		mnode_print(root->right);
}

static struct MNode * mnode_insert(struct MNode *root, struct MNode *node)
{
	int s = 0;

	if (! root)
		return node;

	s = record_cmp(root->row, node->row);
	if (s < 0) {
		root->right = mnode_insert(root->right, node);
	} else if (s == 0) {
		if (root->level < node->level)
			root->right = mnode_insert(root->right, node);
		else
			root->left = mnode_insert(root->left, node);
	} else {
		root->left = mnode_insert(root->left, node);
	}

	return root;
}

static int merge_init(struct MNode **root, struct LL *lsfile,
			struct Field *fld)
{
	int		s	= 0;
	int		level	= 1;
	struct MNode	*node	= 0;

	while (lsfile) {
		s = mnode_new(&node, level, lsfile->str);
		if (s)
			return s;

		s = record_read(&node->row, node->file, fld);
		if (! node->row)
			return s;

		(*root) = mnode_insert((*root), node);

		level++;
		lsfile = lsfile->next;
	}

	return 0;
}

static struct MNode * mnode_get_loser(struct MNode **root,
					struct MNode **loser2)
{
	struct MNode *loser = (*root);

	while (loser->left) {
		(*loser2)	= loser;
		loser		= loser->left;
	}

	if (loser == (*root)) {
		(*root)		= (*root)->right;
		(*loser2)	= (*root);
	} else {
		(*loser2)->left = loser->right;
	}

	if ((*loser2)) {
		while ((*loser2)->left)
			(*loser2) = (*loser2)->left;
	}

	loser->right = 0;
	return loser;
}

int vos_sort_merge(struct Stmt *sort, struct LL *lsfile,
			struct Record **_all_rows, unsigned long all_n_row)
{
	int		s	= 0;
	unsigned long	n_row	= 0;
	struct File	*F	= 0;
	struct MNode	*root	= 0;
	struct MNode	*loser	= 0;
	struct MNode	*loser2	= 0;
	struct Record	*rows	= 0;
	struct Record	*R	= 0;
	struct Record	*all_rows = (*_all_rows);

	/* in case of only one file to merge */
	if (! lsfile->next) {
		s = rename(lsfile->str, sort->out->filename);
		return s;
	}

	s = file_open(&F, sort->out->filename, FOPEN_WO);
	if (s)
		goto err;

	s = merge_init(&root, lsfile, sort->out->fields);
	if (s)
		goto err;

	R		= all_rows;
	all_rows	= R->row_next;
	R->row_next	= 0;
	R->row_last	= 0;
	while (root) {
		loser2 = 0;
		loser = mnode_get_loser(&root, &loser2);

		if (loser2) {
			do {
				record_add_row(&rows, loser->row);
				n_row++;
				if (n_row >= all_n_row || ! all_rows) {
					s = record_write(rows, F,
							sort->out->fields);
					if (s)
						goto err;

					all_rows		= rows;
					all_rows->row_last	= 0;
					rows			= 0;
					n_row			= 0;
				}

				loser->row	= R;
				R		= all_rows;
				all_rows	= R->row_next;
				R->row_next	= 0;
				R->row_last	= 0;

				s = record_read2(loser->row, loser->file,
							sort->out->fields);
				if (s)
					break;

				s = record_cmp(loser2->row, loser->row);
			} while (s > 0
			|| (s == 0 && loser2->level > loser->level));
		} else {
			do {
				record_add_row(&rows, loser->row);
				n_row++;
				if (n_row >= all_n_row || ! all_rows) {
					s = record_write(rows, F,
							sort->out->fields);
					if (s)
						goto err;

					all_rows		= rows;
					all_rows->row_last	= 0;
					rows			= 0;
					n_row			= 0;
				}

				loser->row	= R;
				R		= all_rows;
				all_rows	= R->row_next;
				R->row_next	= 0;
				R->row_last	= 0;

				s = record_read2(loser->row, loser->file,
							sort->out->fields);
			} while (s == 0);
		}
		if (s == E_FILE_END)
			mnode_destroy(&loser);
		else
			root = mnode_insert(root, loser);
	}

	R->row_next = all_rows;
	if (n_row) {
		s = record_write(rows, F, sort->out->fields);
		if (s)
			goto err;

		rows->row_last->row_next = R;
		(*_all_rows) = rows;
	} else {
		(*_all_rows) = R;
	}

	file_write(F);
	s = 0;
err:
	mnode_destroy(&root);
	file_close(&F);
	return s;
}
