#include "libdot/dot.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: dot_dump <file.dot>\n";
        return 2;
    }
    try {
        libdot::Document doc;
        doc.parse_file(argv[1]);
        std::cout << doc.dump(true);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
