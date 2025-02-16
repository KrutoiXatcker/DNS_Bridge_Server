#include "DNS_Server.h"

int main() {
    try {
        // Создаем DNS-сервер на порту 53
        DNS_Server server(53);

        // Запускаем сервер
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}