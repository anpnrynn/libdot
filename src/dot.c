/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * dot.c
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
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
//#include <threads.h>
//thread_local int foo = 0;
#include <dot.h>


typedef enum _DOT_LINE_TYPE {
	DOT_LINE_ELEMENT = 0,     
	DOT_LINE_ELEMENTLINK,    
	DOT_LINE_ATTRIBUTE,
	DOT_LINE_POPELEMENT,
	DOT_LINE_OPERATION,
} DOT_LINE_TYPE;

static DOT_NODE_LIST*   dot_node_list_new(){
	DOT_NODE_LIST *nodeTag = (DOT_NODE_LIST *)malloc(sizeof(DOT_NODE_LIST));
	if( nodeTag ){
		memset(nodeTag, 0, sizeof(DOT_NODE_LIST) );
	}
	return nodeTag;
}

static void dot_node_delete(DOT_NODE_LIST *nodeTag) {
	if( nodeTag ) {
		free(nodeTag);
	}
}

static void dot_node_list_delete( DOT_NODE_LIST *nodeTag ){
	DOT_NODE_LIST *tmpTag = nodeTag;
	while( nodeTag ){
		tmpTag = nodeTag->nextNode;
		free( nodeTag->value );
		nodeTag->value     = 0;
		nodeTag->nextNode = 0;
		free(nodeTag);
		nodeTag = tmpTag;
	}
}

static DOT_NODE_LIST *dot_node_list_duplicate( DOT_NODE_LIST *fromListNode ){
	DOT_NODE_LIST *tmpListNode = 0, *tmpLastListNode = 0, *firstListNode = 0;
	while( fromListNode ){
		tmpListNode = dot_node_list_new ();
		tmpListNode->value = fromListNode->value?strdup( fromListNode->value ):0;
		if( !firstListNode )
			firstListNode = tmpListNode;
		if( tmpLastListNode ){
			tmpLastListNode->nextNode = tmpListNode;
		}
		tmpLastListNode = tmpListNode;
		fromListNode = fromListNode->nextNode;
	}
	return firstListNode;
}

static int dot_node_list_get_values ( DOT_NODE_LIST *list, char **value1, char **value2, char **value3, char **value4 ){
	*value1 = *value2 = *value3 = *value4 = 0;
	if (!list ){
		return 0;
	} else {
		*value1 = list->value;
		if( list->nextNode ){
			*value2 = list->nextNode->value;
			if( list->nextNode->nextNode ){
				*value3 = list->nextNode->nextNode->value;
				if( list->nextNode->nextNode->nextNode){
					*value4 = list->nextNode->nextNode->nextNode->value;
					return 4;
				}
				return 3;
			}
			return 2;
		}
		return 1;
	}
}

DOT_NODE*   dot_node_new(DOT_PARSER *parser){
	DOT_NODE *node = (DOT_NODE *)malloc(sizeof(DOT_NODE));
	if( node ){
		memset(node, 0, sizeof(DOT_NODE) );
		(parser->nodeCount)++;
	}
	return node;
}

static char* dot_node_id_dup( DOT_PARSER *parser, char *id ){
	char *dupId = 0;
	int   n = strlen(id);
	dupId = (char *) malloc ( n + 24 );
	sprintf( dupId,"%s-%010d", id, parser->nodeCount  );
	return dupId;
}

static void dot_node_delete_fix_nodes ( DOT_PARSER *parser, DOT_NODE *node){
	if( !node )
		return;
	if( node->dotNodePrevSibling ){
		node->dotNodePrevSibling->dotNodeNextSibling = node->dotNodeNextSibling;
	} else{
		if( node->dotNodeNextSibling ){
			node->dotNodeNextSibling->dotNodePrevSibling = 0;
			if( node->dotNodeParent )
				node->dotNodeParent->dotNodeFirstChild = node->dotNodeNextSibling;
		}
	}
	if( node->dotNodeNextSibling ){
		node->dotNodeNextSibling->dotNodePrevSibling = node->dotNodePrevSibling;
	} else {
		if( node->dotNodePrevSibling ){
			node->dotNodePrevSibling->dotNodeNextSibling = 0;
		}
	}
	node->dotNodePrevSibling = 0;
	node->dotNodeNextSibling = 0;
	node->dotNodeParent      = 0;
}

void dot_node_delete_tree(DOT_PARSER *parser, DOT_NODE *node, int freeUserData ){
	if(!node)
		return;
	DOT_NODE *child = node->dotNodeFirstChild;
	DOT_NODE *nextChild = 0;
	while( child ){
		nextChild = child->dotNodeNextSibling;
		dot_node_delete_tree ( parser, child, freeUserData );
		child = nextChild;
	}
	if( node->tags )
		dot_node_list_delete  ( node->tags);
	if( node->list )
		dot_node_list_delete  ( node->list );
	if( node->dotNodeName )
		free( node->dotNodeName );
	if( node->dotNodeId )
		free( node->dotNodeId );
	if( node->dotNodeValue )
		free( node->dotNodeValue );
	if( freeUserData)
		free( node->dotNodeUserData );
	dot_node_delete_fix_nodes ( parser, node );
	free (node);
}

static void dot_node_merge_tags ( DOT_PARSER *parser, DOT_NODE *origNode, DOT_NODE_LIST *newTag ){
	DOT_NODE_LIST *tag = origNode->tags;
	DOT_NODE_LIST *lastTag = 0;
	while( tag ){
		lastTag = tag;
		tag = tag->nextNode;
	}
	if (lastTag){
		lastTag->nextNode = newTag;
	}
	return;
}

