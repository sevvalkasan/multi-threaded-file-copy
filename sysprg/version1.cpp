#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

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

    destination_file << source_file.rdbuf();

    std::cout << "File " << source << " copied to " << destination << std::endl;
}

void copy_directory_recursive(const fs::path& source_dir, const fs::path& destination_dir) {
    for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
        if (fs::is_regular_file(entry)) {
            const fs::path& source = entry.path();
            const fs::path& relative_path = fs::relative(source, source_dir);
            const fs::path& destination = destination_dir / relative_path;
            fs::create_directories(destination.parent_path());
            my_copy_file(source, destination);
        }
    }
}

int main() {
    fs::path source_dir = "/home/sevval/Masaüstü/sysprg/dsy"; // Kaynak dizin yolu
    fs::path destination_dir = "/home/sevval/Masaüstü/sysprg/dsy_copy"; // Hedef dizin yolu

    if (fs::exists(source_dir) && fs::is_directory(source_dir)) {
        fs::create_directories(destination_dir);
        copy_directory_recursive(source_dir, destination_dir);
        std::cout << "All files copied successfully!" << std::endl;
    } else {
        std::cerr << "Error: Source directory does not exist or is not a directory." << std::endl;
    }

    return 0;
}

