#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace fs = std::filesystem;

constexpr size_t BUFFER_SIZE = 4096; // Sabit bir buffer boyutu

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

    std::cout << "File " << source << " copied to " << destination << std::endl;
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

void copy_files(const fs::path& source_dir, const fs::path& destination_dir) {
    if (fs::exists(source_dir) && fs::is_directory(source_dir)) {
        fs::create_directories(destination_dir);

        std::atomic<int> files_copied(0);
        std::mutex mtx;
        std::vector<std::thread> threads;


        for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
            threads.emplace_back(copy_directory_recursive, std::cref(source_dir), std::cref(destination_dir), std::ref(files_copied), std::ref(mtx));
        }


        for (auto& thread : threads) {
            thread.join();
        }

        std::cout << "Total files copied: " << files_copied << std::endl;
    } else {
        std::cerr << "Error: Source directory does not exist or is not a directory." << std::endl;
    }
}

int main() {
    fs::path source_dir = "/home/sevval/Masaüstü/sysprg/dsy"; // Kaynak dizin yolu
    fs::path destination_dir = "/home/sevval/Masaüstü/sysprg/dsy_copy"; // Hedef dizin yolu

    copy_files(source_dir, destination_dir);

    return 0;
}