static void dot_node_merge_list ( DOT_PARSER *parser, DOT_NODE *origNode, DOT_NODE_LIST *list ){
	DOT_NODE_LIST *listNode = origNode->list;
	DOT_NODE_LIST *lastListNode = 0;
	while( listNode ){
		lastListNode = listNode;
		listNode = listNode->nextNode;
	}
	if (lastListNode){
		lastListNode->nextNode = list;
	}
	return;
}

typedef enum _DOT_NODE_STRTYPE {
	DOT_NODE_STR_NAME = 0,
	DOT_NODE_STR_ID,
	DOT_NODE_STR_VALUE,
} DOT_NODE_STRTYPE;

static void dot_node_merge_strings( DOT_PARSER *parser, DOT_NODE *toNode, DOT_NODE *fromNode, DOT_NODE_STRTYPE which ){
	char **toString=0;
	char **fromString=0;
	if( toNode && fromNode ){
		switch( which ){
			case DOT_NODE_STR_NAME:
				toString = &toNode->dotNodeName;
				fromString = &fromNode->dotNodeName;
				break;
			case DOT_NODE_STR_ID:
				toString = &toNode->dotNodeId;
				fromString = &fromNode->dotNodeId;
				break;
			case DOT_NODE_STR_VALUE:
				toString = &toNode->dotNodeValue;
				fromString = &fromNode->dotNodeValue;
				break;
			default:
				return;
		}
		
	} else 
	if ( fromNode ){
		toString = 0;
		switch( which ){
			case DOT_NODE_STR_NAME:
				fromString = &fromNode->dotNodeName;
				break;
			case DOT_NODE_STR_ID:
				fromString = &fromNode->dotNodeId;
				break;
			case DOT_NODE_STR_VALUE:
				fromString = &fromNode->dotNodeValue;
				break;
			default:
				return;
		}
	} else {
		return;
	}

	if( *toString ){
		if( *fromString ){
			*toString = (char*) realloc( *toString,
			                              strlen(*toString) +
			                              strlen(*fromString) +
			                              1 );
			strcat( *toString, *fromString );
			return;
		} else {
			return; //nothing to do
		}
	} else {
		if( *fromString ){
			*toString = strdup( *fromString );
			return;
		} else {
			return; //nothing to do
		}
	}
}

static inline void dot_node_copy(DOT_PARSER *parser, DOT_NODE *to, DOT_NODE *from, int depth){
	if( to && from ){
		to->dotNodeType           = from->dotNodeType;
		to->dotNodeState          = from->dotNodeState;
		to->dotNodeDepth          = depth;
		to->dotNodeEvaluated      = from->dotNodeEvaluated;
		to->dotNodeValueSize      = from->dotNodeValueSize; //set only for BLOBs
		to->dotNodeIdHash         = from->dotNodeIdHash;    //to speed up marker lookup
		to->dotNodeId             = from->dotNodeId?dot_node_id_dup(parser, from->dotNodeId):0;
		to->dotNodeName           = from->dotNodeName?strdup(from->dotNodeName):0;
		to->dotNodeValue          = from->dotNodeValue?strdup(from->dotNodeValue):0;
		to->tags                  = dot_node_list_duplicate(from->tags);
		to->list                  = dot_node_list_duplicate(from->list);
		to->dotNodeParent         = 0;
		to->dotNodeFirstChild     = 0;
		to->dotNodeNextSibling    = 0;
		to->dotNodePrevSibling    = 0;
		to->dotNodeUserData       = from->dotNodeUserData;
	} else 
		return;
}

static DOT_NODE* dot_node_duplicate_node( DOT_PARSER *parser, DOT_NODE *dupNode, unsigned int depth ){
	if ( dupNode ){
		DOT_NODE *tmpNode = dot_node_new(parser);
		dot_node_copy (parser, tmpNode, dupNode, depth+1);
		return tmpNode;
	}
	return 0;
}

static DOT_NODE* dot_node_duplicate_tree( DOT_PARSER *parser, DOT_NODE *parent, DOT_NODE *dupNode, unsigned int depth ){
	if(! dupNode )
		return 0;
	else{
		DOT_NODE *tmpFirstChild = 0;
		DOT_NODE *tmpLastNode   = 0;
		while( dupNode ){
			DOT_NODE *tmpNode = dot_node_new(parser);
			if( !tmpFirstChild )
				tmpFirstChild = tmpNode;
			dot_node_copy (parser, tmpNode, dupNode, depth+1);
			tmpNode->dotNodeParent = parent;
			tmpNode->dotNodeFirstChild = dot_node_duplicate_tree ( parser, tmpNode, dupNode->dotNodeFirstChild, depth+1);
			if( tmpLastNode ) {
				tmpLastNode->dotNodeNextSibling = tmpNode;
				tmpNode->dotNodePrevSibling = tmpLastNode;
			}
			tmpLastNode = tmpNode;
			dupNode = dupNode->dotNodeNextSibling;
		}
		return tmpFirstChild;
	}
}

static void dot_node_add_child(DOT_PARSER *parser, DOT_NODE *parent, DOT_NODE *current, int isList ){
	if(!current)
		return;
	DOT_NODE *child = parent->dotNodeFirstChild;
	DOT_NODE *lastChild = 0;
	if( !child ) {
		parent->dotNodeFirstChild = current;
		if( !isList ){
			current->dotNodeNextSibling = 0;
			current->dotNodePrevSibling = 0;
		}
		current->dotNodeParent = parent;
		return;
	}
	while ( child ){
		lastChild = child;
		child = child->dotNodeNextSibling;
	}
	if ( lastChild ) {
		lastChild->dotNodeNextSibling = current;
		current->dotNodePrevSibling = lastChild;
		if(!isList){
			current->dotNodeNextSibling = 0;
		} else {
			child = current;
			while ( child ){
				child->dotNodeParent = parent;
				child = child->dotNodeNextSibling;
			}
		}
		current->dotNodeParent = parent;
	}
	return;
}

