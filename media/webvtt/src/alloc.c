#include <webvtt/util.h>
#if ( defined(__APPLE__) && defined(__MACH__) )
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>

static void *default_alloc( void *unused, webvtt_uint nb );
static void default_free( void *unused, void *ptr );

struct {
  webvtt_uint n_alloc; /* Number of allocated objects. Forbid changing the allocator if this is not equal to 0 */
  webvtt_alloc_fn_ptr alloc;
  webvtt_free_fn_ptr free;
  void *alloc_data;
} allocator = { 0, default_alloc, default_free, 0 };

static void *WEBVTT_CALLBACK
default_alloc( void *unused, webvtt_uint nb )
{
  return malloc( nb );
}

static void WEBVTT_CALLBACK
default_free( void *unused, void *ptr )
{
  free( ptr );
}

WEBVTT_EXPORT void
webvtt_set_allocator( webvtt_alloc_fn_ptr alloc, webvtt_free_fn_ptr free, void *userdata )
{
  /**
   * TODO:
   * This really needs a lock. But then, so does all the allocation/free functions...
   * that could be a problem.
   */
  if( allocator.n_alloc == 0 ) {
    if( alloc && free ) {
      allocator.alloc = alloc;
      allocator.free = free;
      allocator.alloc_data = userdata;
    } else if( !alloc && !free ) {
      allocator.alloc = &default_alloc;
      allocator.free = &default_free;
      allocator.alloc_data = 0;
    }
  }
}

/**
 * public alloc/dealloc functions
 */
WEBVTT_EXPORT void *
webvtt_alloc( webvtt_uint nb )
{
  void *ret = allocator.alloc( allocator.alloc_data, nb );
  if( ret )
  { ++allocator.n_alloc; }
  return ret;
}

WEBVTT_EXPORT void *
webvtt_alloc0( webvtt_uint nb )
{
  void *ret = allocator.alloc( allocator.alloc_data, nb );
  if( ret ) {
    ++allocator.n_alloc;
    memset( ret, 0, nb );
  }
  return ret;
}

WEBVTT_EXPORT void
webvtt_free( void *data )
{
  if( data && allocator.n_alloc ) {
    allocator.free( allocator.alloc_data, data );
    --allocator.n_alloc;
  }
}
