#ifndef __WEBVTT_CUE_H__
# define __WEBVTT_CUE_H__
# include "util.h"
# include <webvtt/string.h>

#if defined(__cplusplus) || defined(c_plusplus)
#define WEBVTT_CPLUSPLUS 1
extern "C" {
#endif

#define WEBVTT_AUTO (0xFFFFFFFF)

typedef enum
webvtt_node_kind_t {
  WEBVTT_NODE_LEAF = 0x80000000,
  WEBVTT_NODE_INTERNAL = 0x00000000,

  /**
    * Internal Node objects
    */
  WEBVTT_NODE_INTERNAL_START = 0,
  WEBVTT_CLASS = 0 | WEBVTT_NODE_INTERNAL,
  WEBVTT_ITALIC = 1 | WEBVTT_NODE_INTERNAL,
  WEBVTT_BOLD = 2 | WEBVTT_NODE_INTERNAL,
  WEBVTT_UNDERLINE = 3 | WEBVTT_NODE_INTERNAL,
  WEBVTT_RUBY = 4 | WEBVTT_NODE_INTERNAL,
  WEBVTT_RUBY_TEXT = 5 | WEBVTT_NODE_INTERNAL,
  WEBVTT_VOICE = 6 | WEBVTT_NODE_INTERNAL,

  /**
    * This type of node has should not be rendered.
    * It is the top of the node list and only contains a list of nodes.
    */
  WEBVTT_HEAD_NODE = 7,

  WEBVTT_NODE_INTERNAL_END = 7,

  /**
    * Leaf Node objects
    */
  WEBVTT_NODE_LEAF_START = 256,
  WEBVTT_TEXT = 256 | WEBVTT_NODE_LEAF,
  WEBVTT_TIME_STAMP = 257 | WEBVTT_NODE_LEAF,

  WEBVTT_NODE_LEAF_END = 257
} webvtt_node_kind;

/**
  * Macros to assist in validating node kinds, so that C++ compilers don't complain (as long as they provide reinterpret_cast, which they should)
  */
#if defined(__cplusplus) || defined(__cplusplus_cli) || defined(__embedded_cplusplus) || defined(c_plusplus)
# define WEBVTT_CAST(Type,Val) (reinterpret_cast<Type>(Val))
#else
# define WEBVTT_CAST(Type,Val) ((Type)(Val))
#endif

#define WEBVTT_IS_LEAF(Kind) ( ((Kind) & WEBVTT_NODE_LEAF) != 0 )
#define WEBVTT_NODE_INDEX(Kind) ( (Kind) & ~WEBVTT_NODE_LEAF )
#define WEBVTT_IS_VALID_LEAF_NODE(Kind) ( WEBVTT_IS_LEAF(Kind) && (WEBVTT_NODE_INDEX(Kind) >= WEBVTT_NODE_LEAF_START && WEBVTT_NODE_INDEX(Kind) <= WEBVTT_NODE_LEAF_END ) )
#define WEBVTT_IS_VALID_INTERNAL_NODE(Kind) ( (!WEBVTT_IS_LEAF(Kind)) && (WEBVTT_NODE_INDEX(Kind) >= WEBVTT_NODE_INTERNAL_START && WEBVTT_NODE_INDEX(Kind) <= WEBVTT_NODE_INTERNAL_END) )
#define WEBVTT_IS_VALID_NODE_KIND(Kind) ( WEBVTT_IS_VALID_INTERNAL_NODE(Kind) || WEBVTT_IS_VALID_LEAF_NODE(Kind) )

/**
  * Casting helpers
  */
#define WEBVTT_GET_INTERNAL_NODE(Node) ( WEBVTT_IS_VALID_INTERNAL_NODE(WEBVTT_CAST(webvtt_node,Node)->kind) ? WEBVTT_CAST(webvtt_internal_node,Node) : 0 )
#define WEBVTT_GET_LEAF_NODE(Node) ( WEBVTT_IS_VALID_LEAF_NODE((WEBVTT_CAST(webvtt_node,Node))->kind) ? WEBVTT_CAST(webvtt_leaf_node,Node) : 0 )

struct webvtt_internal_node_data_t;

typedef enum
webvtt_vertical_type_t {
  WEBVTT_HORIZONTAL = 0,
  WEBVTT_VERTICAL_LR = 1,
  WEBVTT_VERTICAL_RL = 2
} webvtt_vertical_type;

typedef enum
webvtt_align_type_t {
  WEBVTT_ALIGN_START = 0,
  WEBVTT_ALIGN_MIDDLE,
  WEBVTT_ALIGN_END,

  WEBVTT_ALIGN_LEFT,
  WEBVTT_ALIGN_RIGHT
} webvtt_align_type;

typedef struct
webvtt_node_t {
  /**
    * The specification asks for uni directional linked list, but we have added a parent
    * node in order to facilitate an iterative cue text parsing solution.
    */
  struct webvtt_node_t *parent;
  webvtt_node_kind kind;

  union {
    webvtt_string text;
    webvtt_timestamp timestamp;
    struct webvtt_internal_node_data_t *internal_data;
  } data;
} webvtt_node;

typedef struct
webvtt_internal_node_data_t {
  webvtt_string annotation;
  webvtt_stringlist *css_classes;

  webvtt_uint alloc;
  webvtt_uint length;
  webvtt_node **children;
} webvtt_internal_node_data;

typedef struct
webvtt_cue_settings_t {
  webvtt_vertical_type vertical;
  webvtt_bool line_is_relative;
  union {
    webvtt_int no; /* line.no */
    webvtt_uint relative_position; /* line.relative_position */
  } line;
  webvtt_uint position;
  webvtt_uint size;
  webvtt_align_type align;
} webvtt_cue_settings;

typedef struct
webvtt_cue_t {
  /**
    * PRIVATE.
    * Do not touch, okay?
    */
  struct webvtt_refcount_t refs;
  webvtt_uint flags;

  /**
    * PUBLIC:
    */
  webvtt_timestamp from;
  webvtt_timestamp until;
  webvtt_cue_settings settings;
  webvtt_bool snap_to_lines;
  webvtt_string id;

  /**
    * Parsed cue-text (NULL if has not been parsed)
    */
  webvtt_node *node_head;
} webvtt_cue;

WEBVTT_EXPORT webvtt_status webvtt_create_cue( webvtt_cue **pcue );
WEBVTT_EXPORT void webvtt_ref_cue( webvtt_cue *cue );
WEBVTT_EXPORT void webvtt_release_cue( webvtt_cue **pcue );
WEBVTT_EXPORT int webvtt_validate_cue( webvtt_cue *cue );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif