#ifndef __WEBVTT_PARSER_H__
# define __WEBVTT_PARSER_H__
# include "string.h"
# include "cue.h"
# include "error.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

  typedef struct webvtt_parser_t *webvtt_parser;

  /**
   * Allows application to request error reporting
   */
  typedef int ( WEBVTT_CALLBACK *webvtt_error_fn )( void *userdata, webvtt_uint line, webvtt_uint col,
      webvtt_error error );
  typedef void ( WEBVTT_CALLBACK *webvtt_cue_fn )( void *userdata, webvtt_cue *cue );


  WEBVTT_EXPORT webvtt_status webvtt_create_parser( webvtt_cue_fn on_read, webvtt_error_fn on_error, void *
      userdata, webvtt_parser *ppout );
  WEBVTT_EXPORT void webvtt_delete_parser( webvtt_parser parser );
  WEBVTT_EXPORT webvtt_status webvtt_parse_chunk( webvtt_parser self, const void *buffer, webvtt_uint len, webvtt_bool is_final_chunk );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
