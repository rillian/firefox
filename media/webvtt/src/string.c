#include "string_internal.h"
#include <stdlib.h>
#include <string.h>


static webvtt_string_data empty_string = {
  { 1 }, /* init refcount */
  0, /* length */
  0, /* capacity */
  empty_string.array, /* text */
  { '\0' } /* array */
};

WEBVTT_EXPORT void
webvtt_init_string( webvtt_string *result )
{
  if( result && result->d != &empty_string ) {
    webvtt_string_data *d = result->d;
    result->d = &empty_string;
    webvtt_ref( &result->d->refs );
    if( d && ( webvtt_deref( &d->refs ) == 0 ) ) {
      webvtt_free( d );
    }
  }
}

/**
 * Allocate new string.
 */
WEBVTT_EXPORT webvtt_status
webvtt_create_string( webvtt_uint32 alloc, webvtt_string *result )
{
  webvtt_string_data *d;

  if( !result ) {
    return WEBVTT_INVALID_PARAM;
  }

  d = ( webvtt_string_data * )webvtt_alloc( sizeof( webvtt_string_data ) + ( alloc * sizeof( webvtt_byte ) ) );

  if( !d ) {
    return WEBVTT_OUT_OF_MEMORY;
  }

  d->refs.value = 1;
  d->alloc = alloc;
  d->length = 0;
  d->text = d->array;
  d->text[0] = 0;

  result->d = d;

  return WEBVTT_SUCCESS;
}

WEBVTT_EXPORT webvtt_status
webvtt_create_string_with_text( webvtt_string *result, const webvtt_byte *init_text, int len )
{
  webvtt_uint pos = 0;
  
  if( !result && !init_text ) {
    return WEBVTT_INVALID_PARAM;
  }
  
  /**
   * initialize the string by referencing empty_string
   */
  webvtt_init_string( result );

  /**
   * append the appropriate data to the empty string
   */
  return webvtt_string_append( result, init_text, len );  
}

/**
 * reference counting
 */
WEBVTT_EXPORT void
webvtt_ref_string( webvtt_string *str )
{
  if( str ) {
    webvtt_ref( &str->d->refs );
  }
}

WEBVTT_EXPORT void
webvtt_release_string( webvtt_string *str )
{
  /**
   * pulls the string data out of the string container, decreases the string
   */
  if( str ) {
    webvtt_string_data *d = str->d;
    str->d = &empty_string;
    webvtt_ref( &str->d->refs );
    if( d && webvtt_deref( &d->refs ) == 0 ) {
      webvtt_free( d );
    }
  }
}

/**
 * "Detach" a shared string, so that it's safely mutable
 */
WEBVTT_EXPORT webvtt_status
webvtt_string_detach( /* in, out */ webvtt_string *str )
{
  webvtt_string_data *d, *q;

  if( !str ) {
    return WEBVTT_INVALID_PARAM;
  }

  q = str->d;

  if( q->refs.value == 1 ) {
    return WEBVTT_SUCCESS;
  }

  d = ( webvtt_string_data * )webvtt_alloc( sizeof( webvtt_string_data ) + ( sizeof( webvtt_byte ) * str->d->alloc ) );

  d->refs.value = 1;
  d->text = d->array;
  d->alloc = q->alloc;
  d->length = q->length;
  memcpy( d->text, q->text, q->length );

  str->d = d;

  if( webvtt_deref( &q->refs ) == 0 ) {
    webvtt_free( q );
  }

  return WEBVTT_SUCCESS;
}

WEBVTT_EXPORT void
webvtt_copy_string( webvtt_string *left, const webvtt_string *right )
{
  if( left ) {
    webvtt_string_data *d = left->d;
    if( right ) {
      left->d = right->d;
    } else {
      left->d = &empty_string;
    }
    webvtt_deref( &left->d->refs );
    if( webvtt_deref( &d->refs ) == 0 ) {
      /**
       * We don't try to check if we're freeing a static string or not.
       * Static strings should be initialized with a reference count of '1',
       * and should be ref'd or deref'd properly.
       *
       * If this is difficult, use the C++ bindings!
       */
      webvtt_free( d );
    }
  }
}

