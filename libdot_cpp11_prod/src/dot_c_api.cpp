#include "libdot/dot.hpp"
#include "libdot/dot_c_api.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

using libdot::AttributeValue;
using libdot::Document;
using libdot::Node;
using libdot::NodeState;
using libdot::NodeType;

struct DOT_PARSER {
    Document* doc;
    std::map<const Node*, DOT_NODE*> cache;
    std::vector<DOT_NODE*> allocated;
    std::string last_dump;
};

static unsigned int fnv1a(const char* s) {
    unsigned int h = 2166136261u;
    while (s && *s) {
        h ^= static_cast<unsigned char>(*s++);
        h *= 16777619u;
    }
    return h;
}

static char* dup_cstr(const std::string& s) {
    char* p = new char[s.size() + 1];
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

static void free_list(DOT_NODE_LIST* head) {
    while (head) {
        DOT_NODE_LIST* next = head->nextNode;
        delete [] head->value;
        delete head;
        head = next;
    }
}

static void free_node(DOT_NODE* n) {
    if (!n) return;
    delete [] n->dotNodeId;
    delete [] n->dotNodeName;
    delete [] n->dotNodeValue;
    free_list(n->tags);
    free_list(n->list);
    delete n;
}

static DOT_NODE_TYPE map_type(NodeType t) { return static_cast<DOT_NODE_TYPE>(static_cast<int>(t)); }
static DOT_NODE_STATE map_state(NodeState s) { return static_cast<DOT_NODE_STATE>(static_cast<int>(s)); }

static DOT_NODE_LIST* build_list(const std::vector<std::string>& values) {
    DOT_NODE_LIST* head = NULL;
    DOT_NODE_LIST* tail = NULL;
    for (std::size_t i = 0; i < values.size(); ++i) {
        DOT_NODE_LIST* item = new DOT_NODE_LIST();
        item->value = dup_cstr(values[i]);
        item->nextNode = NULL;
        if (!head) head = item; else tail->nextNode = item;
        tail = item;
    }
    return head;
}

static DOT_NODE* bridge(DOT_PARSER* p, const Node* n) {
    if (!p || !n) return NULL;
    std::map<const Node*, DOT_NODE*>::iterator it = p->cache.find(n);
    if (it != p->cache.end()) return it->second;

    DOT_NODE* out = new DOT_NODE();
    std::memset(out, 0, sizeof(*out));
    out->dotNodeType = map_type(n->type());
    out->dotNodeState = map_state(n->state());
    out->dotNodeDepth = static_cast<unsigned int>(n->depth());
    out->dotNodeId = dup_cstr(n->id());
    out->dotNodeName = dup_cstr(n->name());
    out->dotNodeValue = dup_cstr(n->value());
    out->dotNodeValueSize = static_cast<unsigned int>(n->value().size());
    out->dotNodeIdHash = fnv1a(out->dotNodeId);
    out->tags = build_list(n->tags());

    std::vector<std::string> attrs;
    const std::vector<AttributeValue>& av = n->attributes();
    for (std::size_t i = 0; i < av.size(); ++i) {
        if (av[i].deleted) continue;
        attrs.push_back(av[i].name + ":" + av[i].value);
    }
    out->list = build_list(attrs);

    p->allocated.push_back(out);
    p->cache[n] = out;
    out->dotNodeParent = bridge(p, n->parent());
    out->dotNodeFirstChild = bridge(p, n->first_child());
    out->dotNodeNextSibling = bridge(p, n->next_sibling());
    out->dotNodePrevSibling = bridge(p, n->prev_sibling());
    return out;
}

extern "C" {

DOT_PARSER* dot_parser_new(void) {
    DOT_PARSER* p = new DOT_PARSER();
    p->doc = new Document();
    return p;
}

void dot_parser_delete(DOT_PARSER* parser) {
    if (!parser) return;
    for (std::size_t i = 0; i < parser->allocated.size(); ++i) free_node(parser->allocated[i]);
    delete parser->doc;
    delete parser;
}

DOT_NODE* dot_parser_get_root_node(DOT_PARSER* parser) { return parser ? bridge(parser, parser->doc->root()) : NULL; }
DOT_NODE* dot_parser_get_current_node(DOT_PARSER* parser) { return parser ? bridge(parser, parser->doc->current_node()) : NULL; }

int dot_parser_parse_line(DOT_PARSER* parser, char* line, unsigned int len) {
    if (!parser || !line) return -1;
    try {
        parser->doc->parse_line(std::string(line, line + len));
        return 0;
    } catch (...) {
        return -1;
    }
}

int dot_parser_parse_file(DOT_PARSER* parser, char* path) {
    if (!parser || !path) return -1;
    try {
        parser->doc->parse_file(std::string(path));
        return 0;
    } catch (...) {
        return -1;
    }
}

int dot_parser_dump(DOT_PARSER* parser, DOT_NODE*, unsigned int) {
    if (!parser) return -1;
    parser->last_dump = parser->doc->dump(false);
    return 0;
}

int dot_parser_pretty_print(DOT_PARSER* parser, DOT_NODE*, unsigned int) {
    if (!parser) return -1;
    parser->last_dump = parser->doc->dump(true);
    return 0;
}

unsigned int dot_parser_get_id_hash(DOT_PARSER*, char* id) { return fnv1a(id); }

DOT_NODE* dot_parser_get_node(DOT_PARSER* parser, DOT_NODE*, char* name) {
    if (!parser || !name) return NULL;
    return bridge(parser, parser->doc->get_node(parser->doc->current_node(), std::string(name)));
}

DOT_NODE* dot_parser_get_node_by_id(DOT_PARSER* parser, DOT_NODE*, char* id, unsigned int) {
    if (!parser || !id) return NULL;
    return bridge(parser, parser->doc->get_node_by_id(std::string(id)));
}

const char* dot_parser_last_dump(DOT_PARSER* parser) {
    return parser ? parser->last_dump.c_str() : NULL;
}

} // extern "C"
