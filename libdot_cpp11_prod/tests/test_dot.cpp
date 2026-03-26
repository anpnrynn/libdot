#include "libdot/dot.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

static void test_basic_tree() {
    libdot::Document doc;
    std::istringstream in(
        "version:1\n"
        ".html\n"
        "..body @:body1 class:main #:ram,ddr4\n"
        "...h1 .:Hello_World\n"
    );
    doc.parse_stream(in);
    const libdot::Node* html = doc.get_node(doc.root(), "html");
    assert(html != NULL);
    const libdot::Node* body = doc.get_node(html, "body");
    assert(body != NULL);
    assert(body->id() == "body1");
    assert(body->find_attribute("class")->value == "main");
    assert(doc.dump(false).find("version:1") != std::string::npos);
}

static void test_selector_ops() {
    libdot::Document doc;
    std::istringstream in(
        ".src @:src1 text:Hello_\n"
        ".dst @:dst1 text:World\n"
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
        ".n @:n1 a:one a:two\n"
        "@ n1\n"
        "# -:a,0\n"
    );
    doc.parse_stream(in);
    const libdot::Node* n = doc.get_node_by_id("n1");
    assert(n->find_attribute("a")->value == "two");
}

static void test_tag_search_and_escape() {
    libdot::Document doc;
    std::istringstream in(
        ".mem1 @:m1 #:ram,ddr4`_2400Mhz\n"
        ".mem2 @:m2 #:ssd,nvme\n"
        ".result @:r1\n"
        "@ r1\n"
        "# ?:matches,ram\n"
    );
    doc.parse_stream(in);
    const libdot::Node* r = doc.get_node_by_id("r1");
    assert(r->find_attribute("matches")->value.find("m1") != std::string::npos);
}

static void test_blob_roundtrip() {
    libdot::Document doc;
    std::string payload = "ABC\0XYZ";
    payload.push_back('!');
    std::ostringstream input;
    input << ".lob @:lob1\n"
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
    test_tag_search_and_escape();
    test_blob_roundtrip();
    std::cout << "All tests passed\n";
    return 0;
}
