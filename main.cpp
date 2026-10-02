#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Не удалось создать сокет\n";
        return 1;
    }
    sockaddr_in address{}; 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(8080);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Ошибка привязки сокета к порту\n";
        close(server_fd); 
        return 1;
    }
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Ошибка при включении прослушивания\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Сервер успешно запущен\n";
    int addrlen = sizeof(address);
    int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

    if (client_socket < 0) {
        std::cerr << "Ошибка при принятии подключения клиента\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Клиент успешно подключился! Новый дескриптор: " << client_socket << "\n";
    char buffer[1024] = {0};

    int bytes_read = read(client_socket, buffer, 1024);
    if (bytes_read > 0) {
        std::cout << "Клиент прислал: " << buffer << "\n";
        send(client_socket, buffer, bytes_read, 0);
        std::cout << "Эхо-ответ отправлен!\n";
    }
    close(client_socket);
    close(server_fd);
    std::cout << "Сервер закрыл все соединения.\n";

    return 0;
}