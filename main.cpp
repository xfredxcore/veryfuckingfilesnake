#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <random>
#include <algorithm>
#include <unistd.h>
#include <ncurses.h>

namespace fs = std::filesystem;

const std::string VERSION = "1.0.1-real-wipe";

struct Point {
    int x, y;
};

void print_help(const char* prog_name) {
    std::cout << "Very Fucking File Snake (VFFS)\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << prog_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help menu\n";
    std::cout << "  -v, --version    Show version info\n";
    std::cout << "  -d <path>        Specify the target directory (default: current directory)\n";
}

void check_not_root() {
    if (getuid() == 0 || geteuid() == 0) {
        std::cerr << "ERROR: Running as root is strictly prohibited.\n";
        exit(EXIT_FAILURE);
    }
}

void fucking_confirmations(const fs::path& target_dir) {
    std::cout << "WARNING: You are running VFFS in: " << target_dir << "\n";
    std::cout << "If you lose, this directory will be wiped clean.\n\n";

    std::vector<std::string> questions = {
        "Are you absolutely sure? (yes/no): ",
        "Are you completely out of your mind? (yes/no): ",
        "Important files might be deleted. Do you care? (yes/no): ",
        "LAST CHANCE TO BACK OUT. Ready to risk it all? (yes/no): "
    };

    std::string answer;
    for (const auto& q : questions) {
        std::cout << q;
        std::cin >> answer;
        if (answer != "yes" && answer != "YES" && answer != "y") {
            std::cout << "Exit. Smart choice.\n";
            exit(EXIT_SUCCESS);
        }
    }
}

std::vector<fs::path> get_target_files(const fs::path& base_dir, const fs::path& backup_dir) {
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(base_dir)) {
        if (entry.is_regular_file()) {
            auto filename = entry.path().filename().string();
            if (filename[0] != '.' && entry.path() != backup_dir) {
                files.push_back(entry.path());
            }
        }
    }
    return files;
}

void wipe_directory(const fs::path& target_dir) {
    endwin();
    std::cout << "GAME OVER! The snake bit its own tail.\n";
    std::cout << "Wiping directory: " << target_dir << " ...\n";

    try {
        for (const auto& entry : fs::directory_iterator(target_dir)) {
            fs::remove_all(entry.path());
        }
        std::cout << "Directory successfully formatted. Nothing survived.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error during wipe: " << e.what() << "\n";
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    fs::path target_dir = fs::current_path();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "VFFS Version: " << VERSION << "\n";
            return 0;
        } else if (arg == "-d") {
            if (i + 1 < argc) {
                target_dir = fs::absolute(argv[++i]);
            } else {
                std::cerr << "ERROR: -d requires a path argument.\n";
                return 1;
            }
        } else {
            std::cerr << "UNKNOWN OPTION: " << arg << "\n";
            print_help(argv[0]);
            return 1;
        }
    }

    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "ERROR: Target path does not exist or is not a directory.\n";
        return 1;
    }

    check_not_root();
    fucking_confirmations(target_dir);

    fs::path backup_dir = target_dir / "snakebackup";
    fs::create_directory(backup_dir);

    std::vector<fs::path> available_files = get_target_files(target_dir, backup_dir);
    if (available_files.empty()) {
        std::cout << "No files to eat in target directory. Add some dummy files (or something like vmlinuz* if you are fucking archlinux/cachyos-shit user lolxd) and return.\n";
        return 0;
    }

    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    int height = 20;
    int width = 50;

    std::vector<Point> snake = {{width / 2, height / 2}};
    int dir_x = 1, dir_y = 0;

    std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<size_t> dist(0, available_files.size() - 1);
    fs::path current_apple_file = available_files[dist(rng)];

    std::uniform_int_distribution<int> rand_x(1, width - 2);
    std::uniform_int_distribution<int> rand_y(1, height - 2);
    Point apple = {rand_x(rng), rand_y(rng)};

    bool game_over = false;

    while (!game_over) {
        int ch = getch();
        switch (ch) {
            case KEY_UP:    if (dir_y == 0) { dir_x = 0; dir_y = -1; } break;
            case KEY_DOWN:  if (dir_y == 0) { dir_x = 0; dir_y = 1; } break;
            case KEY_LEFT:  if (dir_x == 0) { dir_x = -1; dir_y = 0; } break;
            case KEY_RIGHT: if (dir_x == 0) { dir_x = 1; dir_y = 0; } break;
            case 'q':       game_over = true; break;
        }

        Point next_head = {snake.front().x + dir_x, snake.front().y + dir_y};

        if (next_head.x >= width - 1) next_head.x = 1;
        else if (next_head.x <= 0) next_head.x = width - 2;
        if (next_head.y >= height - 1) next_head.y = 1;
        else if (next_head.y <= 0) next_head.y = height - 2;

        for (const auto& part : snake) {
            if (part.x == next_head.x && part.y == next_head.y) {
                wipe_directory(target_dir);
            }
        }

        snake.insert(snake.begin(), next_head);

        if (next_head.x == apple.x && next_head.y == apple.y) {
            try {
                fs::rename(current_apple_file, backup_dir / current_apple_file.filename());
            } catch (...) {}

            available_files = get_target_files(target_dir, backup_dir);
            if (available_files.empty()) {
                endwin();
                std::cout << "VICTORY! You ate all files from target directory and saved them.\n";
                exit(EXIT_SUCCESS);
            }

            std::uniform_int_distribution<size_t> new_dist(0, available_files.size() - 1);
            current_apple_file = available_files[new_dist(rng)];
            apple = {rand_x(rng), rand_y(rng)};
        } else {
            snake.pop_back();
        }

        clear();
        for (int i = 0; i < width; ++i) { mvaddch(0, i, '#'); mvaddch(height - 1, i, '#'); }
        for (int i = 0; i < height; ++i) { mvaddch(i, 0, '#'); mvaddch(i, width - 1, '#'); }

        mvaddch(apple.y, apple.x, '@');

        for (size_t i = 0; i < snake.size(); ++i) {
            mvaddch(snake[i].y, snake[i].x, i == 0 ? 'O' : 'o');
        }

        mvprintw(height + 1, 0, "Target file: %s", current_apple_file.filename().c_str());
        mvprintw(height + 2, 0, "Files left: %lu", available_files.size());

        refresh();
        usleep(100000);
    }

    endwin();
    return 0;
}