static DOT_NODE *dot_node_attribute_find( DOT_PARSER *parser, DOT_NODE *start, char *attrName, char *attrIndex ){
	DOT_NODE *tmpStart = start;
	int   attrIndexInt    = attrIndex?abs(atoi(attrIndex)):0;
	int   curAttrIndexInt = 0;
	while ( tmpStart ){
		if( tmpStart->dotNodeType > DOT_NODE_ELEMENT &&
		    tmpStart->dotNodeType < DOT_NODE_MAXTYPE &&
		    strcmp( tmpStart->dotNodeName, attrName ) == 0 ){

			if ( attrIndexInt != curAttrIndexInt ){
				curAttrIndexInt++;
				tmpStart = tmpStart->dotNodeNextSibling;
				continue;
			}

			return tmpStart;
		}
		tmpStart = tmpStart->dotNodeNextSibling;
	}
	return 0;
}


static void dot_node_attribute_plus_attribute_value ( DOT_PARSER *parser, DOT_NODE *marked, DOT_NODE *markedFrom, DOT_NODE_LIST *list ){
	char *attrName = list->value;
	if(!attrName)
		return;
	char *attrIndex = list->nextNode?list->nextNode->value:0;
	if(!attrIndex)
		return;
	DOT_NODE_LIST *secondAttr = list->nextNode->nextNode?list->nextNode->nextNode:0;
	if(!secondAttr)
		return;
	else {
		char *attrName2 = secondAttr->value;
		if(!attrName2)
			return;
		char *attrIndex2 = secondAttr->nextNode?secondAttr->nextNode->value:0;
		if(!attrIndex2) {
			// Assume this is append to toAttrNode and the 3rd param is actually a value.
			DOT_NODE *toAttrNode = dot_node_attribute_find (parser, marked->dotNodeFirstChild, attrName, attrIndex  );
			if(!toAttrNode)
				return;
			if( toAttrNode->dotNodeValue ){
				toAttrNode->dotNodeValue = (char *) realloc( toAttrNode->dotNodeValue , 
				                                        strlen(toAttrNode->dotNodeValue)
					                                    +strlen(attrName2)
					                                    +1 );
				strcat( toAttrNode->dotNodeValue , attrName2 );
			} else {
				marked->dotNodeValue = attrName2;
				secondAttr->value = 0; //its anyway strdup-ed 
			}
		} else {
			// All 4 values present, merge or copy the value fromAttrNode to toAttrNode
			DOT_NODE *toAttrNode = dot_node_attribute_find (parser, marked->dotNodeFirstChild, attrName, attrIndex  );
			if(!toAttrNode)
				return;
			DOT_NODE *fromAttrNode = dot_node_attribute_find (parser, markedFrom, attrName2, attrIndex2);
			if(!fromAttrNode)
				return;
			if( toAttrNode->dotNodeValue && fromAttrNode->dotNodeValue ) {
				//merge the value
				toAttrNode->dotNodeValue = (char *) realloc( toAttrNode->dotNodeValue,
				                                         strlen(toAttrNode->dotNodeValue)
					                                    +strlen(fromAttrNode->dotNodeValue)
					                                    +1 );
				strcat( toAttrNode->dotNodeValue , fromAttrNode->dotNodeValue );
			} else 
			if ( fromAttrNode->dotNodeValue ) {
				//copy from fromAttrNode to toAttrNode
				toAttrNode->dotNodeValue = strdup(fromAttrNode->dotNodeValue);
			} else {
				//nothing to do
				return;
			}
		}
	}
}

static void dot_node_attribute_minus_attribute_value ( DOT_PARSER *parser, DOT_NODE *marked, DOT_NODE_LIST *list ){
	char *attrName  = list->value;
	if( !attrName )
		return;
	if( strcmp(attrName, "#" ) == 0 ) {
		dot_node_list_delete (marked->tags?marked->tags:0);
		marked->tags = 0;
		return;
	}
	int   removeAll = 0;
	char *attrIndex = 0;
	if( !list->nextNode )
		removeAll = 1;
	else
		attrIndex = list->nextNode?list->nextNode->value:0;
	int   attrIndexInt    = attrIndex?abs(atoi(attrIndex)):0;
	int   curAttrIndexInt = 0;
	DOT_NODE *tmpMarked   = marked->dotNodeFirstChild;
	DOT_NODE *tmpMarkedNext = 0;
	while( tmpMarked ){
		tmpMarkedNext = tmpMarked->dotNodeNextSibling;
		if( tmpMarked->dotNodeType > DOT_NODE_ELEMENT &&
		    tmpMarked->dotNodeType < DOT_NODE_MAXTYPE &&
		    strcmp( tmpMarked->dotNodeName, attrName ) == 0 ){
			if ( !removeAll && attrIndexInt != curAttrIndexInt ){
				curAttrIndexInt++;
				tmpMarked = tmpMarkedNext;
				continue;
			}
			dot_node_delete_fix_nodes (parser, tmpMarked );	
			dot_node_delete_tree(parser, tmpMarked, 0);
			if(!removeAll)
				return;
		}
		tmpMarked = tmpMarkedNext;
	}
}

