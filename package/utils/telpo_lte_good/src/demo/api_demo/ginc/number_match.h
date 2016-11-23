/*

*/

#ifndef _NUMBER_MATCH_H
#define _NUMBER_MATCH_H

#define E_EXACT		1
#define E_PART			2

enum ext_match_t {
	E_MATCHMORE = 	0x00,	/* extension can match but only with more 'digits' */
	E_CANMATCH =	0x01,	/* extension can match with or without more 'digits' */
	E_MATCH =	0x02,	/* extension is an exact match */
	E_MATCH_MASK =	0x03,	/* mask for the argument to extension_match_core() */
	E_SPAWN =	0x12,	/* want to spawn an extension. Requires exact match */
	E_FINDLABEL =	0x22	/* returns the priority for a given label. Requires exact match */
};

int ast_extension_match(const char *pattern, const char *data);
int ast_extension_close(const char *pattern, const char *data, int needmore);

#define _NUMDUG_
#undef _NUMDUG_

#endif
