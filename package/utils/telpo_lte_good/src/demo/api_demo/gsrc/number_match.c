/*  
asterisk ÂåπÈÖçËßÑÂàô ÁöÑ Áõ∏ÂÖ≥ÂáΩÊï∞
Áõ∏ÂÖ≥Êñá‰ª∂Ôºöasterisk.c/h, pbx.c/h
*/

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  "number_match.h"
#define ARRAY_LEN(a) (size_t) (sizeof(a) / sizeof(0[a]))
#define ast_log() 
 int ast_strlen_zero(const char *s)
{
	return (!s || (*s == '\0'));
}

#if 1
#define ast_add_profile(a, b) 0
#define ast_profile(a, b) do { } while (0)
#define ast_mark(a, b) do { } while (0)

#else

struct profile_data {
	int entries;
	int max_size;
	struct profile_entry e[0];
};

static struct profile_data *prof_data;

/*! \brief allocates a counter with a given name and scale.
 * \return Returns the identifier of the counter.
 */
int ast_add_profile(const char *name, uint64_t scale)
{
	int l = sizeof(struct profile_data);
	int n = 10;	/* default entries */

	if (prof_data == NULL) {
		prof_data = ast_calloc(1, l + n*sizeof(struct profile_entry));
		if (prof_data == NULL)
			return -1;
		prof_data->entries = 0;
		prof_data->max_size = n;
	}
	if (prof_data->entries >= prof_data->max_size) {
		void *p;
		n = prof_data->max_size + 20;
		p = ast_realloc(prof_data, l + n*sizeof(struct profile_entry));
		if (p == NULL)
			return -1;
		prof_data = p;
		prof_data->max_size = n;
	}
	n = prof_data->entries++;
	prof_data->e[n].name = ast_strdup(name);
	prof_data->e[n].value = 0;
	prof_data->e[n].events = 0;
	prof_data->e[n].mark = 0;
	prof_data->e[n].scale = scale;
	return n;
}

int64_t ast_profile(int i, int64_t delta)
{
	if (!prof_data || i < 0 || i > prof_data->entries)	/* invalid index */
		return 0;
	if (prof_data->e[i].scale > 1)
		delta /= prof_data->e[i].scale;
	prof_data->e[i].value += delta;
	prof_data->e[i].events++;
	return prof_data->e[i].value;
}

/* The RDTSC instruction was introduced on the Pentium processor and is not
 * implemented on certain clones, like the Cyrix 586. Hence, the previous
 * expectation of __i386__ was in error. */
#if defined ( __i686__) && (defined(__FreeBSD__) || defined(linux))
#if defined(__FreeBSD__)
#include <machine/cpufunc.h>
#elif defined(linux)
static __inline uint64_t
rdtsc(void)
{ 
	uint64_t rv;

	__asm __volatile(".byte 0x0f, 0x31" : "=A" (rv));
	return (rv);
}
#endif
#else	/* supply a dummy function on other platforms */
static __inline uint64_t
rdtsc(void)
{
	return 0;
}
#endif

int64_t ast_mark(int i, int startstop)
{
	if (!prof_data || i < 0 || i > prof_data->entries) /* invalid index */
		return 0;
	if (startstop == 1)
		prof_data->e[i].mark = rdtsc();
	else {
		prof_data->e[i].mark = (rdtsc() - prof_data->e[i].mark);
		if (prof_data->e[i].scale > 1)
			prof_data->e[i].mark /= prof_data->e[i].scale;
		prof_data->e[i].value += prof_data->e[i].mark;
		prof_data->e[i].events++;
	}
	return prof_data->e[i].mark;
}


#endif



/*!
 * \internal
 * \brief Get the length of the exten string.
 *
 * \param str Exten to get length.
 *
 * \retval strlen of exten.
 */
static int ext_cmp_exten_strlen(const char *str)
{
	int len;

	len = 0;
	for (;;) {
		/* Ignore '-' chars as eye candy fluff. */
		while (*str == '-') {
			++str;
		}
		if (!*str) {
			break;
		}
		++str;
		++len;
	}
	return len;
}

/*!
 * \internal
 * \brief Partial comparison of non-pattern extens.
 *
 * \param left Exten to compare.
 * \param right Exten to compare.  Also matches if this string ends first.
 *
 * \retval <0 if left < right
 * \retval =0 if left == right
 * \retval >0 if left > right
 */
