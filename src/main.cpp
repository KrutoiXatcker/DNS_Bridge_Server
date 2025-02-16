#include "DNS_Server.h"

int main() {
    try {
        DNS_Server server;
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}