static void dot_node_merge  (DOT_PARSER *parser, DOT_NODE *original, DOT_NODE *current ){

	dot_node_merge_tags (parser, original, current->tags );
	dot_node_add_child  (parser, original, current->dotNodeFirstChild, 1 );
//	#if 0
	original->dotNodeState = current->dotNodeState;
	char * origval = original->dotNodeValue;
	char * newval  = current->dotNodeValue;
	if( origval !=0 && newval != 0 ){
		if( original->dotNodeType == DOT_NODE_BLOBATTRIBUTE ){
			int newsize = original->dotNodeValueSize + current->dotNodeValueSize;
			original->dotNodeValue = (char*) malloc( newsize );
			memcpy ( original->dotNodeValue, origval, original->dotNodeValueSize );
			memcpy( &(original->dotNodeValue[original->dotNodeValueSize]), newval, current->dotNodeValueSize );
			original->dotNodeValueSize = newsize;
		} else {
			original->dotNodeValue = (char*) malloc( strlen(origval) + strlen(newval) + 1 );
			strcpy( original->dotNodeValue , origval );
			strcat( original->dotNodeValue , newval );
		}
		free ( origval );
		free ( newval );
	}
	memset( current , 0, sizeof(DOT_NODE) );
//#endif
	free(current);
}

DOT_PARSER* dot_parser_new(){
	DOT_PARSER *parser = (DOT_PARSER *)malloc(sizeof(DOT_PARSER));
	if( parser ){
		memset(parser, 0, sizeof(DOT_PARSER) );
		parser->lastc = '\n';
	}
	parser->docRoot = dot_node_new (parser);
	parser->docRoot->dotNodeName = strdup("root");
	parser->docRoot->dotNodeType = DOT_NODE_ROOT;
	return parser;
}

void        dot_parser_delete(DOT_PARSER *parser){
	if( parser ) {
		dot_node_delete_tree(parser, parser->docRoot, 0);
		bzero( parser, sizeof(DOT_PARSER) );
		free(parser);
	}
}

DOT_NODE*   dot_parser_get_root_node ( DOT_PARSER *parser){
	if( parser ){
		return parser->docRoot;
	}
	return 0;
}

DOT_NODE*   dot_parser_get_previous_node ( DOT_PARSER *parser){
	if( parser ){
		return parser->previousNode;
	}
	return 0;
}

DOT_NODE*   dot_parser_get_current_node ( DOT_PARSER *parser){
	if( parser ){
		return parser->currentNode;
	}
	return 0;
}

static inline DOT_NODE*    dot_parser_get_node_compare ( DOT_PARSER *parser, DOT_NODE *node, char *str ){
	if ( strcmp( node->dotNodeName?node->dotNodeName:"", str) == 0 )
		return node;
	else
		return 0;
}

DOT_NODE*    dot_parser_get_node ( DOT_PARSER *parser, DOT_NODE *node, char *str ){
	if( node ){
		DOT_NODE * tmpNode = node, * searchResult = 0;
		do {
			if ( searchResult = dot_parser_get_node_compare (parser, tmpNode, str) )
				return searchResult;
			if ( tmpNode->dotNodeFirstChild ){
				dot_parser_get_node (parser, tmpNode->dotNodeFirstChild, str);
			}
			tmpNode = tmpNode->dotNodeNextSibling;
		}while ( tmpNode );
	}
	return 0;
}

const unsigned int magic = 0x1a2b3c4d;


//A simple 32bit XOR hash
inline unsigned int dot_parser_get_id_hash( DOT_PARSER *parser, char *str){
	unsigned int   hash = magic;
	unsigned char *hashPtr = (unsigned char *)&hash;
	int hashIndex = 0;
	int i = 0;
	char ch = str[i];
	if(!ch)
		return hash;
	do{
		hashIndex = i%4;
		hashPtr[hashIndex] ^= ((unsigned char)ch);
		ch = str[i++];
	} while( ch ); 
	
	return hash;
}

static inline DOT_NODE*    dot_parser_get_node_compare_by_id ( DOT_PARSER *parser, DOT_NODE *node, char *str , unsigned int hash){
	if ( hash == node->dotNodeIdHash ){
		if ( strcmp( node->dotNodeId?node->dotNodeId:"", str) == 0 )
			return node;
		else
			return 0;
	} else {
		return 0;
	}
}

DOT_NODE*    dot_parser_get_node_by_id ( DOT_PARSER *parser, DOT_NODE *node, char *str, unsigned int hash ){
	if( node ){
		DOT_NODE * tmpNode = node, * searchResult = 0;
		do {
			if ( searchResult = dot_parser_get_node_compare_by_id (parser, tmpNode, str, hash) )
				return searchResult;
			if ( tmpNode->dotNodeFirstChild ){
				searchResult = dot_parser_get_node_by_id (parser, tmpNode->dotNodeFirstChild, str, hash);
				if ( searchResult )
					return searchResult;
			}
			tmpNode = tmpNode->dotNodeNextSibling;
		}while ( tmpNode );
	}
	return 0;
}

static inline char dot_getch( char *line, int *i){
	return line[(*i)++];
}

static inline void dot_ungetch ( int *i ){
	(*i)--;
}

static int read_till_end_of_char( char *line , char separator){
	int i = 0;
	while ( line[i] == separator && line[i] != '\0' ){
		i++;
	}
	return i;
}

