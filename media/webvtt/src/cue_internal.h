#ifndef __INTERN_CUE_H__
# define __INTERN_CUE_H__
# include <webvtt/string.h>
# include <webvtt/cue.h>

/**
 * Routines for creating nodes.
 */
WEBVTT_INTERN webvtt_status webvtt_create_node( webvtt_node **node, webvtt_node_kind kind, webvtt_node *parent );
WEBVTT_INTERN webvtt_status webvtt_create_internal_node( webvtt_node **node, webvtt_node *parent, webvtt_node_kind kind, webvtt_stringlist *css_classes, webvtt_string annotation );
/**
 * We probably shouldn't have a 'head node' type. 
 * We should just return a list of node trees...
 */
WEBVTT_INTERN webvtt_status webvtt_create_head_node( webvtt_node **node );
WEBVTT_INTERN webvtt_status webvtt_create_time_stamp_leaf_node( webvtt_node **node, webvtt_node *parent, webvtt_timestamp time_stamp );
WEBVTT_INTERN webvtt_status webvtt_create_text_leaf_node( webvtt_node **node, webvtt_node *parent, webvtt_string text );

/**
 * Routines for deleting nodes.
 */
WEBVTT_INTERN void webvtt_delete_node( webvtt_node *node );
WEBVTT_INTERN void webvtt_delete_leaf_node( webvtt_node *leaf_node );
WEBVTT_INTERN void webvtt_delete_internal_node( webvtt_node *internal_node );

/**
 * Attaches a node to the internal node list of another node.
 */
WEBVTT_INTERN webvtt_status webvtt_attach_internal_node( webvtt_node *current, webvtt_node *to_attach );

/**
 * Private cue flags
 */
enum {
  CUE_HAVE_VERTICAL = (1 << 0),
  CUE_HAVE_SIZE = (1 << 1),
  CUE_HAVE_POSITION = (1 << 2),
  CUE_HAVE_LINE = (1 << 3),
  CUE_HAVE_ALIGN = (1 << 4),

  CUE_HAVE_SETTINGS = (CUE_HAVE_VERTICAL | CUE_HAVE_SIZE | CUE_HAVE_POSITION | CUE_HAVE_LINE | CUE_HAVE_ALIGN),

  CUE_HAVE_CUEPARAMS = 0x40000000,
  CUE_HAVE_ID = 0x80000000,
};

#endif