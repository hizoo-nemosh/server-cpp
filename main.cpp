#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
private:
  int m_fd = -1;

public:
  Socket(int fd) : m_fd(fd) {}
  virtual ~Socket() {
    if (m_fd != -1) {
      close(m_fd);
    }
  }
Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;
};

std::string calculator(const std::string &message) {
  std::stringstream ss(message);
  std::string command;
  int num1, num2;
  char operation;

  ss >> command;

  if (ss >> num1 >> operation >> num2) {
    if (operation == '+') {
      return "Ответ: " + std::to_string(num1 + num2) + "\n";
    }
    return "Ошибка: Неизвестный оператор. Только '+' поддерживает.\n";
  }
  return "Ошибка: Неизвестный синтаксис. Используйте: /getAnswer X+Y\n";
}

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "Не удалось создать сокет\n";
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Ошибка привязки сокета к порту\n";
    close(server_fd);
    return 1;
  }
  if (listen(server_fd, 3) < 0) {
    std::cerr << "Ошибка при включении прослушивания\n";
    close(server_fd);
    return 1;
  }

  std::cout << "Сервер успешно запущен на порту 8080...\n";
  int addrlen = sizeof(address);
  int client_socket =
      accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

  if (client_socket < 0) {
    std::cerr << "Ошибка при принятии подключения клиента\n";
    close(server_fd);
    return 1;
  }
  std::cout << "Клиент успешно подключился! Новый дескриптор: " << client_socket
            << "\n";

  while (true) {
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
      std::cout << "Клиент отключился или произошла ошибка.\n";
      break;
    }

    std::string message(buffer);
    while (!message.empty() &&
           (message.back() == '\n' || message.back() == '\r')) {
      message.pop_back();
    }

    std::cout << "Клиент прислал: \"" << message << "\"\n";

    if (message == "/exit") {
      std::string quit_msg = "Goodbye!\n";
      send(client_socket, quit_msg.c_str(), quit_msg.length(), 0);
      break;
    }

    if (message.find("/getAnswer") == 0) {
      std::string answer_msg = calculator(message);
      send(client_socket, answer_msg.c_str(), answer_msg.length(), 0);
      std::cout << "Отправлен результат калькулятора.\n";
      continue;
    }

    std::string response = "[Server echo]: " + message + "\n";
    send(client_socket, response.c_str(), response.length(), 0);
    std::cout << "Эхо-ответ отправлен!\n";
  }

  close(client_socket);
  close(server_fd);
  std::cout << "Сервер закрыл все соединения.\n";

  return 0;
}