static int ext_cmp_exten_partial(const char *left, const char *right)
{
	int cmp;

	for (;;) {
		/* Ignore '-' chars as eye candy fluff. */
		while (*left == '-') {
			++left;
		}
		while (*right == '-') {
			++right;
		}

		if (!*right) {
			/*
			 * Right ended first for partial match or both ended at the same
			 * time for a match.
			 */
			cmp = 0;
			break;
		}

		cmp = *left - *right;
		if (cmp) {
			break;
		}
		++left;
		++right;
	}
	return cmp;
}

/*!
 * \internal
 * \brief Comparison of non-pattern extens.
 *
 * \param left Exten to compare.
 * \param right Exten to compare.
 *
 * \retval <0 if left < right
 * \retval =0 if left == right
 * \retval >0 if left > right
 */
static int ext_cmp_exten(const char *left, const char *right)
{
	int cmp;

	for (;;) {
		/* Ignore '-' chars as eye candy fluff. */
		while (*left == '-') {
			++left;
		}
		while (*right == '-') {
			++right;
		}

		cmp = *left - *right;
		if (cmp) {
			break;
		}
		if (!*left) {
			/*
			 * Get here only if both strings ended at the same time.  cmp
			 * would be non-zero if only one string ended.
			 */
			break;
		}
		++left;
		++right;
	}
	return cmp;
}

/*
 * Special characters used in patterns:
 *	'_'	underscore is the leading character of a pattern.
 *		In other position it is treated as a regular char.
 *	'-' The '-' is a separator and ignored.  Why?  So patterns like NXX-XXX-XXXX work.
 *	.	one or more of any character. Only allowed at the end of
 *		a pattern.
 *	!	zero or more of anything. Also impacts the result of CANMATCH
 *		and MATCHMORE. Only allowed at the end of a pattern.
 *		In the core routine, ! causes a match with a return code of 2.
 *		In turn, depending on the search mode: (XXX check if it is implemented)
 *		- E_MATCH retuns 1 (does match)
 *		- E_MATCHMORE returns 0 (no match)
 *		- E_CANMATCH returns 1 (does match)
 *
 *	/	should not appear as it is considered the separator of the CID info.
 *		XXX at the moment we may stop on this char.
 *
 *	X Z N	match ranges 0-9, 1-9, 2-9 respectively.
 *	[	denotes the start of a set of character. Everything inside
 *		is considered literally. We can have ranges a-d and individual
 *		characters. A '[' and '-' can be considered literally if they
 *		are just before ']'.
 *		XXX currently there is no way to specify ']' in a range, nor \ is
 *		considered specially.
 *
 * When we compare a pattern with a specific extension, all characters in the extension
 * itself are considered literally.
 * XXX do we want to consider space as a separator as well ?
 * XXX do we want to consider the separators in non-patterns as well ?
 */

/*!
 * \brief helper functions to sort extension patterns in the desired way,
 * so that more specific patterns appear first.
 *
 * \details
 * The function compares individual characters (or sets of), returning
 * an int where bits 0-7 are the ASCII code of the first char in the set,
 * bits 8-15 are the number of characters in the set, and bits 16-20 are
 * for special cases.
 * This way more specific patterns (smaller character sets) appear first.
 * Wildcards have a special value, so that we can directly compare them to
 * sets by subtracting the two values. In particular:
 *  0x001xx     one character, character set starting with xx
 *  0x0yyxx     yy characters, character set starting with xx
 *  0x18000     '.' (one or more of anything)
 *  0x28000     '!' (zero or more of anything)
 *  0x30000     NUL (end of string)
 *  0x40000     error in set.
 * The pointer to the string is advanced according to needs.
 * NOTES:
 *  1. the empty set is ignored.
 *  2. given that a full set has always 0 as the first element,
 *     we could encode the special cases as 0xffXX where XX
 *     is 1, 2, 3, 4 as used above.
 */