static int strcpy_till_char( char *result, char *line, char separator ){
	int  i = 0, j = 0;
	char lastc    = 0;
	char c = dot_getch( line , &i );
	while ( 1 ){
		if( ( c == separator && lastc != '`' ) || c == '\0' )
			break;
		if ( c == '`' ){
			if ( lastc == 0 ){
				char nextc = dot_getch( line, &i);
				if( nextc == separator || nextc == 0 ){
					dot_ungetch( &i );
				} else {
					dot_ungetch( &i );
					lastc = c;
				}
			} else {
				if( lastc == '`' ){
					//save char
					result[j++] = c;
					lastc = 0;
				} else {
					lastc = c;
				}
			}
		} else {
			if( lastc == '`' ){
				if ( c == '_' ){
					result[j++] = c;
					lastc = c;
					//save char
				} else 
				if ( c == 'n' || c == 't' || c == 'r' ){
					char bkpc = c;
					c=(c=='n'?'\n':(c=='t'?'\t':(c=='r'?'\r':0)));
					result[j++] = c;
					lastc = bkpc;
					//save char
				} else 
				if ( c == separator ){
					result[j++] = c;
					lastc = c;
				} else {
					//save lastc 
					//save char
					result[j++] = lastc;
					result[j++] = c;
					lastc = c;
				}
			} else {
				//save char
				if ( c == '_' )
					result[j++] = ' ';
				else
					result[j++] = c;
				lastc = c;
			}
		}
		c = dot_getch( line, &i );
	}
	result[j] = 0;
	return i;
}

static DOT_NODE_LIST * dot_parser_parse_attribute_tags ( DOT_PARSER *parser, char *tags, char* separator ){
	DOT_NODE_LIST *tag = 0, *oldTag = 0, *rootTag = 0;
	if( !tags || tags[0] == ' ')
		return 0;
	int   size = strlen(tags);
	char *result  = (char*) malloc (size+1);
	int   i = 0;
	while(1) {
		if ( i >= size )
			break;
		i += strcpy_till_char( result, &tags[i], separator[0] );
		if( result[0] != 0 ) {
			tag = dot_node_list_new ();
			tag->value = strdup( result );
			if ( oldTag ){
				oldTag->nextNode = tag;
			} else {
				rootTag = tag;
			}
			oldTag = tag;
		} else {
			break;
		}
	}
	free( result );
	return rootTag;
}

