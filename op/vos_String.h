#ifndef _VOS_STRING_H
#define	_VOS_STRING_H	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vos_errno.h"
#include "type/vos_TString.h"

#define	CH_NEWLINE	'\n'
#define	CH_2_REPLACE	'X'

#define	STR_DEF_LEN		15
#define	STR_DEF_REALLOC_LEN	15
#define	N_ALNUM			62

int str_create(struct String **str);
int str_append_c(struct String *str, const int c);
int str_append(struct String *S, const char *str);
int str_detach(struct String *str, char **buf);
void str_prune(struct String *str);
void str_destroy(struct String **str);

int str_raw_copy(const char *str, char **buf);
int str_raw_randomize(const char *format, char **buf);
unsigned long str_raw_hash(char *str, unsigned long hash);

#endif
