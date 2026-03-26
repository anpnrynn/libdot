#include "libdot/dot.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace libdot {

namespace {

static bool all_digits(const std::string& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
        return !std::isdigit(static_cast<unsigned char>(c));
    }) == s.end();
}

static std::vector<std::string> split_escaped_public(const std::string& text, char separator) {
    std::vector<std::string> out;
    std::string cur;
    bool esc = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (esc) { cur.push_back('`'); cur.push_back(c); esc = false; continue; }
        if (c == '`') { esc = true; continue; }
        if (c == separator) { out.push_back(cur); cur.clear(); } else { cur.push_back(c); }
    }
    if (esc) cur.push_back('`');
    out.push_back(cur);
    return out;
}

static void collect_elements(Node* node, std::vector<Node*>& out) {
    if (!node) return;
    if (node->type() == NodeType::Element) out.push_back(node);
    std::vector<std::unique_ptr<Node> >& kids = node->children();
    for (std::size_t i = 0; i < kids.size(); ++i) collect_elements(kids[i].get(), out);
}

static bool has_tag(const Node* node, const std::string& tag) {
    std::vector<std::string> tags = node->tags();
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

} // namespace

LobValue::LobValue() : declared_size(0) {}
bool LobValue::empty() const { return data.empty(); }

AttributeValue::AttributeValue() : type(NodeType::Attribute), deleted(false) {}
AttributeValue::AttributeValue(const std::string& n, const std::string& v, NodeType t)
    : name(n), value(v), type(t), deleted(false) {}
bool AttributeValue::has_lob() const { return type == NodeType::BlobAttribute || type == NodeType::ClobAttribute; }

Node::Node(NodeType type)
    : type_(type), state_(NodeState::Normal), depth_(0), parent_(NULL), prev_sibling_(NULL), next_sibling_(NULL) {}

NodeType Node::type() const { return type_; }
NodeState Node::state() const { return state_; }
void Node::set_state(NodeState state) { state_ = state; }
std::size_t Node::depth() const { return depth_; }
void Node::set_depth(std::size_t depth) { depth_ = depth; }
const std::string& Node::name() const { return name_; }
void Node::set_name(const std::string& name) { name_ = name; }
const std::string& Node::value() const { return value_; }
void Node::set_value(const std::string& value) { value_ = value; }
const std::string& Node::id() const { return id_; }
void Node::set_id(const std::string& id) { id_ = id; }
Node* Node::parent() const { return parent_; }
Node* Node::first_child() const { return children_.empty() ? NULL : children_.front().get(); }
Node* Node::next_sibling() const { return next_sibling_; }
Node* Node::prev_sibling() const { return prev_sibling_; }
const std::vector<std::unique_ptr<Node> >& Node::children() const { return children_; }
std::vector<std::unique_ptr<Node> >& Node::children() { return children_; }
const std::vector<AttributeValue>& Node::attributes() const { return attributes_; }
std::vector<AttributeValue>& Node::attributes() { return attributes_; }

void Node::add_child(std::unique_ptr<Node> child) {
    child->parent_ = this;
    if (!children_.empty()) {
        child->prev_sibling_ = children_.back().get();
        children_.back()->next_sibling_ = child.get();
    }
    children_.push_back(std::move(child));
}

void Node::add_attribute(const AttributeValue& attr) { attributes_.push_back(attr); }

AttributeValue* Node::find_attribute(const std::string& name, std::size_t occurrence, bool include_deleted) {
    std::size_t seen = 0;
    for (std::size_t i = 0; i < attributes_.size(); ++i) {
        AttributeValue& attr = attributes_[i];
        if (attr.name == name && (include_deleted || !attr.deleted)) {
            if (seen == occurrence) return &attr;
            ++seen;
        }
    }
    return NULL;
}

const AttributeValue* Node::find_attribute(const std::string& name, std::size_t occurrence, bool include_deleted) const {
    std::size_t seen = 0;
    for (std::size_t i = 0; i < attributes_.size(); ++i) {
        const AttributeValue& attr = attributes_[i];
        if (attr.name == name && (include_deleted || !attr.deleted)) {
            if (seen == occurrence) return &attr;
            ++seen;
        }
    }
    return NULL;
}

std::vector<const AttributeValue*> Node::find_attributes(const std::string& name, bool include_deleted) const {
    std::vector<const AttributeValue*> out;
    for (std::size_t i = 0; i < attributes_.size(); ++i) {
        if (attributes_[i].name == name && (include_deleted || !attributes_[i].deleted)) out.push_back(&attributes_[i]);
    }
    return out;
}

std::vector<std::string> Node::tags() const {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < attributes_.size(); ++i) {
        const AttributeValue& attr = attributes_[i];
        if (attr.deleted || attr.name != "#") continue;
        std::vector<std::string> parts = split_escaped_public(attr.value, ',');
        for (std::size_t j = 0; j < parts.size(); ++j) out.push_back(Document::unescape(parts[j]));
    }
    return out;
}

Error::Error(const std::string& message) : std::runtime_error(message) {}

Document::PendingLob::PendingLob() : node(NULL), attr_type(NodeType::BlobAttribute), size(0) {}
bool Document::PendingLob::active() const { return node != NULL; }
void Document::PendingLob::reset() { node = NULL; attr_name.clear(); media_type.clear(); size = 0; attr_type = NodeType::BlobAttribute; }

Document::Document() { clear(); }

Node* Document::root() { return root_.get(); }
const Node* Document::root() const { return root_.get(); }
Node* Document::current_node() { return current_node_; }
const Node* Document::current_node() const { return current_node_; }
Node* Document::previous_node() { return previous_node_; }
const Node* Document::previous_node() const { return previous_node_; }
std::size_t Document::line_number() const { return line_number_; }

void Document::clear() {
    root_.reset(new Node(NodeType::Root));
    root_->set_name("root");
    root_->set_depth(0);
    line_number_ = 0;
    previous_node_ = root_.get();
    current_node_ = root_.get();
    last_node_before_selector_ = root_.get();
    from_node_ = NULL;
    to_node_ = NULL;
    depth_stack_.assign(1, root_.get());
    marker_index_.clear();
    pending_lob_.reset();
}

std::string Document::trim(const std::string& s) {
    std::size_t begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) ++begin;
    std::size_t end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(begin, end - begin);
}

std::size_t Document::count_leading_dots(const std::string& line) {
    std::size_t depth = 0;
    while (depth < line.size() && line[depth] == '.') ++depth;
    return depth;
}

std::vector<std::string> Document::split_escaped(const std::string& text, char separator) {
    std::vector<std::string> out;
    std::string cur;
    bool esc = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (esc) {
            cur.push_back('`');
            cur.push_back(c);
            esc = false;
            continue;
        }
        if (c == '`') {
            esc = true;
            continue;
        }
        if (c == separator) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (esc) cur.push_back('`');
    out.push_back(cur);
    return out;
}

std::vector<std::string> Document::split_tokens(const std::string& text) {
    std::vector<std::string> out;
    std::string cur;
    bool esc = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (esc) {
            cur.push_back('`');
            cur.push_back(c);
            esc = false;
            continue;
        }
        if (c == '`') {
            esc = true;
            continue;
        }
        if (c == ' ') {
            if (!cur.empty()) {
                out.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (esc) cur.push_back('`');
    if (!cur.empty()) out.push_back(cur);
    return out;
}

std::string Document::unescape(const std::string& value) {
    std::string out;
    bool esc = false;
    for (std::size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (esc) {
            switch (c) {
                case '`': out.push_back('`'); break;
                case '_': out.push_back('_'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case ':': out.push_back(':'); break;
                case ',': out.push_back(','); break;
                case ' ': out.push_back(' '); break;
                default: out.push_back(c); break;
            }
            esc = false;
        } else if (c == '`') {
            esc = true;
        } else if (c == '_') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
    }
    if (esc) out.push_back('`');
    return out;
}

std::string Document::escape(const std::string& value) {
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        switch (c) {
            case '`': out += "``"; break;
            case '_': out += "`_"; break;
            case '\n': out += "`n"; break;
            case '\r': out += "`r"; break;
            case '\t': out += "`t"; break;
            case ' ': out += "_"; break;
            case ':': out += "`:"; break;
            case ',': out += "`,"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

Node* Document::ensure_parent_for_depth(std::size_t depth) {
    if (depth == 0) return root_.get();
    if (depth_stack_.size() <= depth) depth_stack_.resize(depth + 1, NULL);
    Node* parent = depth_stack_[depth - 1];
    if (!parent) throw Error("invalid depth at line " + std::to_string(line_number_));
    return parent;
}

void Document::reindex_markers(Node* node) {
    if (!node) return;
    const std::vector<AttributeValue>& attrs = node->attributes();
    for (std::size_t i = 0; i < attrs.size(); ++i) {
        if (attrs[i].deleted) continue;
        if (attrs[i].name == "@") {
            node->set_id(attrs[i].value);
            marker_index_.push_back(std::make_pair(attrs[i].value, node));
        }
    }
}

Node* Document::get_node(Node* from, const std::string& name) {
    if (!from) return NULL;
    std::vector<std::unique_ptr<Node> >& kids = from->children();
    for (std::size_t i = 0; i < kids.size(); ++i) if (kids[i]->name() == name) return kids[i].get();
    return NULL;
}

const Node* Document::get_node(const Node* from, const std::string& name) const {
    if (!from) return NULL;
    const std::vector<std::unique_ptr<Node> >& kids = from->children();
    for (std::size_t i = 0; i < kids.size(); ++i) if (kids[i]->name() == name) return kids[i].get();
    return NULL;
}

Node* Document::get_node_by_id(const std::string& id) {
    for (std::size_t i = marker_index_.size(); i > 0; --i) {
        if (marker_index_[i - 1].first == id) return marker_index_[i - 1].second;
    }
    return NULL;
}

const Node* Document::get_node_by_id(const std::string& id) const {
    for (std::size_t i = marker_index_.size(); i > 0; --i) {
        if (marker_index_[i - 1].first == id) return marker_index_[i - 1].second;
    }
    return NULL;
}

void Document::parse_configuration_line(const std::string& line) {
    std::size_t sep = line.find(':');
    if (sep == std::string::npos) return;
    std::unique_ptr<Node> node(new Node(NodeType::Configuration));
    node->set_name(unescape(trim(line.substr(0, sep))));
    node->set_value(unescape(trim(line.substr(sep + 1))));
    node->set_depth(0);
    root_->add_child(std::move(node));
    current_node_ = root_->children().back().get();
    previous_node_ = current_node_;
}

void Document::parse_selector_line(const std::string& line) {
    std::string marker = trim(line.substr(1));
    if (marker.empty()) {
        from_node_ = NULL;
        to_node_ = NULL;
        current_node_ = previous_node_;
        return;
    }
    Node* found = get_node_by_id(unescape(marker));
    if (!found) throw Error("unknown selector at line " + std::to_string(line_number_));
    from_node_ = to_node_;
    to_node_ = found;
    last_node_before_selector_ = previous_node_;
    current_node_ = found;
    previous_node_ = found;
}

void Document::op_append(const std::vector<std::string>& args) {
    if (!to_node_ || !from_node_ || args.size() < 4) throw Error("append op requires to/from nodes and 4 args");
    const AttributeValue* src = from_node_->find_attribute(unescape(args[2]), static_cast<std::size_t>(std::atoi(args[3].c_str())));
    AttributeValue* dst = to_node_->find_attribute(unescape(args[0]), static_cast<std::size_t>(std::atoi(args[1].c_str())));
    if (!src || !dst) throw Error("append op attribute not found");
    dst->value += src->value;
}

void Document::op_assign(const std::vector<std::string>& args) {
    if (!to_node_ || !from_node_ || args.size() < 4) throw Error("assign op requires to/from nodes and 4 args");
    const AttributeValue* src = from_node_->find_attribute(unescape(args[2]), static_cast<std::size_t>(std::atoi(args[3].c_str())));
    AttributeValue* dst = to_node_->find_attribute(unescape(args[0]), static_cast<std::size_t>(std::atoi(args[1].c_str())));
    if (!src || !dst) throw Error("assign op attribute not found");
    *dst = *src;
    dst->name = unescape(args[0]);
}

void Document::op_delete(const std::vector<std::string>& args) {
    if (!to_node_ || args.size() < 2) throw Error("delete op requires to node and 2 args");
    AttributeValue* dst = to_node_->find_attribute(unescape(args[0]), static_cast<std::size_t>(std::atoi(args[1].c_str())));
    if (!dst) throw Error("delete op attribute not found");
    dst->deleted = true;
}

void Document::op_link(const std::vector<std::string>& args) {
    if (!to_node_) throw Error("link op requires to node");
    if (args.size() == 1) {
        if (!from_node_) throw Error("link op requires from node");
        to_node_->add_attribute(AttributeValue(unescape(args[0]), "node:" + from_node_->id(), NodeType::ResultAttribute));
        return;
    }
    if (!from_node_ || args.size() < 4) throw Error("link op requires to/from nodes and 4 args");
    const AttributeValue* src = from_node_->find_attribute(unescape(args[2]), static_cast<std::size_t>(std::atoi(args[3].c_str())));
    if (!src) throw Error("link op source attribute not found");
    to_node_->add_attribute(AttributeValue(unescape(args[0]), "attr:" + from_node_->id() + ":" + src->name + ":" + src->value, NodeType::ResultAttribute));
}

void Document::op_tag_search(const std::vector<std::string>& args, bool invert) {
    if (!to_node_ || args.size() < 2) throw Error("tag search op requires at least 2 args");
    std::string target_name = unescape(args[0]);
    std::size_t arg_index = 1;
    bool use_existing = false;
    std::size_t occurrence = 0;
    if (args.size() >= 3 && all_digits(args[1])) {
        use_existing = true;
        occurrence = static_cast<std::size_t>(std::atoi(args[1].c_str()));
        arg_index = 2;
    }
    std::vector<std::string> required;
    for (std::size_t i = arg_index; i < args.size(); ++i) required.push_back(unescape(args[i]));

    std::vector<Node*> elems;
    collect_elements(root_.get(), elems);
    std::ostringstream joined;
    bool first = true;
    for (std::size_t i = 0; i < elems.size(); ++i) {
        bool ok = true;
        for (std::size_t j = 0; j < required.size(); ++j) {
            if (!has_tag(elems[i], required[j])) { ok = false; break; }
        }
        if (invert) ok = !ok;
        if (!ok) continue;
        if (!first) joined << ',';
        first = false;
        joined << escape(elems[i]->id().empty() ? elems[i]->name() : elems[i]->id());
    }
    if (use_existing) {
        AttributeValue* dst = to_node_->find_attribute(target_name, occurrence);
        if (!dst) throw Error("tag search target attribute not found");
        dst->value = joined.str();
        dst->type = NodeType::ResultListAttribute;
    } else {
        to_node_->add_attribute(AttributeValue(target_name, joined.str(), NodeType::ResultListAttribute));
    }
}

void Document::op_split(const std::vector<std::string>& args) {
    if (!to_node_ || args.size() < 2) throw Error("split op requires 2 args");
    AttributeValue* dst = to_node_->find_attribute(unescape(args[0]), 0);
    if (!dst) throw Error("split op target not found");
    const std::string delim_text = unescape(args[1]);
    const char delim = delim_text.empty() ? ',' : delim_text[0];
    std::vector<std::string> parts = split_escaped(dst->value, delim);
    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i) out << ',';
        out << parts[i];
    }
    dst->value = out.str();
    dst->type = NodeType::ResultListAttribute;
}

void Document::op_blob(const std::vector<std::string>& args) {
    if (!to_node_ || args.size() < 4) throw Error("blob op requires attr, kind, mime, size");
    pending_lob_.node = to_node_;
    pending_lob_.attr_name = unescape(args[0]);
    const std::string kind = unescape(args[1]);
    pending_lob_.attr_type = (kind == "clob") ? NodeType::ClobAttribute : NodeType::BlobAttribute;
    pending_lob_.media_type = unescape(args[2]);
    pending_lob_.size = static_cast<std::size_t>(std::strtoul(args[3].c_str(), NULL, 10));
}

void Document::parse_operation_line(const std::string& line) {
    std::string body = trim(line.substr(1));
    std::size_t sep = body.find(':');
    if (sep == std::string::npos || sep == 0) throw Error("malformed operation at line " + std::to_string(line_number_));
    char op = body[0];
    std::vector<std::string> args = split_escaped(body.substr(sep + 1), ',');
    switch (op) {
        case '+': op_append(args); break;
        case '=': op_assign(args); break;
        case '-': op_delete(args); break;
        case '@': op_link(args); break;
        case '?': op_tag_search(args, false); break;
        case '~': op_tag_search(args, true); break;
        case ',': op_split(args); break;
        case '<': op_blob(args); break;
        default: throw Error("unsupported operation at line " + std::to_string(line_number_));
    }
}

void Document::parse_dot_line(const std::string& line) {
    std::size_t depth = count_leading_dots(line);
    std::string body = trim(line.substr(depth));
    if (body.empty()) return;

    if (body[0] == '^') {
        current_node_ = last_node_before_selector_;
        previous_node_ = last_node_before_selector_;
        return;
    }

    bool append_only = false;
    if (body[0] == '+') {
        append_only = true;
        body = trim(body.substr(1));
    }

    std::vector<std::string> tokens = split_tokens(body);
    if (tokens.empty()) return;

    Node* target = NULL;
    if (append_only) {
        if (!previous_node_) throw Error("attribute append without previous node");
        if (depth != previous_node_->depth() + 1) throw Error("attribute append depth mismatch at line " + std::to_string(line_number_));
        target = previous_node_;
    } else {
        std::unique_ptr<Node> node(new Node(NodeType::Element));
        node->set_depth(depth);
        node->set_name(unescape(tokens[0]));
        Node* parent = ensure_parent_for_depth(depth);
        parent->add_child(std::move(node));
        target = parent->children().back().get();
        current_node_ = target;
        previous_node_ = target;
        if (depth_stack_.size() <= depth) depth_stack_.resize(depth + 1, NULL);
        depth_stack_[depth] = target;
        for (std::size_t i = depth + 1; i < depth_stack_.size(); ++i) depth_stack_[i] = NULL;
    }

    const std::size_t begin = append_only ? 0 : 1;
    for (std::size_t i = begin; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        std::size_t sep = std::string::npos;
        bool esc = false;
        for (std::size_t j = 0; j < tok.size(); ++j) {
            if (esc) { esc = false; continue; }
            if (tok[j] == '`') { esc = true; continue; }
            if (tok[j] == ':') { sep = j; break; }
        }
        if (sep == std::string::npos) continue;
        std::string name = unescape(tok.substr(0, sep));
        std::string value = unescape(tok.substr(sep + 1));
        NodeType type = NodeType::Attribute;
        if (name == ".") type = NodeType::TextAttribute;
        else if (name == "#") type = NodeType::ListAttribute;
        target->add_attribute(AttributeValue(name, value, type));
        if (name == "@") target->set_id(value);
    }
    reindex_markers(target);
}

void Document::attach_lob_payload(const std::vector<unsigned char>& bytes) {
    if (!pending_lob_.active()) throw Error("internal pending lob state missing");
    AttributeValue attr(pending_lob_.attr_name, std::string(), pending_lob_.attr_type);
    attr.lob.media_type = pending_lob_.media_type;
    attr.lob.declared_size = pending_lob_.size;
    attr.lob.data = bytes;
    pending_lob_.node->add_attribute(attr);
    pending_lob_.reset();
}

void Document::parse_line(const std::string& raw_line) {
    if (pending_lob_.active()) throw Error("parse_line cannot accept LOB payload; use parse_stream or parse_file");
    ++line_number_;
    std::string line = raw_line;
    if (!line.empty() && line[line.size() - 1] == '\n') line.erase(line.size() - 1);
    if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
    if (line.empty()) return;
    if (line[0] == ' ') return;
    if (line[0] == '@') parse_selector_line(line);
    else if (line[0] == '#') parse_operation_line(line);
    else if (line[0] == '.') parse_dot_line(line);
    else parse_configuration_line(line);
}

void Document::parse_stream(std::istream& input) {
    std::string line;
    while (true) {
        if (!std::getline(input, line)) break;
        parse_line(line + "\n");
        if (pending_lob_.active()) {
            std::vector<unsigned char> data(pending_lob_.size);
            if (pending_lob_.size != 0U) {
                input.read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(pending_lob_.size));
                if (static_cast<std::size_t>(input.gcount()) != pending_lob_.size) {
                    throw Error("LOB payload truncated at line " + std::to_string(line_number_));
                }
            }
            attach_lob_payload(data);
            int ch = input.peek();
            if (ch == '\r') { input.get(); ch = input.peek(); }
            if (ch == '\n') input.get();
        }
    }
    if (pending_lob_.active()) throw Error("unterminated LOB payload");
}

void Document::parse_file(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) throw Error("unable to open file: " + path);
    parse_stream(in);
}

void Document::serialize_attrs(const Node* node, std::string& out, bool) const {
    const std::vector<AttributeValue>& attrs = node->attributes();
    for (std::size_t i = 0; i < attrs.size(); ++i) {
        if (attrs[i].deleted || attrs[i].has_lob()) continue;
        out.push_back(' ');
        out += escape(attrs[i].name);
        out.push_back(':');
        out += escape(attrs[i].value);
    }
}

void Document::serialize_node(const Node* node, std::string& out, bool pretty) const {
    if (!node) return;
    if (node->type() == NodeType::Configuration) {
        out += escape(node->name()) + ":" + escape(node->value()) + "\n";
        return;
    }
    if (node->type() != NodeType::Element) return;
    out.append(node->depth(), '.');
    out += escape(node->name());
    serialize_attrs(node, out, pretty);
    out += "\n";
    const std::vector<AttributeValue>& attrs = node->attributes();
    for (std::size_t i = 0; i < attrs.size(); ++i) {
        const AttributeValue& attr = attrs[i];
        if (attr.deleted || !attr.has_lob()) continue;
        out += "@ " + escape(node->id()) + "\n";
        out += "# <:" + escape(attr.name) + "," + (attr.type == NodeType::ClobAttribute ? "clob" : "blob") + "," + escape(attr.lob.media_type) + "," + std::to_string(attr.lob.declared_size) + "\n";
        out.append(reinterpret_cast<const char*>(attr.lob.data.data()), attr.lob.data.size());
        out += "\n";
    }
    const std::vector<std::unique_ptr<Node> >& kids = node->children();
    for (std::size_t i = 0; i < kids.size(); ++i) serialize_node(kids[i].get(), out, pretty);
}

std::string Document::dump(bool pretty) const {
    std::string out;
    const std::vector<std::unique_ptr<Node> >& kids = root_->children();
    for (std::size_t i = 0; i < kids.size(); ++i) serialize_node(kids[i].get(), out, pretty);
    return out;
}

} // namespace libdot