int dot_parser_parse_line (DOT_PARSER *parser, char *line, unsigned int length ){
	if ( !parser ){
		fprintf( stderr, "ERRR: dot_parser_parse_line(): parser object is null \n");
		return 1;
	}
	if ( !line ) {
		fprintf( stderr, "ERRR: dot_parser_parse_line(): line string is null \n");
		return 2;
	}
	if ( length == 0 ){
		fprintf( stderr, "ERRR: dot_parser_parse_line(): length is zero \n");
		return 3;
	}
	if ( line[length] != '\0' ){
		fprintf( stderr, "ERRR: dot_parser_parse_line(): line string not null terminated \n");
		return 4;
	}
	if ( line[length-1] != '\n' ){
		fprintf( stderr, "ERRR: dot_parser_parse_line(): line string doesn't have EOL \n");
		return 5;
	}
	if ( length > 1 && line[length-2] == '\r' ){
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected \\r\\n in the line, ignoring \n");
	}

	unsigned int buflen = parser->buflen>=length?parser->buflen:length;
	if ( buflen != parser->buflen ){
		if( parser->buflen == 0 ){
			parser->name   = (char*) malloc (buflen);
			parser->value  = (char*) malloc (buflen);
			parser->buflen = buflen;
		} else {
			parser->name   = (char *) realloc(parser->name  , buflen);
			parser->value  = (char *) realloc(parser->value , buflen);
			parser->buflen = buflen;
		}
	}

	if ( line[0] == ' ' || line[0] == '\t' ){
		//its a coment line
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected comment line \n");
		if ( line[length-1] == '\n') 
			line[length-1] = 0;
		if ( line[length-2] == '\r') 
			line[length-2] = 0;
		char *comment  = strdup(line);
		DOT_NODE *node = dot_node_new (parser);
		node->dotNodeValue  = comment;
		node->dotNodeType   = DOT_NODE_COMMENT;
		node->dotNodeDepth  = 1;
		node->dotNodeParent = parser->docRoot;
		dot_node_add_child (parser, parser->docRoot, node, 0);
	} else
	if ( line[0] == '.' ) {
		//its not comment line
		//its a dot line
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected actual dot line \n");
		if ( line[length-1] == '\n') 
			line[length-1] = 0;
		if ( line[length-2] == '\r') 
			line[length-2] = 0;
		DOT_NODE *previousNode = parser->currentNode;
		int depth  = read_till_end_of_char (line, '.');
		if ( !previousNode ) {
			if ( depth > 1 ){
				fprintf( stderr, "WARN: dot_parser_parse_line(): Depth greater than 1 when this is the first Node... Dropping Node \n");
				return 1;
			}
		} else {
			if ( previousNode->dotNodeDepth != (depth-1) ) {
				if( previousNode->dotNodeDepth < (depth-1) ) {
					fprintf( stderr, "WARN: dot_parser_parse_line(): Depth greater than previous node depth + 1 ... Dropping node \n");
					return 1;
				}
			}
		}

		int chars = strcpy_till_char ( parser->name , &line[depth], ' ' );
		DOT_LINE_TYPE dotLineType = DOT_LINE_ELEMENT;
		DOT_NODE *thisNode = 0;
		if( parser->name[0] == '+' && parser->name[1] == 0 ) {
			fprintf( stderr, "INFO: dot_parser_parse_line(): Detected DOT Node Append Action  \n");
			if( previousNode ){
				if( previousNode->dotNodeDepth+1 == depth ){
					thisNode = previousNode;
					dotLineType = DOT_LINE_ATTRIBUTE;
				} else {
					fprintf( stderr, "WARN: dot_parser_parse_line(): Depth of attribute line incorrect... Dropping attributes \n");
					return 1;
				}
			}
		} else 
		if( parser->name[0] == '^' && parser->name[1] == 0 ){
			//thisNode = parser->lastNode;
			//if ( thisNode )
			//	previousNode = thisNode->dotNodePrevSibling;
			fprintf( stderr, "INFO: dot_parser_parse_line(): Detected DOT Node POP action \n");
			parser->currentNode = parser->lastNode;
			if( parser->currentNode ){
				parser->previousNode = parser->currentNode->dotNodePrevSibling;
			}
			parser->lastNode = 0;
			return 0;
		} else {
			thisNode = dot_node_new (parser);
		}

		if ( dotLineType == DOT_LINE_ELEMENT ) {
			thisNode->dotNodeType    = DOT_NODE_ELEMENT;
			parser->previousNode     = parser->currentNode;
			parser->currentNode      = thisNode;
			thisNode->dotNodeDepth   = depth;
			thisNode->dotNodeName    = strdup(parser->name);
			thisNode->dotNodeValue   = 0;
		}

		int curIndex = depth + chars;
		int indexOffset = 0;
		while( curIndex < length && line[curIndex] != 0 ){
			parser->name[0]  = 0;
			parser->value[0] = 0;
			indexOffset = strcpy_till_char( parser->name, &line[curIndex], ':');
			curIndex += indexOffset;
			if ( curIndex < length && line[curIndex] != 0 ){
					if ( line[curIndex] == ' ' ) {
						fprintf( stderr, "DBUG: dot_parser_parse_line(): Skipping attribute as the value is empty \n");
						indexOffset = read_till_end_of_char ( &line[curIndex], ' ');
						curIndex += indexOffset;
					} else {
						indexOffset = strcpy_till_char ( parser->value, &line[curIndex], ' ' );
						curIndex += indexOffset;
						DOT_NODE *childNode = 0;
						DOT_NODE_LIST *tags;
						if ( parser->name[0] != 0 ){
							if ( parser->name[1] == 0 ){
								switch ( parser->name[0] ){
									case '@':
										{
											unsigned int hash = dot_parser_get_id_hash (parser, parser->value);
											DOT_NODE *idNode = dot_parser_get_node_by_id (parser, parser->docRoot, parser->value, hash );
											fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected id in node, Looking up %s %s\n",
											        parser->value,idNode?"Successful":"Unsuccessful");
											if( idNode && idNode->dotNodeType == thisNode->dotNodeType && 
											   strcmp(idNode->dotNodeName, thisNode->dotNodeName) == 0 ){
												parser->lastNode = previousNode;
												dot_node_merge (parser, idNode, thisNode);
												parser->previousNode = previousNode = idNode->dotNodePrevSibling;
												parser->currentNode  = thisNode     = idNode;
												dotLineType = DOT_LINE_ELEMENTLINK;
												continue;
											} else {
												fprintf(stderr, "DBUG: dot_parser_parse_line(): Not Merging \n");
												thisNode->dotNodeId     = strdup(parser->value);
												thisNode->dotNodeIdHash = hash;
											}
										}
										break;
									case '#':
										fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected tags in node \n");
										//indexOffset = strcpy_till_char ( parser->value, &line[curIndex], ' ' );
										tags = dot_parser_parse_attribute_tags( parser, &(parser->value[0]), "," );
										if ( thisNode->tags ){
											dot_node_merge_tags (parser, thisNode, tags);
										} else {
											thisNode->tags = tags;
										}
										break;
									case '+':
										fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected append attribute operation in node \n");
										break;
									case '-':
										fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected delete attribute operation in node \n");
										break;
									case '.':
										{
											childNode = dot_node_new (parser);
											childNode->dotNodeType  = DOT_NODE_TEXTATTRIBUTE;
											childNode->dotNodeDepth = thisNode->dotNodeDepth + 1;
											childNode->dotNodeName  = strdup(parser->name);
											if ( parser->value[0] != 0 ){
												childNode->dotNodeValue = strdup(parser->value);
											}
										}
										break;
									default:
										fprintf( stderr, "WARN: dot_parser_parse_line(): Detected Unknown Character %d value in node \n", parser->name[0]);
										break;
								}
							} else {
								childNode = dot_node_new (parser);
								childNode->dotNodeType  = DOT_NODE_ATTRIBUTE;
								childNode->dotNodeDepth = thisNode->dotNodeDepth + 1;
								childNode->dotNodeName  = strdup(parser->name);
								if ( parser->value[0] != 0 ){
									childNode->dotNodeValue = strdup(parser->value);
								}
							}
						}
						if( childNode ){
							dot_node_add_child ( parser, thisNode, childNode, 0 );
						}
						if ( curIndex >= length || line[curIndex] == 0 )
							break;
						indexOffset = read_till_end_of_char ( &line[curIndex], ' ');
						curIndex += indexOffset;
					}
#if 0
				}
#endif
			} else {
				break;
			}
		}
		if( dotLineType == DOT_LINE_ELEMENT ) { 
			if ( previousNode ) {
				if ( previousNode->dotNodeDepth == thisNode->dotNodeDepth - 1 ){
					dot_node_add_child (parser, previousNode, thisNode, 0);
					fprintf( stderr, "WARN: dot_parser_parse_line(): Reached here 7\n");
				} else
				if ( previousNode->dotNodeDepth == thisNode->dotNodeDepth ){
					dot_node_add_child (parser, previousNode->dotNodeParent, thisNode, 0);
					fprintf( stderr, "WARN: dot_parser_parse_line(): Reached here 8\n");
				} else {
					if ( thisNode->dotNodeDepth == 1 ){
						dot_node_add_child (parser, parser->docRoot, thisNode, 0);
						fprintf( stderr, "WARN: dot_parser_parse_line(): Reached here 9\n");
					} else {
						int depthDiff = previousNode->dotNodeDepth - thisNode->dotNodeDepth;
						if ( depthDiff >= 1 ){
							DOT_NODE *tmpNode = previousNode;
							while (depthDiff){
								depthDiff--;
								tmpNode = tmpNode->dotNodeParent;
							}
							if ( tmpNode ) {
								fprintf( stderr, "WARN: dot_parser_parse_line(): Reached here 10\n");
								dot_node_add_child (parser, tmpNode, thisNode, 0);
							} else {
								fprintf( stderr, "WARN: dot_parser_parse_line(): Didn't find suitable parent node, adding to docRoot\n");
								dot_node_add_child (parser, parser->docRoot, thisNode, 0);
							}
						} else if ( depthDiff < 0 ){
							fprintf( stderr, "WARN: dot_parser_parse_line(): Current node is higher than previous node by a depth > 1\n");
							//need to free the node here
						} else {
							dot_node_add_child (parser, parser->docRoot, thisNode, 0);
						}
					}
				}
			} else {
				if( thisNode->dotNodeDepth == 1 ) {
					dot_node_add_child (parser, parser->docRoot, thisNode, 0);
				} else {
					fprintf( stderr, "WARN: dot_parser_parse_line(): The first dot node doesn't have depth of 1 \n");
					//Need to free up memory
				}
			}
		}
	} else
	if ( line[0] == '\n' || ( line[0] == '\r' && line[1] == '\n' ) ){
		//its not comment line
		//its not dot line
		//its a empty line
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected empty line \n");
		DOT_NODE *node = dot_node_new (parser);
		node->dotNodeType   = DOT_NODE_EMPTY;
		node->dotNodeDepth  = 1;
		node->dotNodeParent = parser->docRoot;
		dot_node_add_child (parser, parser->docRoot, node, 0);
	}else 
	if ( line[0] == '@' && line[1] == ' ' ){

		//its not comment line
		//its not dot line
		//its not empty line
		//its a marker-selector line
		if ( line[length-1] == '\n') 
			line[length-1] = 0;
		if ( line[length-2] == '\r') 
			line[length-2] = 0;
		parser->name[0] = 0;
		int indexOffset  = read_till_end_of_char ( &line[1], ' ');
		indexOffset++;
		if ( line[indexOffset] == 0 ){
			parser->fromNode = 0;
			parser->toNode   = 0;
			fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected marker selector dot clear line \n");
		} else {
			
			indexOffset += strcpy_till_char ( parser->name, &line[indexOffset], ' ' );
			fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected marker selector @%s dot  line \n", parser->name?parser->name:"");
			unsigned int hash = dot_parser_get_id_hash (parser, parser->name);
			DOT_NODE *realNode = dot_parser_get_node_by_id (parser, parser->docRoot, parser->name, hash );
			if( realNode ){
				if( parser->toNode ){
					parser->fromNode = parser->toNode;
				}
				parser->toNode = realNode;
			}
		}
		DOT_NODE *node = dot_node_new (parser);
		node->dotNodeType   = DOT_NODE_MARKER;
		node->dotNodeDepth  = 1;
		node->dotNodeParent = parser->docRoot;
		if( parser->name && parser->name[0] ){
			node->dotNodeName = strdup( parser->name );
		}
		dot_node_add_child (parser, parser->docRoot, node, 0);
	} else 
	if ( line[0] == '#' && line[1] == ' ' ){
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected operation line \n");
		//its not comment line
		//its not dot line
		//its not empty line
		//its not marker-selector line
		//its an operation line

		if ( line[length-1] == '\n') 
			line[length-1] = 0;
		if ( line[length-2] == '\r') 
			line[length-2] = 0;
		int indexOffset  = read_till_end_of_char ( &line[1], ' ');
		indexOffset++;
		if ( line[indexOffset] == 0 ){
			fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected empty operation line \n");
		} else {
			indexOffset += strcpy_till_char ( parser->name,  &line[indexOffset], ':' );
			indexOffset += strcpy_till_char ( parser->value, &line[indexOffset], ' ' );
			fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected operation from=%s to=%s arguments=%s dot line \n", 
			        parser->fromNode?parser->fromNode->dotNodeName?parser->fromNode->dotNodeName:"":"",
			        parser->toNode?parser->toNode->dotNodeName?parser->toNode->dotNodeName:"":"",
			        parser->value
			        );
		}

		DOT_NODE *node = dot_node_new (parser);
		node->dotNodeType   = DOT_NODE_OPERATION;
		node->dotNodeDepth  = 1;
		node->dotNodeParent = parser->docRoot;
		if( parser->name && parser->name[0] ){
			node->dotNodeName = strdup( parser->name );
		}
		if( parser->value && parser->value[0] ){
			node->dotNodeValue = strdup( parser->value );
		}
		node->list = dot_parser_parse_attribute_tags( parser, &(parser->value[0]), "," );
		if( parser->name && parser->name[0] ){
			switch( parser->name[0] ){
				case '+':
						break;
				case '-':
						fprintf( stderr, "DBUG: Deleting Attribute with Name: %s \n", node->list?node->list->value:"");
						dot_node_attribute_minus_attribute_value (parser, parser->toNode, node->list );
						break;
				default:
						fprintf( stderr, "DBUG: Undefined Operation\n");
						break;
			}
		}
		dot_node_add_child (parser, parser->docRoot, node, 0);
		
	} else 
	{
		//its not comment line
		//its not dot line
		//its not empty line
		//its not marker-selector line
		//its not operation line
		//its a configuration line
		fprintf( stderr, "DBUG: dot_parser_parse_line(): Detected configuration line \n");
		if ( line[length-1] == '\n') 
			line[length-1] = 0;
		if ( line[length-2] == '\r') 
			line[length-2] = 0;
		int chars = strcpy_till_char ( parser->name , line, ':' );
		strcpy( parser->value, &line[chars] ) ;
		DOT_NODE *node = dot_node_new (parser);
		node->dotNodeName   = strdup(parser->name);
		node->dotNodeValue  = strdup(parser->value);
		node->dotNodeType   = DOT_NODE_CONFIGURATION;
		node->dotNodeDepth  = 1;
		node->dotNodeParent = parser->docRoot;
		dot_node_add_child (parser, parser->docRoot, node, 0);
	}
}

