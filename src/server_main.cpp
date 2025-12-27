
#include "httplib.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

int main() {
    httplib::Server svr;

    std::string www_dir = "";
    
    // Check common locations for www directory
    if (fs::exists("www") && fs::is_directory("www")) {
        www_dir = "./www";
    } else if (fs::exists("../www") && fs::is_directory("../www")) {
        www_dir = "../www";
    }

    if (www_dir.empty()) {
        std::cerr << "Error: Could not find 'www' directory." << std::endl;
        std::cerr << "Please run from the project root or build directory." << std::endl;
        return 1;
    }

    std::cout << "Serving static files from: " << fs::absolute(www_dir) << std::endl;
    std::cout << "Server running at http://localhost:8080" << std::endl;

    // Mount the directory to root
    svr.set_mount_point("/", www_dir);

    // Serve index.html for root path
    svr.Get("/", [&](const httplib::Request&, httplib::Response& res) {
        std::string index_path = www_dir + "/index.html";
        std::ifstream file(index_path);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("Index file not found", "text/plain");
        }
    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}