static int ext_cmp_pattern_pos(const char **p, unsigned char *bitwise)
{
#define BITS_PER	8	/* Number of bits per unit (byte). */
	unsigned char c;
	unsigned char cmin;
	int count;
	const char *end;

	do {
		/* Get character and advance. (Ignore '-' chars as eye candy fluff.) */
		do {
			c = *(*p)++;
		} while (c == '-');

		/* always return unless we have a set of chars */
		switch (c) {
		default:
			/* ordinary character */
			bitwise[c / BITS_PER] = 1 << ((BITS_PER - 1) - (c % BITS_PER));
			return 0x0100 | c;

		case 'n':
		case 'N':
			/* 2..9 */
			bitwise[6] = 0x3f;
			bitwise[7] = 0xc0;
			return 0x0800 | '2';

		case 'x':
		case 'X':
			/* 0..9 */
			bitwise[6] = 0xff;
			bitwise[7] = 0xc0;
			return 0x0A00 | '0';

		case 'z':
		case 'Z':
			/* 1..9 */
			bitwise[6] = 0x7f;
			bitwise[7] = 0xc0;
			return 0x0900 | '1';

		case '.':
			/* wildcard */
			return 0x18000;

		case '!':
			/* earlymatch */
			return 0x28000;	/* less specific than '.' */

		case '\0':
			/* empty string */
			*p = NULL;
			return 0x30000;

		case '[':
			/* char set */
			break;
		}
		/* locate end of set */
		end = strchr(*p, ']');

		if (!end) {
			//ast_log(LOG_WARNING, "Wrong usage of [] in the extension\n");
			return 0x40000;	/* XXX make this entry go last... */
		}

		count = 0;
		cmin = 0xFF;
		for (; *p < end; ++*p) {
			unsigned char c1;	/* first char in range */
			unsigned char c2;	/* last char in range */

			c1 = (*p)[0];
			if (*p + 2 < end && (*p)[1] == '-') { /* this is a range */
				c2 = (*p)[2];
				*p += 2;    /* skip a total of 3 chars */
			} else {        /* individual character */
				c2 = c1;
			}
			if (c1 < cmin) {
				cmin = c1;
			}
			for (; c1 <= c2; ++c1) {
				unsigned char mask = 1 << ((BITS_PER - 1) - (c1 % BITS_PER));

				/*
				 * Note: If two character sets score the same, the one with the
				 * lowest ASCII values will compare as coming first.  Must fill
				 * in most significant bits for lower ASCII values to accomplish
				 * the desired sort order.
				 */
				if (!(bitwise[c1 / BITS_PER] & mask)) {
					/* Add the character to the set. */
					bitwise[c1 / BITS_PER] |= mask;
					count += 0x100;
				}
			}
		}
		++*p;
	} while (!count);/* While the char set was empty. */
	return count | cmin;
}

/*!
 * \internal
 * \brief Comparison of exten patterns.
 *
 * \param left Pattern to compare.
 * \param right Pattern to compare.
 *
 * \retval <0 if left < right
 * \retval =0 if left == right
 * \retval >0 if left > right
 */
static int ext_cmp_pattern(const char *left, const char *right)
{
	int cmp;
	int left_pos;
	int right_pos;

	for (;;) {
		unsigned char left_bitwise[32] = { 0, };
		unsigned char right_bitwise[32] = { 0, };

		left_pos = ext_cmp_pattern_pos(&left, left_bitwise);
		right_pos = ext_cmp_pattern_pos(&right, right_bitwise);
		cmp = left_pos - right_pos;
		if (!cmp) {
			/*
			 * Are the character sets different, even though they score the same?
			 *
			 * Note: Must swap left and right to get the sense of the
			 * comparison correct.  Otherwise, we would need to multiply by
			 * -1 instead.
			 */
			cmp = memcmp(right_bitwise, left_bitwise, ARRAY_LEN(left_bitwise));
		}
		if (cmp) {
			break;
		}
		if (!left) {
			/*
			 * Get here only if both patterns ended at the same time.  cmp
			 * would be non-zero if only one pattern ended.
			 */
			break;
		}
	}
	return cmp;
}

/*!
 * \internal
 * \brief Comparison of dialplan extens for sorting purposes.
 *
 * \param left Exten/pattern to compare.
 * \param right Exten/pattern to compare.
 *
 * \retval <0 if left < right
 * \retval =0 if left == right
 * \retval >0 if left > right
 */
static int ext_cmp(const char *left, const char *right)
{
	/* Make sure non-pattern extens come first. */
	if (left[0] != '_') {
		if (right[0] == '_') {
			return -1;
		}
		/* Compare two non-pattern extens. */
		return ext_cmp_exten(left, right);
	}
	if (right[0] != '_') {
		return 1;
	}

	/*
	 * OK, we need full pattern sorting routine.
	 *
	 * Skip past the underscores
	 */
	return ext_cmp_pattern(left + 1, right + 1);
}