int         dot_parser_parse_file (DOT_PARSER *parser, char *filename) {
	if ( parser && filename ){
		
	}
	return 1;
}

int         dot_parser_dump_lists ( DOT_PARSER *parser, DOT_NODE *node ) {
	if(!node)
		return 1;
	DOT_NODE_LIST *tags = node->tags;
	while ( tags ) {
		if( tags->value ){
			fprintf( stderr, "INFO: dot_parser_dump_tags(): |----> TAG: %s \n", tags->value?tags->value:"" );
		}
		tags = tags->nextNode;
	}
	DOT_NODE_LIST *list = node->list;
	if(!list)
		return 0;
	fprintf( stderr, "INFO: dot_parser_dump_tags(): ###########\n");
	
	while ( list ) {
		if( list->value ){
			fprintf( stderr, "INFO: dot_parser_dump_tags(): |---->LIST: %s \n", list->value?list->value:"" );
		}
		list = list->nextNode;
	}
	return 0;
}

char*       dot_parser_dump_node_type( DOT_PARSER *parser, unsigned int type ){
	static char *dotNodeTypeInvalid = "INVALID NODE TYPE";
	static char *dotNodeTypeStr[DOT_NODE_MAXTYPE + 1] = {
		 "DOT_NODE_ROOT",   //DOT node root node
		 "DOT_NODE_CONFIGURATION",         //DOT node configuration
		 "DOT_NODE_COMMENT",               //DOT node lines starting with space
		 "DOT_NODE_EMPTY",                 //DOT node empty line
		 "DOT_NODE_SPACELINE",             //DOT node space, tab, carage return line
		 "DOT_NODE_MARKER",
		 "DOT_NODE_OPERATION",
		 "DOT_NODE_ELEMENT",               //DOT node element
		 "DOT_NODE_ATTRIBUTE",             //DOT node normal attribute
		 "DOT_NODE_TEXTATTRIBUTE",         //DOT node text/textdata attribute
		 "DOT_NODE_LISTATTRIBUTE",
		 "DOT_NODE_RESULTATTRIBUTE",
		 "DOT_NODE_RESULTLISTATTRIBUTE",
		 "DOT_NODE_BLOBATTRIBUTE",         //DOT node binary large object attribute for embedding binary objects
		 "DOT_NODE_CLOBATTRIBUTE"          //DOT node character large object attribute for embedding scripts etc
		 
	};
	if ( type >= DOT_NODE_MAXTYPE ){
		fprintf (stderr, "ERRR: dot_parser_dump_node_type(): nodetype is invalid \n");
		return dotNodeTypeInvalid;
	} else {
		return dotNodeTypeStr[type];
	}
}


