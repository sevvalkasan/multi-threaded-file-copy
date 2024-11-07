#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace fs = std::filesystem;

constexpr size_t BUFFER_SIZE = 4096; 

void my_copy_file(const fs::path& source, const fs::path& destination) {
    std::ifstream source_file(source, std::ios::binary);
    std::ofstream destination_file(destination, std::ios::binary);

    if (!source_file) {
        std::cerr << "Error: Could not open source file: " << source << std::endl;
        return;
    }

    if (!destination_file) {
        std::cerr << "Error: Could not create destination file: " << destination << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        source_file.read(buffer, BUFFER_SIZE);
        std::streamsize bytes_read = source_file.gcount();
        if (bytes_read <= 0)
            break;
        destination_file.write(buffer, bytes_read);
    }
}

void copy_directory_recursive(const fs::path& source_dir, const fs::path& destination_dir, std::atomic<int>& files_copied, std::mutex& mtx) {
    for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
        if (fs::is_regular_file(entry)) {
            const fs::path& source = entry.path();
            const fs::path& relative_path = fs::relative(source, source_dir);
            const fs::path& destination = destination_dir / relative_path;
            fs::create_directories(destination.parent_path());

            {
                std::lock_guard<std::mutex> lock(mtx);
                my_copy_file(source, destination);
                files_copied++;
            }
        }
    }
}

void copy_directory_for(const fs::path& source_dir, const fs::path& destination_dir, std::atomic<int>& files_copied, std::mutex& mtx) {
    for (const auto& entry : fs::directory_iterator(source_dir)) {
        if (fs::is_regular_file(entry)) {
            const fs::path& source = entry.path();
            const fs::path& relative_path = fs::relative(source, source_dir);
            const fs::path& destination = destination_dir / relative_path;
            fs::create_directories(destination.parent_path());

            {
                std::lock_guard<std::mutex> lock(mtx);
                my_copy_file(source, destination);
                files_copied++;
            }
        }
    }
}

void copy_files(const fs::path& source_dir, const fs::path& destination_dir) {
    if (fs::exists(source_dir) && fs::is_directory(source_dir)) {
        fs::create_directories(destination_dir);

        std::atomic<int> files_copied_recursive(0);
        std::atomic<int> files_copied_for(0);
        std::mutex mtx;

        auto start_recursive = std::chrono::steady_clock::now();
        copy_directory_recursive(source_dir, destination_dir, files_copied_recursive, mtx);
        auto end_recursive = std::chrono::steady_clock::now();
        std::cout << "Recursive copy completed in: " << std::chrono::duration<double>(end_recursive - start_recursive).count() << " seconds" << std::endl;

        auto start_for = std::chrono::steady_clock::now();
        copy_directory_for(source_dir, destination_dir, files_copied_for, mtx);
        auto end_for = std::chrono::steady_clock::now();
        std::cout << "For loop copy completed in: " << std::chrono::duration<double>(end_for - start_for).count() << " seconds" << std::endl;

        std::cout << "Total files copied (recursive): " << files_copied_recursive << std::endl;
        std::cout << "Total files copied (for loop): " << files_copied_for << std::endl;
    } else {
        std::cerr << "Error: Source directory does not exist or is not a directory." << std::endl;
    }
}

int main() {
    fs::path source_dir = "/home/sevval/Masaüstü/sysprg/dsy";
    fs::path destination_dir = "/home/sevval/Masaüstü/sysprg/dsy_copy";

    copy_files(source_dir, destination_dir);

    return 0;
}