int ast_extension_cmp(const char *a, const char *b)
{
	int cmp;

	cmp = ext_cmp(a, b);
	if (cmp < 0) {
		return -1;
	}
	if (cmp > 0) {
		return 1;
	}
	return 0;
}

int data_pattern_cmp(const char *data, const char *pattern)
{
	if (*data == *pattern)
		return 1;
	else if(*data == '*') 
	{
		if(*pattern == '*' || *pattern == 'e' || *pattern == 'E')
			return 1;
		else
			return 0;
	}
	else if(*data == '#') 
	{
		if(*pattern == '#' || *pattern == 'f' || *pattern == 'F')
			return 1;
		else
			return 0;
	}	
	else
	{
		return 0;
	}
}

int data_match_rules1(const char *pattern, const char *data)
{
	const char *end;
	++pattern;
	end = strchr(pattern, ']'); /* XXX should deal with escapes ? */
	if (!end) 
	{
		//ast_log(LOG_WARNING, "Wrong usage of [] in the extension\n");
		return 0;	/* unconditional failure */
	}
	if (pattern == end) 
	{
		return 2;
	}
	for (; pattern < end; ++pattern) 
	{
		if (pattern+2 < end && pattern[1] == '-') 
		{ /* this is a range */
			if (*data >= pattern[0] && *data <= pattern[2])
				break;	/* match found */
			else 
			{
				pattern += 2; /* skip a total of 3 chars */
				continue;
			}
		} 
		else if (data_pattern_cmp(data, pattern))
			break;
	}
	if (pattern >= end) 
	{
		return 0;
	}
	return 1;
}

/*!
 * \internal
 * \brief used ast_extension_{match|close}
 * mode is as follows:
 *	E_MATCH		success only on exact match
 *	E_MATCHMORE	success only on partial match (i.e. leftover digits in pattern)
 *	E_CANMATCH	either of the above.
 * \retval 0 on no-match
 * \retval 1 on match
 * \retval 2 on early match.
 */
 
static int _extension_match_core(const char *pattern, const char *data, enum ext_match_t mode)
{
	mode &= E_MATCH_MASK;	/* only consider the relevant bits */
	int ret = -1;
	char pattbuf[20] = {0};
#ifdef NEED_DEBUG_HERE
	ast_log(LOG_NOTICE,"match core: pat: '%s', dat: '%s', mode=%d\n", pattern, data, (int)mode);
#endif

#if 0
	if (pattern[0] != '_') { /* not a pattern, try exact or partial match */
		int lp = ext_cmp_exten_strlen(pattern);
		int ld = ext_cmp_exten_strlen(data);

		if (lp < ld) {		/* pattern too short, cannot match */
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE,"return (0) - pattern too short, cannot match\n");
#endif
			return 0;
		}
		/* depending on the mode, accept full or partial match or both */
		if (mode == E_MATCH) {
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE,"return (!ext_cmp_exten(%s,%s) when mode== E_MATCH)\n", pattern, data);
#endif
			return !ext_cmp_exten(pattern, data); /* 1 on match, 0 on fail */
		}
		if (ld == 0 || !ext_cmp_exten_partial(pattern, data)) { /* partial or full match */
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE,"return (mode(%d) == E_MATCHMORE ? lp(%d) > ld(%d) : 1)\n", mode, lp, ld);
#endif
			return (mode == E_MATCHMORE) ? lp > ld : 1; /* XXX should consider '!' and '/' ? */
		} else {
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE,"return (0) when ld(%d) > 0 && pattern(%s) != data(%s)\n", ld, pattern, data);
#endif
			return 0;
		}
	}
	if (mode == E_MATCH && data[0] == '_') {
		/*
		 * XXX It is bad design that we don't know if we should be
		 * comparing data and pattern as patterns or comparing data if
		 * it conforms to pattern when the function is called.  First,
		 * assume they are both patterns.  If they don't match then try
		 * to see if data conforms to the given pattern.
		 *
		 * note: if this test is left out, then _x. will not match _x. !!!
		 */
#ifdef NEED_DEBUG_HERE
		ast_log(LOG_NOTICE, "Comparing as patterns first. pattern:%s data:%s\n", pattern, data);
#endif
		if (!ext_cmp_pattern(pattern + 1, data + 1)) {
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE,"return (1) - pattern matches pattern\n");
#endif
			return 1;
		}
	}

	++pattern; /* skip leading _ */
#endif

	/*
	 * XXX below we stop at '/' which is a separator for the CID info. However we should
	 * not store '/' in the pattern at all. When we insure it, we can remove the checks.
	 */
	for (;;) 
	{
		const char *end;
		/* Ignore '-' chars as eye candy fluff. */
		while (*data == '-') 
		{
			++data;
		}
		while (*pattern == '-') 
		{
			++pattern;
		}
		if (!*data || !*pattern || *pattern == '/') 
		{
			break;
		}

		switch (*pattern) 
		{
#if 0
			case '[':	/* a range */
				memset(pattbuf, 0, sizeof(pattbuf));				
				ret = data_match_rules1(pattern, data);
				++pattern;
				end = strchr(pattern, ']'); 
				memcpy(pattbuf, pattern - 1, end - pattern + 2);
				if (ret == 0) // ≤ª∆•≈‰
				{
					return 0;
				}
				else if (ret == 1) // ∆•≈‰
				{
					
					pattern = end;				
				}
				else  // [] ¿Ô√ÊŒ™ø’
				{
					++pattern;
					continue;
				}
				break;
#else
			case '[':	/* a range */
				++pattern;
				end = strchr(pattern, ']'); /* XXX should deal with escapes ? */
				if (!end) 
				{
					//ast_log(LOG_WARNING, "Wrong usage of [] in the extension\n");
					return 0;	/* unconditional failure */
				}
				if (pattern == end) 
				{
					/* Ignore empty character sets. */
					++pattern;
					continue;
				}
				for (; pattern < end; ++pattern) 
				{
					if (pattern+2 < end && pattern[1] == '-') 
					{ /* this is a range */
						if (*data >= pattern[0] && *data <= pattern[2])
							break;	/* match found */
						else 
						{
							pattern += 2; /* skip a total of 3 chars */
							continue;
						}
					} 
					else if (*data == pattern[0])
						break;	/* match found */
				}
				if (pattern >= end) 
				{
					return 0;
				}
				pattern = end;	/* skip and continue */
				break;
#endif
			case 'n':
			case 'N':
				memset(pattbuf, 0, sizeof(pattbuf));
				*pattbuf = 'n';
				if (*data < '2' || *data > '9') 
				{
					return 0;
				}
				break;
			case 'x':
			case 'X':
				memset(pattbuf, 0, sizeof(pattbuf));
				*pattbuf = 'x';
				if (*data < '0' || *data > '9') 
				{
					return 0;
				}
				break;
			case 'z':
			case 'Z':
				memset(pattbuf, 0, sizeof(pattbuf));
				*pattbuf = 'z';
				if (*data < '1' || *data > '9') 
				{
					return 0;
				}
				break;
#if 0
			case '*':
			case 'e':
			case 'E':
				memset(pattbuf, 0, sizeof(pattbuf));
				*pattbuf = '*';
				if (*data != '*')
						return 0;
				break;
			case '#':
			case 'f':
			case 'F':
				memset(pattbuf, 0, sizeof(pattbuf));
				*pattbuf = '#';
				if (*data != '#')
						return 0;
				break;
			case '.':
				if (*pattbuf == '[')
				{
					ret = data_match_rules1(pattbuf, data);
					if (ret == 1)
					{
						++data;
						continue;
					}
					else if (ret == 0)
					{
						return 0;
					}
					else // [ ] ¿Ô√ÊŒ™ø’£
					{
						++pattern;
						continue;
					}
				}
				else if (*pattbuf == 'n')
				{
					if (*data < '2' || *data > '9') 
					{
						return 0;
					}
					else
					{
						++data;
						continue;
					}
				}
				else if (*pattbuf == 'x')
				{
					if (*data < '0' || *data > '9') 
					{
						return 0;
					}
					else
					{
						++data;
						continue;
					}
				}
				else if (*pattbuf == 'z')
				{
					if (*data < '1' || *data > '9') 
					{
						return 0;
					}
					else
					{
						++data;
						continue;
					}
				}
				else if (*pattbuf == '*')
				{
					if (*data != '*')
						return 0;
					else
					{
						++data;
						continue;
					}
				}
				else if (*pattbuf == '#')
				{
					if (*data != '#')
						return 0;
					else
					{
						++data;
						continue;
					}
				}			
				break;			
			default:
				if (*data != *pattern) 
				{
					return 0;
				}
				break;
#endif
#if 0			
			case '.':	/* Must match, even with more digits */
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE, "return (1) when '.' is matched\n");
#endif
			return 1;
			case '!':	/* Early match */
#ifdef NEED_DEBUG_HERE
			ast_log(LOG_NOTICE, "return (2) when '!' is matched\n");
#endif
			return 2;
#endif
#if 1
			case '.':	/* Must match, even with more digits */
				if (*data < '0' || *data > '9') //≤ª « ˝◊÷
					return 0;
				data++;
				if (!*data) //√ª”–÷µ
					return 2;
				while (1)
				{
					if (*data < '0' || *data > '9') //≤ª « ˝◊÷
					{
						pattern++;
						if (!*pattern)
							return 0;
						else
						{
							if (data_pattern_cmp(data, pattern))
								break;
							else
								return 0;
						}
					}
					else
					{
						data++;
						if (!*data) //√ª”–÷µ
							return 2;
					}
				}
				break;
			default:
				if (data_pattern_cmp(data, pattern))
					break;
				else
					return 0;
#endif
#if 0
			if (*data != *pattern) 
			{
				return 0;
			}
			break;
#endif
		}
		++data;
		++pattern;
	}
	if (*data)			/* data longer than pattern, no match */ 
	{
		return 0;
	}

	/*
	 * match so far, but ran off the end of data.
	 * Depending on what is next, determine match or not.
	 */
	if (*pattern == '\0' || *pattern == '/') 
	{	/* exact match */
		return (mode == E_MATCHMORE) ? 0 : 1;	/* this is a failure for E_MATCHMORE */ // πÊ‘ÚÕÍ¡À£¨ ˝æ›“≤ÕÍ¡À£¨∑µªÿ1
	}
	else
	{
		return (mode == E_CANMATCH) ? 0 : 2;	/* this is a failure for E_MATCH */  // πÊ‘Ú√ªÕÍ£¨ ˝æ›ÕÍ¡À£¨∑µªÿ2
	}