WEBVTT_EXPORT const webvtt_byte *
webvtt_string_text(const webvtt_string *str)
{
  if( !str || !str->d )
  {
    return 0;
  }
  
  return str->d->text;
}

WEBVTT_EXPORT const webvtt_uint32
webvtt_string_length(const webvtt_string *str)
{
  if( !str || !str->d )
  {
    return 0;
  }
  
  return str->d->length;
}

WEBVTT_EXPORT const webvtt_uint32
webvtt_string_capacity(const webvtt_string *str)
{
  if( !str || !str->d )
  {
    return 0;
  }
  
  return str->d->alloc;
}

/**
 * Reallocate string.
 * Grow to at least 'need' characters. Power of 2 growth.
 */
static webvtt_status
grow( webvtt_string *str, webvtt_uint need )
{
  static const webvtt_uint page = 0x1000;
  webvtt_uint32 n;
  webvtt_string_data *p, *d;
  webvtt_uint32 grow;

  if( !str )
  {
    return WEBVTT_INVALID_PARAM;
  }

  if( ( str->d->length + need ) <= str->d->alloc ) 
  { 
    return WEBVTT_SUCCESS;
  } 

  p = d = str->d;
  grow = sizeof( *d ) + ( sizeof( webvtt_byte ) * ( d->length + need ) );

  if( grow < page ) {
    n = page;
    do {
      n = n / 2;
    } while( n > grow );
    if( n < 1 << 6 ) {
      n = 1 << 6;
    } else {
      n = n * 2;
    }
  } else {
    n = page;
    do {
      n = n * 2;
    } while ( n < grow );
  }

  p = ( webvtt_string_data * )webvtt_alloc( n );

  if( !p ) {
    return WEBVTT_OUT_OF_MEMORY;
  }

  p->refs.value = 1;
  p->alloc = ( n - sizeof( *p ) ) / sizeof( webvtt_byte );
  p->length = d->length;
  p->text = p->array;
  memcpy( p->text, d->text, sizeof( webvtt_byte ) * p->length );
  p->text[ p->length ] = 0;
  str->d = p;

  if( webvtt_deref( &d->refs ) == 0 ) {
    webvtt_free( d );
  }

  return WEBVTT_SUCCESS;
}

WEBVTT_EXPORT int
webvtt_string_getline( webvtt_string *src, const webvtt_byte *buffer,
    webvtt_uint *pos, webvtt_uint len, int *truncate, webvtt_bool finish, webvtt_bool retain_new_line )
{
  int ret = 0;
  webvtt_string *str = src;
  webvtt_string_data *d = 0;
  const webvtt_byte *s = buffer + *pos;
  const webvtt_byte *p = s;
  const webvtt_byte *n = buffer + len;

  /**
   *if this is public now, maybe we should return webvtt_status so we can
   * differentiate between WEBVTT_OUT_OF_MEMORY and WEBVTT_INVALID_PARAM
   */
  if( !str ) {
    return -1;
  }

  /* This had better be a valid string_data, or else NULL. */
  d = str->d;
  if( !str->d ) {
    if(WEBVTT_FAILED(webvtt_create_string( 0x100, str ))) {
      return -1;
    }
    d = str->d;
  }

  while( p < n && *p != UTF8_CARRIAGE_RETURN && *p != UTF8_LINE_FEED ) {
    ++p;
  }
  /* Retain the new line character. */
  if( retain_new_line ) {
    p++;
  }

  if( p < n || finish ) {
    ret = 1; /* indicate that we found EOL */
  }
  len = (webvtt_uint)( p - s );
  *pos += len;
  if( d->length + len + 1 >= d->alloc ) {
    if( truncate && d->alloc >= WEBVTT_MAX_LINE ) {
      /* truncate. */
      (*truncate)++;
    } else {
      if( grow( str, len ) == WEBVTT_OUT_OF_MEMORY ) {
        ret = -1;
      }
      d = str->d;
    }
  }

  /* Copy everything in */
  if( len && ret >= 0 && d->length + len < d->alloc ) {
    memcpy( d->text + d->length, s, len );
    d->length += len;
    d->text[ d->length ] = 0;
  }

  return ret;
}

