#include "Input.h"

#include <iostream>
#include <limits>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <sys/select.h>

char getch() {
    char buf = 0;
    termios old = {};
    if (tcgetattr(STDIN_FILENO, &old) < 0) perror("tcgetattr");
    termios now = old;
    now.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
    now.c_cc[VMIN] = 1;
    now.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &now) < 0) perror("tcsetattr");
    if (read(STDIN_FILENO, &buf, 1) < 0) perror("read");
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0) perror("tcsetattr");
    return buf;
}

bool kbhit() {
    termios old = {};
    if (tcgetattr(STDIN_FILENO, &old) < 0) return false;

    termios now = old;
    now.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
    if (tcsetattr(STDIN_FILENO, TCSANOW, &now) < 0) return false;

    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv);

    tcsetattr(STDIN_FILENO, TCSADRAIN, &old);
    return result > 0 && FD_ISSET(STDIN_FILENO, &readfds);
}

void drain_input() {
    termios old = {};
    if (tcgetattr(STDIN_FILENO, &old) < 0) return;

    termios now = old;
    now.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &now);

    int old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    char ch = 0;
    while (read(STDIN_FILENO, &ch, 1) > 0) {}

    fcntl(STDIN_FILENO, F_SETFL, old_flags);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &old);
}

int read_int_safe(int fallback) {
    int value;
    if (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return fallback;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void wait_key() {
    std::cout << "\n[아무 키나 누르면 복귀]" << std::flush;
    drain_input();
    getch();
    drain_input();
}
