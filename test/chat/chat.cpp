#include "chat.h"


std::atomic<bool> running(true);
std::mutex mtx;  // Для синхронизации вывода сообщений

// Функция для получения размера окна терминала
void getTerminalSize(int& width, int& height) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    width = w.ws_col;
    height = w.ws_row;
}

// Функция для вывода чата
void displayChat(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << message << std::endl;
}

// Функция для получения сообщений от собеседника
void receiveMessages() {
    std::string msg;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(3));  // Симуляция задержки
        // Симуляция получения сообщения от собеседника
        msg = "Собеседник: Как жизнь?";
        displayChat(msg);
    }
}

// Функция для получения служебной информации
void receiveSystemMessages() {
    std::string msg;
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));  // Симуляция задержки
        // Симуляция получения служебного сообщения
        msg = "Служебная информация: Все системы работают корректно.";
        displayChat(msg);
    }
}

// Функция для ввода сообщений
void sendMessage() {
    std::string input;
    int width, height;
    getTerminalSize(width, height);

    while (running) {
        std::cout << "Введите сообщение (или 'q' для выхода): ";
        std::getline(std::cin, input);

        if (input == "q") {
            running = false;
            break;
        } else if (input.empty()) {
            continue;
        }

        // Добавляем текст "Вы:" перед сообщением пользователя
        std::string message = "Вы: " + input;
        displayChat(message);
    }
}

void chatMode() {
    // Запускаем асинхронные потоки
    std::thread receiver(receiveMessages);
    std::thread systemInfoReceiver(receiveSystemMessages);
    std::thread sender(sendMessage);

    // Ожидаем завершения работы всех потоков
    sender.join();
    receiver.join();
    systemInfoReceiver.join();
}