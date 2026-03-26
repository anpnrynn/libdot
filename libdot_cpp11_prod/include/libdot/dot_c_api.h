#ifndef LIBDOT_DOT_C_API_H
#define LIBDOT_DOT_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DOT_PARSER DOT_PARSER;

typedef struct DOT_NODE_LIST {
    char *value;
    struct DOT_NODE_LIST *nextNode;
} DOT_NODE_LIST;

typedef enum _DOT_NODE_TYPE {
    DOT_NODE_ROOT = 0,
    DOT_NODE_CONFIGURATION,
    DOT_NODE_COMMENT,
    DOT_NODE_EMPTY,
    DOT_NODE_SPACELINE,
    DOT_NODE_MARKER,
    DOT_NODE_OPERATION,
    DOT_NODE_ELEMENT,
    DOT_NODE_ATTRIBUTE,
    DOT_NODE_TEXTATTRIBUTE,
    DOT_NODE_LISTATTRIBUTE,
    DOT_NODE_RESULTATTRIBUTE,
    DOT_NODE_RESULTLISTATTRIBUTE,
    DOT_NODE_BLOBATTRIBUTE,
    DOT_NODE_CLOBATTRIBUTE,
    DOT_NODE_MAXTYPE
} DOT_NODE_TYPE;

typedef enum _DOT_NODE_STATE {
    DOT_NODE_NORMAL = 0,
    DOT_NODE_DELETED,
    DOT_NODE_HIDDEN
} DOT_NODE_STATE;

typedef struct DOT_NODE {
    DOT_NODE_TYPE dotNodeType;
    DOT_NODE_STATE dotNodeState;
    unsigned int dotNodeDepth;
    unsigned int dotNodeEvaluated;
    unsigned int dotNodeValueSize;
    unsigned int dotNodeIdHash;
    char* dotNodeId;
    char* dotNodeName;
    char* dotNodeValue;
    DOT_NODE_LIST *tags;
    DOT_NODE_LIST *list;
    struct DOT_NODE *dotNodeParent;
    struct DOT_NODE *dotNodeFirstChild;
    struct DOT_NODE *dotNodeNextSibling;
    struct DOT_NODE *dotNodePrevSibling;
    void **dotNodeUserData;
} DOT_NODE;

DOT_PARSER* dot_parser_new(void);
void dot_parser_delete(DOT_PARSER* parser);
DOT_NODE* dot_parser_get_root_node(DOT_PARSER* parser);
DOT_NODE* dot_parser_get_current_node(DOT_PARSER* parser);
int dot_parser_parse_line(DOT_PARSER* parser, char* line, unsigned int len);
int dot_parser_parse_file(DOT_PARSER* parser, char* path);
int dot_parser_dump(DOT_PARSER* parser, DOT_NODE* node, unsigned int flags);
int dot_parser_pretty_print(DOT_PARSER* parser, DOT_NODE* node, unsigned int flags);
unsigned int dot_parser_get_id_hash(DOT_PARSER* parser, char* id);
DOT_NODE* dot_parser_get_node(DOT_PARSER* parser, DOT_NODE* node, char* name);
DOT_NODE* dot_parser_get_node_by_id(DOT_PARSER* parser, DOT_NODE* node, char* id, unsigned int hash);
const char* dot_parser_last_dump(DOT_PARSER* parser);

#ifdef __cplusplus
}
#endif

#endif
