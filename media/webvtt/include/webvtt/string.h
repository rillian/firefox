#ifndef __WEBVTT_STRING_H__
# define __WEBVTT_STRING_H__
# include "util.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/**
* webvtt_string - A buffer of utf16 characters
*/
typedef struct webvtt_string_t webvtt_string;
typedef struct webvtt_string_data_t webvtt_string_data;
typedef struct webvtt_stringlist_t webvtt_stringlist;
struct webvtt_string_data_t;

struct
webvtt_string_t {
  webvtt_string_data *d;
};

/**
 * webvtt_init_string
 *
 * initialize a string to point to the empty string
 */
WEBVTT_EXPORT void webvtt_init_string( webvtt_string *result );

/**
 * webvtt_create_string
 *
 * allocate a new string object with an initial capacity of 'alloc'
 * (the string data of 'result' is not expected to contain string data, regardless of its value.
 * be sure to release existing strings before using webvtt_create_string)
 */
WEBVTT_EXPORT webvtt_status webvtt_create_string( webvtt_uint32 alloc, webvtt_string *result );

/**
 * webvtt_create_init_string
 *
 * allocate and initialize a string with the contents of 'init_text' of length 'len'
 * if 'len' < 0, assume init_text to be null-terminated.
 */
WEBVTT_EXPORT webvtt_status webvtt_create_string_with_text( webvtt_string *result, const webvtt_byte *init_text, int len );

/**
 * webvtt_ref_string
 *
 * increase the reference count of--or retain--a string
 *
 * when the reference count drops to zero, the string is deallocated.
 */
WEBVTT_EXPORT void webvtt_ref_string( webvtt_string *str );

/**
 * webvtt_release_string
 *
 * decrease the reference count of--or release--a string
 *
 * when the reference count drops to zero, the string is deallocated.
 */
WEBVTT_EXPORT void webvtt_release_string( webvtt_string *str );

/**
 * webvtt_string_detach
 *
 * ensure that the reference count of a string is exactly 1
 *
 * if the reference count is greater than 1, allocate a new copy of the string
 * and return it.
 */
WEBVTT_EXPORT webvtt_status webvtt_string_detach( webvtt_string *str );

/**
 * webvtt_copy_string
 *
 * shallow-clone 'right', storing the result in 'left'.
 */
WEBVTT_EXPORT void webvtt_copy_string( webvtt_string *left, const webvtt_string *right );

/**
 * webvtt_string_text
 *
 * return the text contents of a string
 */
WEBVTT_EXPORT const webvtt_byte *webvtt_string_text( const webvtt_string *str );

/**
 * webvtt_string_length
 *
 * return the length of a strings text
 */
WEBVTT_EXPORT const webvtt_uint32 webvtt_string_length( const webvtt_string *str );

/**
 * webvtt_string_capacity
 *
 * return the current capacity of a string
 */
WEBVTT_EXPORT const webvtt_uint32 webvtt_string_capacity( const webvtt_string *str );

/**
 * webvtt_string_getline
 *
 * collect a line of text (terminated by CR/LF/CRLF) from a buffer, without including the terminating character(s)
 */
WEBVTT_EXPORT int webvtt_string_getline( webvtt_string *str, const webvtt_byte *buffer,
    webvtt_uint *pos, webvtt_uint len, int *truncate, webvtt_bool finish, webvtt_bool retain_new_line );

/**
 * webvtt_string_putc
 *
 * append a single byte to a webvtt string
 */
WEBVTT_EXPORT webvtt_status webvtt_string_putc( webvtt_string *str, webvtt_byte to_append );

/**
 * webvtt_string_append
 *
 * append a stream of bytes to the string.
 *
 * if 'len' is < 0, then buffer is expected to be null-terminated.
 */
WEBVTT_EXPORT webvtt_status webvtt_string_append( webvtt_string *str, const webvtt_byte *buffer, webvtt_uint32 len );

/**
 * webvtt_string_appendstr
 *
 * append the contents of a string object 'other' to a string object 'str'
 */
WEBVTT_EXPORT webvtt_status webvtt_string_append_string( webvtt_string *str, const webvtt_string *other );

/**
 * basic dynamic array of strings
 */
struct
webvtt_stringlist_t {
  webvtt_uint alloc;
  webvtt_uint length;
  webvtt_string *items;
};

/**
 * webvtt_create_stringlist
 *
 * allocate a new, empty stringlist
 */
WEBVTT_EXPORT webvtt_status webvtt_create_stringlist( webvtt_stringlist **result );

/**
 * webvtt_delete_stringlist
 *
 * release each listed string, and delete the stringlist object.
 */
WEBVTT_EXPORT void webvtt_delete_stringlist( webvtt_stringlist **list );

/**
 * webvtt_stringlist_push
 *
 * add a new string to the end of the stringlist
 */
WEBVTT_EXPORT webvtt_status webvtt_stringlist_push( webvtt_stringlist *list, webvtt_string *str );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif
