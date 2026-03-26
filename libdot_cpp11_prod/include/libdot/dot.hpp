#ifndef LIBDOT_DOT_HPP
#define LIBDOT_DOT_HPP

#include <cstddef>
#include <cstdint>
#include <istream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace libdot {

enum class NodeType {
    Root = 0,
    Configuration,
    Comment,
    Empty,
    SpaceLine,
    Marker,
    Operation,
    Element,
    Attribute,
    TextAttribute,
    ListAttribute,
    ResultAttribute,
    ResultListAttribute,
    BlobAttribute,
    ClobAttribute
};

enum class NodeState {
    Normal = 0,
    Deleted,
    Hidden
};

struct LobValue {
    std::string media_type;
    std::size_t declared_size;
    std::vector<unsigned char> data;

    LobValue();
    bool empty() const;
};

struct AttributeValue {
    std::string name;
    std::string value;
    NodeType type;
    bool deleted;
    LobValue lob;

    AttributeValue();
    AttributeValue(const std::string& n, const std::string& v, NodeType t);
    bool has_lob() const;
};

class Node {
public:
    explicit Node(NodeType type = NodeType::Element);

    NodeType type() const;
    NodeState state() const;
    void set_state(NodeState state);

    std::size_t depth() const;
    void set_depth(std::size_t depth);

    const std::string& name() const;
    void set_name(const std::string& name);

    const std::string& value() const;
    void set_value(const std::string& value);

    const std::string& id() const;
    void set_id(const std::string& id);

    Node* parent() const;
    Node* first_child() const;
    Node* next_sibling() const;
    Node* prev_sibling() const;

    const std::vector<std::unique_ptr<Node> >& children() const;
    std::vector<std::unique_ptr<Node> >& children();

    const std::vector<AttributeValue>& attributes() const;
    std::vector<AttributeValue>& attributes();

    void add_child(std::unique_ptr<Node> child);
    void add_attribute(const AttributeValue& attr);
    AttributeValue* find_attribute(const std::string& name, std::size_t occurrence = 0, bool include_deleted = false);
    const AttributeValue* find_attribute(const std::string& name, std::size_t occurrence = 0, bool include_deleted = false) const;
    std::vector<const AttributeValue*> find_attributes(const std::string& name, bool include_deleted = false) const;
    std::vector<std::string> tags() const;

private:
    NodeType type_;
    NodeState state_;
    std::size_t depth_;
    std::string name_;
    std::string value_;
    std::string id_;
    Node* parent_;
    Node* prev_sibling_;
    Node* next_sibling_;
    std::vector<std::unique_ptr<Node> > children_;
    std::vector<AttributeValue> attributes_;

    friend class Document;
};

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& message);
};

class Document {
public:
    Document();

    Node* root();
    const Node* root() const;

    void clear();

    void parse_line(const std::string& line);
    void parse_stream(std::istream& input);
    void parse_file(const std::string& path);

    std::string dump(bool pretty = false) const;

    Node* current_node();
    const Node* current_node() const;
    Node* previous_node();
    const Node* previous_node() const;

    Node* get_node(Node* from, const std::string& name);
    const Node* get_node(const Node* from, const std::string& name) const;
    Node* get_node_by_id(const std::string& id);
    const Node* get_node_by_id(const std::string& id) const;

    std::size_t line_number() const;

    static std::string escape_value(const std::string& value);
    static std::string unescape_value(const std::string& value);

private:
    struct PendingLob {
        Node* node;
        std::string attr_name;
        NodeType attr_type;
        std::string media_type;
        std::size_t size;
        PendingLob();
        bool active() const;
        void reset();
    };

    std::unique_ptr<Node> root_;
    std::size_t line_number_;
    Node* previous_node_;
    Node* current_node_;
    Node* last_node_before_selector_;
    Node* from_node_;
    Node* to_node_;
    std::vector<Node*> depth_stack_;
    std::vector<std::pair<std::string, Node*> > marker_index_;
    PendingLob pending_lob_;

    Node* ensure_parent_for_depth(std::size_t depth);
    void parse_configuration_line(const std::string& line);
    void parse_selector_line(const std::string& line);
    void parse_operation_line(const std::string& line);
    void parse_dot_line(const std::string& line);
    void attach_lob_payload(const std::vector<unsigned char>& bytes);

    static std::vector<std::string> split_tokens(const std::string& text);
    static std::vector<std::string> split_escaped(const std::string& text, char separator);
    static std::size_t count_leading_dots(const std::string& line);
    static std::string trim(const std::string& text);

    void reindex_markers(Node* node);
    void serialize_node(const Node* node, std::string& out, bool pretty) const;
    void serialize_attrs(const Node* node, std::string& out, bool pretty) const;

    void op_append(const std::vector<std::string>& args);
    void op_assign(const std::vector<std::string>& args);
    void op_delete(const std::vector<std::string>& args);
    void op_link(const std::vector<std::string>& args);
    void op_tag_search(const std::vector<std::string>& args, bool invert);
    void op_split(const std::vector<std::string>& args);
    void op_blob(const std::vector<std::string>& args);
};

} // namespace libdot

#endif