WEBVTT_EXPORT webvtt_status
webvtt_string_putc( webvtt_string *str, webvtt_byte to_append )
{
  webvtt_status result;

  if( !str ) {
    return WEBVTT_INVALID_PARAM;
  }

  if( WEBVTT_FAILED( result = webvtt_string_detach( str ) ) ) {
    return result;
  }
  
  if( WEBVTT_SUCCESS( result == grow( str, 1 ) ) )
  {
    str->d->text[ str->d->length++ ] = to_append;
    str->d->text[ str->d->length ] = 0;
  }

  return result;
}

WEBVTT_EXPORT webvtt_status
webvtt_string_append( webvtt_string *str, const webvtt_byte *buffer, webvtt_uint32 len )
{
  webvtt_status result;
  webvtt_uint size;

  if( !str || !buffer ) {
    return WEBVTT_INVALID_PARAM;
  }
  if( !str->d ) {
    webvtt_init_string( str );
  }

  size = str->d->length + len;

  /**
   * Ensure that we have at least 'len' characters available.
   */
  if( size && WEBVTT_SUCCESS( result = grow( str, size ) ) ) {
    memcpy( str->d->text, buffer, size );
	str->d->length += size;
  }

  /* null-terminate string */
  str->d->text[ str->d->length ] = 0;
  
  return result;
}

WEBVTT_EXPORT webvtt_status 
 webvtt_string_append_string( webvtt_string *str, const webvtt_string *other )
{
  if( !str || !other ) {
    return WEBVTT_INVALID_PARAM;
  }

  return webvtt_string_append( str, other->d->text, other->d->length );
}

/**
 * String lists
 */
WEBVTT_EXPORT webvtt_status
webvtt_create_stringlist( webvtt_stringlist **result )
{
  webvtt_stringlist *list;

  if( !result ) {
    return WEBVTT_INVALID_PARAM;
  }

  list = ( webvtt_stringlist * )webvtt_alloc0( sizeof( *list ) );

  if( !list ) {
    return WEBVTT_OUT_OF_MEMORY;
  }

  *result = list;

  return WEBVTT_SUCCESS;
}

WEBVTT_EXPORT void
webvtt_delete_stringlist( webvtt_stringlist **list )
{
  webvtt_uint i;

  if( list && *list ) {
    webvtt_stringlist *l = *list;

    *list = 0;
    if( l->items ) {
      for( i = 0; i < l->length; i++ ) {
        webvtt_release_string( &l->items[i] );
      }
      webvtt_free( l->items );
    }
    webvtt_free( l );
  }
}

WEBVTT_EXPORT webvtt_status
webvtt_stringlist_push( webvtt_stringlist *list, webvtt_string *str )
{
  if( !list || !str ) {
    return WEBVTT_INVALID_PARAM;
  }

  if( list->length + 1 >= ( ( list->alloc / 3 ) * 2 ) ) {
    webvtt_string *arr, *old;

    list->alloc = list->alloc == 0 ? 8 : list->alloc * 2;
    arr = ( webvtt_string * )webvtt_alloc0( sizeof( webvtt_string ) * list->alloc );

    if( !arr ) {
      return WEBVTT_OUT_OF_MEMORY;
    }

    memcpy( arr, list->items, sizeof( webvtt_string ) * list->length );
    old = list->items;
    list->items = arr;

    webvtt_free( old );
  }

  list->items[list->length].d = str->d;
  webvtt_ref_string( list->items + list->length++ );

  return WEBVTT_SUCCESS;
}