#if 0
	else if (*pattern == '!')	{		/* early match */
#ifdef NEED_DEBUG_HERE
		ast_log(LOG_NOTICE, "at end, return (2) when '!' is matched\n");
#endif
		return 2;
	}
#endif
#if 0
	else 
	{						/* partial match */
		return (mode == E_MATCH) ? 0 : 1;	/* this is a failure for E_MATCH */
	}
#endif
}

/*
 * Wrapper around _extension_match_core() to do performance measurement
 * using the profiling code.
 */
static int extension_match_core(const char *pattern, const char *data, enum ext_match_t mode)
{
	int i;
	static int prof_id = -2;	/* marker for 'unallocated' id */
	if (prof_id == -2) 
	{
		prof_id = ast_add_profile("ext_match", 0);
	}
	ast_mark(prof_id, 1);
	i = _extension_match_core(ast_strlen_zero(pattern) ? "" : pattern, ast_strlen_zero(data) ? "" : data, mode);
	ast_mark(prof_id, 0);
	return i;
}


int ast_extension_match(const char *pattern, const char *data)
{
	return extension_match_core(pattern, data, E_MATCH);
}

int ast_extension_close(const char *pattern, const char *data, int needmore)
{
	//if (needmore != E_MATCHMORE && needmore != E_CANMATCH)
		//ast_log(LOG_WARNING, "invalid argument %d\n", needmore);
	return extension_match_core(pattern, data, needmore);
}

int pattern_separate_match(char *pattern,char *data)
{	
	int ret = 0, result = 0;
	int len = strlen(pattern)+1;
	
	char *patternx = (char *)malloc(len);
	
	memcpy(patternx,pattern,len);
#ifdef _NUMDUG_
	printf("patternx=%s\n",patternx);
#endif

  	char *cur = NULL;//µ±«∞Œª÷√÷∏’Î
	char *befor = NULL;//…œ“ª¥ŒµƒŒª÷√÷∏’Î

	befor = patternx;
	cur = strchr(befor, '|');

	if (cur == NULL)
	{
		ret = ast_extension_match(befor, data);
		goto out;
	}
	while (cur)
	{
#ifdef _NUMDUG_
		printf("cur-befor=%d\n", cur-befor);
#endif
		*cur = '\0';
		ret = ast_extension_match(befor,data);
		if (ret == 2)
		{
			result = 1;
		}
		else if (ret == 1)
		{
			goto out;
		}
		befor = cur+1;
		cur = strchr(befor, '|');	
	}
	ret = ast_extension_match(befor, data);
out:
	free(patternx);
	if (ret == 1)
		return 1;
	else if (result == 1)
		return 2;
	return ret;
}

