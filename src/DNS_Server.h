#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string>

class DNS_Server {
private:
    int sock;                   // Сокет для UDP-соединения
    sockaddr_in server_addr;    // Адрес сервера
    std::vector<uint8_t> buffer; // Буфер для данных

public:
    // Конструктор
    DNS_Server(int port = 53);

    // Деструктор
    ~DNS_Server();

    // Запуск сервера
    void start();

private:
    // Обработка DNS-запроса
    std::vector<uint8_t> handle_dns_request(const std::vector<uint8_t>& request);

    // Формирование DNS-ответа
    std::vector<uint8_t> build_dns_response(const std::vector<uint8_t>& request);
};

#endif // DNS_SERVER_H