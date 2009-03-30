#include "op/vos_String.h"

const char _alnum[N_ALNUM] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',       /* 10 */
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',       /* 20 */
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',       /* 30 */
	'u', 'v', 'w', 'x', 'y', 'z',                           /* 36 */
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',       /* 46 */
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',       /* 56 */
	'U', 'V', 'W', 'X', 'Y', 'Z'                            /* 62 */
};

/**
 * @desc:
 *	create a new string object.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int str_create(struct String **str) 
{
	(*str) = (struct String *) calloc(1, sizeof(struct String));
	if (! (*str))
		return E_MEM;

	(*str)->buf = (char *) calloc(STR_DEF_LEN + 1, sizeof(char));
	if (! (*str)->buf) {
		free((*str));
		(*str) = 0;
		return E_MEM;
	}
	(*str)->len = STR_DEF_LEN;
	return 0;
}

/**
 * @desc:
 *	append character 'c' to the end of String buffer.
 * @return:
 *	< 0	: success.
 *	< E_MEM : fail, out of memory.
 */
int str_append_c(struct String *str, const int c)
{
	if (! str)
		return 0;
	if (str->idx == str->len) {
		str->len += STR_DEF_REALLOC_LEN;
		str->buf = realloc(str->buf, str->len + 1);
		if (! str->buf)
			return E_MEM;
	}
	str->buf[str->idx++]	= c;
	str->buf[str->idx]	= '\0';
	return 0;
}

/**
 * @desc:
 *	append raw string 'str' to the end of String buffer.
 * @return:
 *	< 0	: success.
 *	< E_MEM	: fail, out of memory.
 */
int str_append(struct String *S, const char *str)
{
	int len;

	if (! S)
		return 0;
	if (! str)
		return 0;

	len = strlen(str);
	if (S->idx + len >= S->len) {
		S->len += len;
		S->buf = realloc(S->buf, S->len + 1);
		if (! S->buf)
			return E_MEM;
	}

	memcpy(&S->buf[S->idx], str, len);

	S->idx		+= len;
	S->buf[S->idx]	= '\0';

	return 0;
}

/**
 * @desc:
 *	Attach String buffer to 'buf', and reset 'str' back to zero with
 *	length leave untouch.
 *	warning: buffer pointed by 'buf' will be lost.
 * @return:
 *	< 0	: success, or maybe str is empty.
 *	< E_MEM	: fail, out of memory.
 */
int str_detach(struct String *str, char **buf)
{
	if (! str)
		return 0;

	(*buf) = (char *) calloc(str->idx + 1, sizeof(char));
	if (! (*buf))
		return E_MEM;

	(*buf)		= memcpy((*buf), str->buf, str->idx);
	str->idx	= 0;
	str->buf[0]	= '\0';
	return 0;
}

/**
 * @desc: reset String buffer to zero
 */
void str_prune(struct String *str)
{
	if (! str)
		return;
	if (str->idx) {
		str->idx = 0;
		str->buf[0] = '\0';
	}
}

void str_destroy(struct String **str)
{
	if ((*str)) {
		if ((*str)->buf)
			free((*str)->buf);
		free((*str));
		(*str) = 0;
	}
}

/**
 * @desc:
 *	copy raw string 'str' into new 'buf'
 *	warning: buffer pointed by 'buf' will be lost.
 * @return:
 *	< 0	: success, or maybe str is empty.
 *	< E_MEM	: fail, out of memory.
 */
int str_raw_copy(const char *str, char **buf)
{
	size_t l;

	if (! str)
		return 0;

	if ((*buf))
		free((*buf));

	l	= strlen(str);
	(*buf)	= (char *) calloc(l + 1, sizeof(char));
	if (! (*buf))
		return E_MEM;

	(*buf) = memcpy((*buf), str, l);
	return 0;
}

/**
 * @desc:
 *	replace each 'X' character on 'format' with random alphanumeric
 *	character.
 * @return:
 *	< 0	: success, or maybe 'format' is empty.
 *	< E_MEM	: fail, out of memory.
 */
int str_raw_randomize(const char *format, char **buf)
{
	int i;

	if (! format)
		return 0;

	i	= strlen(format);
	(*buf)	= (char *) calloc(i + 1, sizeof(char));
	if (! (*buf))
		return E_MEM;

	i--;
	while (i >= 0) {
		if (format[i] == CH_2_REPLACE) {
			(*buf)[i] = _alnum[rand() % N_ALNUM];
		} else
			(*buf)[i] = format[i];
		i--;
	}
	return 0;
}

/**
 * @desc:
 *	generate a hash from raw string 'str'
 * @return:
 *	< hash : a hash value for string 'str'
 */
unsigned long str_raw_hash(char *str, unsigned long hash)
{
	while ((*str)) {
		hash = *str + (hash << 6) + (hash << 16) - hash;
		str++;
	}

	return hash;
}
