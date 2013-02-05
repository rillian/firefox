#include <stdlib.h>
#include <string.h>
#include "parser_internal.h"
#include "cue_internal.h"

WEBVTT_EXPORT webvtt_status
webvtt_create_cue( webvtt_cue **pcue )
{
  webvtt_cue *cue;
  if( !pcue ) {
    return WEBVTT_INVALID_PARAM;
  }
  cue = (webvtt_cue *)webvtt_alloc0( sizeof(*cue) );
  if( !cue ) {
    return WEBVTT_OUT_OF_MEMORY;
  }
  /**
   * From http://dev.w3.org/html5/webvtt/#parsing (10/25/2012)
   *
   * Let cue's text track cue snap-to-lines flag be true.
   *
   * Let cue's text track cue line position be auto.
   *
   * Let cue's text track cue text position be 50.
   *
   * Let cue's text track cue size be 100.
   *
   * Let cue's text track cue alignment be middle alignment.
   */
  webvtt_ref( &cue->refs );
  webvtt_init_string( &cue->id );
  cue->from = 0xFFFFFFFFFFFFFFFF;
  cue->until = 0xFFFFFFFFFFFFFFFF;
  cue->snap_to_lines = 1;
  cue->settings.position = 50;
  cue->settings.size = 100;
  cue->settings.align = WEBVTT_ALIGN_MIDDLE;
  cue->settings.line.no = WEBVTT_AUTO;
  cue->settings.vertical = WEBVTT_HORIZONTAL;

  *pcue = cue;
  return WEBVTT_SUCCESS;
}

WEBVTT_EXPORT void
webvtt_ref_cue( webvtt_cue *cue )
{
  if( cue ) {
    webvtt_ref( &cue->refs );
  }
}

WEBVTT_EXPORT void
webvtt_release_cue( webvtt_cue **pcue )
{
  if( pcue && *pcue ) {
    webvtt_cue *cue = *pcue;
    *pcue = 0;
    if( webvtt_deref( &cue->refs ) == 0 ) {
      webvtt_release_string( &cue->id );
      webvtt_delete_node( cue->node_head );
      webvtt_free( cue );
    }
  }
}

WEBVTT_EXPORT int
webvtt_validate_cue( webvtt_cue *cue )
{
  if( cue ) {
    /**
     * validate cue-times (Can't do checks against previously parsed cuetimes. That's the applications responsibility
     */
    if( BAD_TIMESTAMP(cue->from) || BAD_TIMESTAMP(cue->until) ) {
      goto error;
    }

    if( cue->until <= cue->from ) {
      goto error;
    }

    /**
     * Don't do any payload validation, because this would involve parsing the payload, which is optional.
     */
    return 1;
  }

error:
  return 0;
}

WEBVTT_INTERN webvtt_status
webvtt_create_node( webvtt_node **node, webvtt_node_kind kind, webvtt_node *parent )
{
  webvtt_node *temp_node = (webvtt_node *)webvtt_alloc0(sizeof(*temp_node));
  
  if( !temp_node )
  { 
    return WEBVTT_OUT_OF_MEMORY; 
  }

  temp_node->kind = kind;
  temp_node->parent = parent;
  *node = temp_node;

  return WEBVTT_SUCCESS;
}

WEBVTT_INTERN webvtt_status
webvtt_create_internal_node( webvtt_node **node, webvtt_node *parent, webvtt_node_kind kind, webvtt_stringlist *css_classes, webvtt_string annotation )
{
  webvtt_status status;
  webvtt_internal_node_data *node_data;

  if( WEBVTT_FAILED( status = webvtt_create_node( node, kind, parent ) ) ) {
    return status;
  }

  node_data = (webvtt_internal_node_data *)webvtt_alloc0( sizeof(*node_data) );
  
  if ( !node_data )
  {
    return WEBVTT_OUT_OF_MEMORY;
  }

  node_data->css_classes = css_classes;
  node_data->annotation = annotation;
  node_data->children = NULL;
  node_data->length = 0;
  node_data->alloc = 0;

  (*node)->data.internal_data = node_data;
  
  return WEBVTT_SUCCESS;
}

WEBVTT_INTERN webvtt_status
webvtt_create_head_node( webvtt_node **node )
{
  webvtt_status status;
  webvtt_string temp_annotation;

  /* This isn't the best way to do this... */
  temp_annotation.d = NULL;

  if( WEBVTT_FAILED( status = webvtt_create_internal_node( node, NULL, WEBVTT_HEAD_NODE, NULL, temp_annotation ) ) ) {
    return status;
  }

  return WEBVTT_SUCCESS;
}

WEBVTT_INTERN webvtt_status
webvtt_create_time_stamp_leaf_node( webvtt_node **node, webvtt_node *parent, webvtt_timestamp time_stamp )
{
  webvtt_status status;

  if( WEBVTT_FAILED( status = webvtt_create_node( node, WEBVTT_TIME_STAMP, parent ) ) ) {
    return status;
  }

  (*node)->data.timestamp = time_stamp;

  return WEBVTT_SUCCESS;
}

WEBVTT_INTERN webvtt_status
webvtt_create_text_leaf_node( webvtt_node **node, webvtt_node *parent, webvtt_string text )
{
  webvtt_status status;

  if( WEBVTT_FAILED( status = webvtt_create_node( node, WEBVTT_TIME_STAMP, parent ) ) ) {
    return status;
  }

  (*node)->data.text = text;

  return WEBVTT_SUCCESS;

}

WEBVTT_INTERN void
webvtt_delete_node( webvtt_node *node )
{
  webvtt_uint i;

  if( node ) {
    if( WEBVTT_IS_VALID_LEAF_NODE( node->kind ) ) {
      if( node->data.text.d ) {
        webvtt_release_string( &node->data.text );
      }
    } else if( WEBVTT_IS_VALID_INTERNAL_NODE( node->kind ) ) {
      webvtt_delete_stringlist( &node->data.internal_data->css_classes );
      if( node->data.internal_data->annotation.d ) {
        webvtt_release_string( &node->data.internal_data->annotation );
      }
      for( i = 0; i < node->data.internal_data->length; i++ ) {
        webvtt_delete_node( *(node->data.internal_data->children + i) );
      }
      webvtt_free( node->data.internal_data );
    }
    webvtt_free( node );
  }
}

WEBVTT_INTERN webvtt_status
webvtt_attach_internal_node( webvtt_node *current, webvtt_node *to_attach )
{
  webvtt_node **arr, **old;

  if( !current || !to_attach ) {
    return WEBVTT_INVALID_PARAM;
  }

  if( current->data.internal_data->length + 1 >= ( current->data.internal_data->alloc / 3 ) * 2 ) {
    webvtt_node **arr = 0;
	  webvtt_node **old = 0;
    current->data.internal_data->alloc = current->data.internal_data->alloc ? current->data.internal_data->alloc * 2 : 8;
    *arr = (webvtt_node *)webvtt_alloc0( sizeof(webvtt_node) * (current->data.internal_data->alloc));

    if( !arr ) {
      return WEBVTT_OUT_OF_MEMORY;
    }

    old = current->data.internal_data->children;
    memcpy( arr, old, current->data.internal_data->length * sizeof(webvtt_node) );
    current->data.internal_data->children = arr;
    webvtt_free( old );
  }

  current->data.internal_data->children[current->data.internal_data->length++] = to_attach;

  return WEBVTT_SUCCESS;
}