#include "libdot/dot.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

static void test_basic_tree() {
    libdot::Document doc;
    std::istringstream in(
        "version:1\n"
        ".html\n"
        "..body @:body1; class:main; #:ram,ddr4;\n"
        "...h1 .:Hello World;\n"
    );
    doc.parse_stream(in);
    const libdot::Node* html = doc.get_node(doc.root(), "html");
    assert(html != NULL);
    const libdot::Node* body = doc.get_node(html, "body");
    assert(body != NULL);
    assert(body->id() == "body1");
    assert(body->find_attribute("class")->value == "main");
    const libdot::Node* h1 = doc.get_node(body, "h1");
    assert(h1 != NULL);
    assert(h1->find_attribute(".")->value == "Hello World");
}

static void test_selector_ops() {
    libdot::Document doc;
    std::istringstream in(
        ".src @:src1; text:Hello ;\n"
        ".dst @:dst1; text:World;\n"
        "@ src1\n"
        "@ dst1\n"
        "# +:text,0,text,0\n"
    );
    doc.parse_stream(in);
    const libdot::Node* dst = doc.get_node_by_id("dst1");
    assert(dst != NULL);
    assert(dst->find_attribute("text")->value == "WorldHello ");
}

static void test_duplicate_attrs_and_delete() {
    libdot::Document doc;
    std::istringstream in(
        ".n @:n1; a:one; a:two;\n"
        "@ n1\n"
        "# -:a,0\n"
    );
    doc.parse_stream(in);
    const libdot::Node* n = doc.get_node_by_id("n1");
    assert(n->find_attribute("a")->value == "two");
}

static void test_semicolon_escaping_and_multiline_append() {
    libdot::Document doc;
    std::istringstream in(
        ".html\n"
        "..body .:This is a text data and it is supposed to\n"
        "...+ .:be multiline\\; so, that it can be easily read.;\n"
    );
    doc.parse_stream(in);
    const libdot::Node* html = doc.get_node(doc.root(), "html");
    assert(html != NULL);
    const libdot::Node* body = doc.get_node(html, "body");
    assert(body != NULL);
    const libdot::AttributeValue* text = body->find_attribute(".");
    assert(text != NULL);
    assert(text->value == "This is a text data and it is supposed tobe multiline; so, that it can be easily read.");
    const std::string dumped = doc.dump(false);
    assert(dumped.find(".:This is a text data and it is supposed to;") != std::string::npos);
    assert(dumped.find("\\;") == std::string::npos || dumped.find("read.\\;") == std::string::npos);
}

static void test_blob_roundtrip() {
    libdot::Document doc;
    std::string payload = "ABC\0XYZ";
    payload.push_back('!');
    std::ostringstream input;
    input << ".lob @:lob1;\n"
          << "@ lob1\n"
          << "# <:blob1,blob,application/octet-stream," << payload.size() << "\n";
    input.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    input << "\n";
    std::istringstream in(input.str());
    doc.parse_stream(in);
    const libdot::Node* lob = doc.get_node_by_id("lob1");
    const libdot::AttributeValue* attr = lob->find_attribute("blob1");
    assert(attr != NULL);
    assert(attr->type == libdot::NodeType::BlobAttribute);
    assert(attr->lob.declared_size == payload.size());
    assert(std::string(attr->lob.data.begin(), attr->lob.data.end()) == payload);
    const std::string dumped = doc.dump(false);
    assert(dumped.find("# <:blob1,blob,application/octet-stream") != std::string::npos);
}

int main() {
    test_basic_tree();
    test_selector_ops();
    test_duplicate_attrs_and_delete();
    test_semicolon_escaping_and_multiline_append();
    test_blob_roundtrip();
    std::cout << "All tests passed\n";
    return 0;
}
