/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * dot.h
 * Copyright (C) 2017 Anoop Kumar Narayanan <anoop.kn@live.in>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Anoop Kumar Narayanan'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * libdot IS PROVIDED BY Anoop Kumar Narayanan ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Anoop Kumar Narayanan OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*This is a DOM parser for .dot file */

#ifndef _DOT_H_
#define _DOT_H_

#define DOT_MAXLINE_BUF 65536

typedef enum _DOT_NODE_TYPE {
    DOT_NODE_ROOT            = 0,   //DOT node root node
    DOT_NODE_CONFIGURATION,         //DOT node configuration
    DOT_NODE_COMMENT,               //DOT node lines starting with space
    DOT_NODE_EMPTY,                 //DOT node empty line
    DOT_NODE_SPACELINE,             //DOT node space, tab, carage return line
    DOT_NODE_MARKER,                //DOT node marker selector
    DOT_NODE_OPERATION,             //DOT node operation
    DOT_NODE_ELEMENT,               //DOT node element
    DOT_NODE_ATTRIBUTE,             //DOT node normal attribute
    DOT_NODE_TEXTATTRIBUTE,         //DOT node text/textdata attribute
    DOT_NODE_LISTATTRIBUTE,
    DOT_NODE_RESULTATTRIBUTE,
    DOT_NODE_RESULTLISTATTRIBUTE,
    DOT_NODE_BLOBATTRIBUTE,
    DOT_NODE_CLOBATTRIBUTE,
    DOT_NODE_MAXTYPE
} DOT_NODE_TYPE;

typedef enum _DOT_NODE_STATE {
    DOT_NODE_NORMAL   = 0,
    DOT_NODE_DELETED,
    DOT_NODE_HIDDEN,
} DOT_NODE_STATE;

typedef struct _DOT_NODE_LIST {
    char *value;
    struct _DOT_NODE_LIST *nextNode;
} DOT_NODE_LIST;

typedef struct _DOT_NODE {
    DOT_NODE_TYPE     dotNodeType;
    DOT_NODE_STATE    dotNodeState;
    unsigned int      dotNodeDepth;
    unsigned int      dotNodeEvaluated;
    unsigned int      dotNodeValueSize; //set only for BLOBs
    unsigned int      dotNodeIdHash;    //to speed up marker lookup
    char*             dotNodeId;
    char*             dotNodeName;
    char*             dotNodeValue;
    DOT_NODE_LIST     *tags;
    DOT_NODE_LIST     *list;
    struct _DOT_NODE  *dotNodeParent;
    struct _DOT_NODE  *dotNodeFirstChild;
    struct _DOT_NODE  *dotNodeNextSibling;
    struct _DOT_NODE  *dotNodePrevSibling;
    void *             *dotNodeUserData;
} DOT_NODE;

typedef enum _DOT_CURSOR {
    DOT_CURSOR_ZERO = 0,
    DOT_CURSOR_DOT,
    DOT_CURSOR_ELEMENT,
    DOT_CURSOR_SPACE,
    DOT_CURSOR_NAME,
    DOT_CURSOR_SEPARATOR,
    DOT_CURSOR_VALUE,
    DOT_CURSOR_OPERATOR,
    DOT_CURSOR_COMMA,
    DOT_CURSOR_ESCAPE,
    DOT_CURSOR_ARGTYPE,
    DOT_CURSOR_LOWDASH,
    DOT_CURSOR_DATA,
    DOT_CURSOR_EOL
} DOT_CURSOR;

typedef struct _DOT_PARSER {
    char           c;
    char           lastc;
    char*          name;
    char*          value;
    unsigned int   buflen;
    unsigned int   index;
    unsigned int   line;
    unsigned int   nodeCount;
    unsigned int   location;
    unsigned int   depth;
    /*
    DOT_LINE_TYPE  type;
    DOT_LINE_STAGE stage;
    DOT_CURSOR     cursor;
    */
    //For Parsing
    DOT_NODE       *docRoot;
    DOT_NODE       *previousNode;
    DOT_NODE       *currentNode;
    DOT_NODE       *lastNode;

    //For Operations
    DOT_NODE       *fromNode;
    DOT_NODE       *toNode;
} DOT_PARSER;

DOT_PARSER* dot_parser_new();
void        dot_parser_delete ( DOT_PARSER * );
DOT_NODE*   dot_parser_get_root_node ( DOT_PARSER * );
DOT_NODE*   dot_parser_get_current_node ( DOT_PARSER * );

//To be used in conjunction with getline(), the string buffer must be freed
//Each line should contain a EOL followed by NULL Termination
int         dot_parser_parse_line ( DOT_PARSER *, char *, unsigned int );

int         dot_parser_parse_file ( DOT_PARSER *, char * );

int         dot_parser_dump ( DOT_PARSER *parser, DOT_NODE *, unsigned int );
int         dot_parser_pretty_print ( DOT_PARSER *parser, DOT_NODE *, unsigned int );

unsigned int dot_parser_get_id_hash ( DOT_PARSER *parser, char * );

DOT_NODE*   dot_parser_get_node ( DOT_PARSER *parser, DOT_NODE *node, char * );

DOT_NODE*   dot_parser_get_node_by_id ( DOT_PARSER *parser, DOT_NODE *node, char *, unsigned int );

#endif
