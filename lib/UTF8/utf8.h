/*
 * small library for handling of utf8
 */
#ifndef UTF8_H
#define UTF8 1

#ifdef __cplusplus 
extern "C" {
#endif
/*
 * returns length string if unicode chars are counted as one
*/
int utf8_strlen(char *p);

/*
 * returns the unicode character that corresponds to the parameter as a malloced string 
*/
char *utf8_encode(int val);

/*
 * returns the value of the unicode character starting at the pointer
 * otherwise it returns 0
*/
long utf8_decode(char *p);

#ifdef __cplusplus 
}
#endif
#endif
