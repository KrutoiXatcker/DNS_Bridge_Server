#include "DNS_Server.h"

// Конструктор
DNS_Server::DNS_Server(int port) {
    // Создаем UDP-сокет
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу
    if (bind(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        close(sock);
        throw std::runtime_error("Failed to bind socket");
    }

    buffer.resize(512); // Буфер для данных
}

// Деструктор
DNS_Server::~DNS_Server() {
    if (sock >= 0) {
        close(sock);
    }
}

// Запуск сервера
void DNS_Server::start() {
    std::cout << "DNS Server is running on port 53..." << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Принимаем данные
        ssize_t len = recvfrom(sock, buffer.data(), buffer.size(), 0,
                               reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (len < 0) {
            std::cerr << "Failed to receive data" << std::endl;
            continue;
        }

        // Обрабатываем запрос
        std::vector<uint8_t> response = handle_dns_request(std::vector<uint8_t>(buffer.begin(), buffer.begin() + len));

        // Отправляем ответ
        if (sendto(sock, response.data(), response.size(), 0,
                   reinterpret_cast<sockaddr*>(&client_addr), client_len) < 0) {
            std::cerr << "Failed to send response" << std::endl;
        }
    }
}

// Обработка DNS-запроса
std::vector<uint8_t> DNS_Server::handle_dns_request(const std::vector<uint8_t>& request) {
    // Просто возвращаем фиктивный ответ
    return build_dns_response(request);
}

// Формирование DNS-ответа
std::vector<uint8_t> DNS_Server::build_dns_response(const std::vector<uint8_t>& request) {
    std::vector<uint8_t> response = request; // Копируем запрос

    // Меняем флаги в заголовке (устанавливаем бит ответа)
    response[2] |= 0x80; // Устанавливаем бит QR (ответ)

    // Добавляем фиктивный ответ
    // Пример: возвращаем IP-адрес 127.0.0.1
    uint8_t answer[] = {
        0xC0, 0x0C, // Указатель на имя (сжатое)
        0x00, 0x01, // Тип A
        0x00, 0x01, // Класс IN
        0x00, 0x00, 0x00, 0x3C, // TTL (60 секунд)
        0x00, 0x04, // Длина данных (4 байта)
        0x7F, 0x00, 0x00, 0x01 // IP-адрес (127.0.0.1)
    };

    // Добавляем ответ в пакет
    response.insert(response.end(), answer, answer + sizeof(answer));

    // Обновляем количество ответов в заголовке
    response[6] = 0x00;
    response[7] = 0x01; // ANCOUNT = 1

    return response;
}