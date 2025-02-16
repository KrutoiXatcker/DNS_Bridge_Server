#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

class DNS_Server {
public:
    // Конструктор
    DNS_Server(int port);

    // Деструктор
    ~DNS_Server();

    // Запуск сервера
    void start();

private:
    // Загрузка базы данных доменов из файла
    void load_hostnames(const std::string& filename);

    // Обработка DNS-запроса
    std::vector<uint8_t> handle_dns_request(const std::vector<uint8_t>& request);

    // Извлечение доменного имени из DNS-запроса
    std::string extract_domain_name(const std::vector<uint8_t>& request);

    // Формирование DNS-ответа
    std::vector<uint8_t> build_dns_response(const std::vector<uint8_t>& request, const std::string& ip);

    // Сокет
    int sock;

    // Адрес сервера
    sockaddr_in server_addr;

    // Буфер для данных
    std::vector<uint8_t> buffer;

    // База данных доменов и IP-адресов
    std::unordered_map<std::string, std::string> hostnames;
};

#endif // DNS_SERVER_H