int         dot_parser_dump_node ( DOT_PARSER *parser, DOT_NODE *node, unsigned options ){
	/*
	  	unsigned int      dotNodeDepth;
		unsigned int      dotNodeEvaluated;
		char*             dotNodeId;
		char*             dotNodeName;
		char*             dotNodeValue;
	 */
	if( node) {
		if( node->dotNodeType <= DOT_NODE_ELEMENT )
				fprintf ( stderr, "INFO: =============================================================================\n");
		fprintf ( stderr, "INFO: -----------------------------------------------------------------------------\n");
		fprintf ( stderr, "INFO: dot_parser_dump_node(): depth:%u, type:%s, eval:%u, pname:%s, name:%s, @:%s, value:%s \n",
	     											node->dotNodeDepth,
		     										dot_parser_dump_node_type (parser, node->dotNodeType),
	     											node->dotNodeEvaluated,
		     										node->dotNodeParent?(node->dotNodeParent->dotNodeName?node->dotNodeParent->dotNodeName:""):"",
	     											node->dotNodeName?node->dotNodeName:"",
	     											node->dotNodeId?node->dotNodeId:"",
	     											node->dotNodeValue?node->dotNodeValue:"" );
	}
}

int         dot_parser_dump ( DOT_PARSER *parser, DOT_NODE *node, unsigned int options ){
	if( node ){
		DOT_NODE * tmpNode = node;
		do {
			dot_parser_dump_node (parser, tmpNode, options);
			dot_parser_dump_lists (parser, tmpNode);
			if ( tmpNode->dotNodeFirstChild ){
				dot_parser_dump (parser, tmpNode->dotNodeFirstChild, options);
			}
			tmpNode = tmpNode->dotNodeNextSibling;
		}while ( tmpNode );
	}
}


