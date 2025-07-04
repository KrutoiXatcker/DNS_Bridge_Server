#include "DNS_Server.h"
#include <iomanip>

// Конструктор
DNS_Server::DNS_Server(int port) {
    // Создаем UDP-сокет
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    crate_flag = 0;

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

    // Загрузка базы данных доменов
    load_hostnames("hostname.txt");

    bayt_cripted("db");
}

// Деструктор
DNS_Server::~DNS_Server() {
    if (sock >= 0) {
        close(sock);
    }
}

// Загрузка базы данных доменов из файла
void DNS_Server::load_hostnames(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string domain, ip;
        if (iss >> domain >> ip) {
            hostnames[domain] = ip;
        }
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
    // Извлекаем доменное имя из запроса
    std::string domain = extract_domain_name(request);

    // Ищем IP-адрес в базе данных
    auto it = hostnames.find(domain);

    if (it != hostnames.end()) {
        // Обработка зашифрованных данных
        if (hostnames_cripted.find(domain) != hostnames_cripted.end()) {
            const std::string& cripted_value = hostnames_cripted[domain];

            if (cripted_value == "256") {
                // Очистка буфера
                bufer.clear();

            } else if (cripted_value == "257") {
                // Запись буфера в файл
                std::string outfile = "123";
                std::ofstream file(outfile, std::ios::binary);

                if (file.is_open()) {
                    file.write(reinterpret_cast<const char*>(bufer.data()), bufer.size());
                    file.close();
                } else {
                    std::cerr << "Ошибка открытия файла для записи: " << outfile << std::endl;
                }

            } else {
                // Добавление значения в буфер
                try {
                    uint8_t value = static_cast<uint8_t>(std::stoul(cripted_value));
                    bufer.push_back(value);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Ошибка преобразования строки в число: " << cripted_value << std::endl;
                }
            }
        }

        // Логирование
        std::cout <<std::setw(20)<< domain << "\t" << it->second << "\tyes\t" << hostnames_cripted[domain] << std::endl;

        // Возвращаем ответ
        return build_dns_response(request, it->second);
    } else {
        // Если домен не найден, возвращаем ошибку
        //std::cout << domain << "\tnot found\tno\t" << std::endl;
        return build_dns_response(request, "0.0.0.0");
    }
}

// Извлечение доменного имени из DNS-запроса
std::string DNS_Server::extract_domain_name(const std::vector<uint8_t>& request) {
    std::string domain;
    size_t pos = 12; // Пропускаем заголовок

    while (request[pos] != 0) {
        size_t len = request[pos];
        pos++;
        for (size_t i = 0; i < len; i++) {
            domain += request[pos++];
        }
        domain += '.';
    }

    if (!domain.empty()) {
        domain.pop_back(); // Убираем последнюю точку
    }

    return domain;
}

// Формирование DNS-ответа
std::vector<uint8_t> DNS_Server::build_dns_response(const std::vector<uint8_t>& request, const std::string& ip) {
    std::vector<uint8_t> response = request; // Копируем запрос

    // Меняем флаги в заголовке (устанавливаем бит ответа)
    response[2] |= 0x80; // Устанавливаем бит QR (ответ)

    // Добавляем ответ
    uint8_t answer[16];
    size_t answer_len = 0;

    // Указатель на имя (сжатое)
    answer[answer_len++] = 0xC0;
    answer[answer_len++] = 0x0C;

    // Тип A
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x01;

    // Класс IN
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x01;

    // TTL (60 секунд)
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x3C;

    // Длина данных (4 байта)
    answer[answer_len++] = 0x00;
    answer[answer_len++] = 0x04;

    // IP-адрес
    std::istringstream iss(ip);
    std::string part;
    while (std::getline(iss, part, '.')) {
        answer[answer_len++] = static_cast<uint8_t>(std::stoi(part));
    }

    // Добавляем ответ в пакет
    response.insert(response.end(), answer, answer + answer_len);

    // Обновляем количество ответов в заголовке
    response[6] = 0x00;
    response[7] = 0x01; // ANCOUNT = 1

    return response;
}


// Загрузка базы данных доменов из файла
void DNS_Server::bayt_cripted(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string domain,id;
        if (iss >> domain >> id) {
            hostnames_cripted[domain] = id;
        }
    }
}