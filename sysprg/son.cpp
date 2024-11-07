#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace fs = std::filesystem;

std::mutex mtx;
std::atomic<int> fileCount(0);

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);
    void waitForCompletion();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
    std::atomic<int> activeTasks;
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false), activeTasks(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    activeTasks++;
                }
                task();
                activeTasks--;
                condition.notify_all();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    condition.wait(lock, [this] { return tasks.empty() && activeTasks == 0; });
}

void copyFileWithVector(const fs::path& sourceFile, const fs::path& destinationFile) {
    const std::streamsize bufferSize = 8 * 1024 * 1024; // 8MB buffer size
    std::ifstream inputFile(sourceFile, std::ios::binary);
    if (!inputFile) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Kaynak dosya açılamadı! Dosya yolu: " << sourceFile.string() << std::endl;
        return;
    }

    std::ofstream outputFile(destinationFile, std::ios::binary);
    if (!outputFile) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Hedef dosya açılamadı! Dosya yolu: " << destinationFile.string() << std::endl;
        return;
    }

    std::vector<char> buffer(bufferSize);
    while (inputFile.read(buffer.data(), bufferSize)) {
        outputFile.write(buffer.data(), inputFile.gcount());
    }
    // Write any remaining bytes
    outputFile.write(buffer.data(), inputFile.gcount());

    fileCount++;
}

void copyFilesRecursively(const fs::path& sourceDir, const fs::path& destinationDir, ThreadPool& pool) {
    if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Kaynak dizin açılamadı! Dizin yolu: " << sourceDir.string() << std::endl;
        return;
    }

    if (!fs::exists(destinationDir)) {
        if (!fs::create_directories(destinationDir)) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cerr << "Hedef dizin oluşturulamadı! Dizin yolu: " << destinationDir.string() << std::endl;
            return;
        }
    }

    for (const auto& entry : fs::directory_iterator(sourceDir)) {
        const auto& path = entry.path();
        const auto& filename = path.filename();

        if (fs::is_regular_file(path)) {
            pool.enqueue([path, destinationFile = destinationDir / filename] {
                copyFileWithVector(path, destinationFile);
            });
        } else if (fs::is_directory(path)) {
            pool.enqueue([path, destinationDir, &pool] {
                copyFilesRecursively(path, destinationDir / path.filename(), pool);
            });
        }
    }
}

int main() {
    fs::path sourceDir = "/home/sevval/Masaüstü/sysprg/dsy/";
    fs::path destinationDir = "/home/sevval/Masaüstü/sysprg/dsy_cpy/";

    const size_t numThreads = std::thread::hardware_concurrency(); // Use the number of hardware threads
    ThreadPool pool(numThreads);

    copyFilesRecursively(sourceDir, destinationDir, pool);

    pool.waitForCompletion(); // Wait for all tasks to complete

    std::cout << "Tüm dosyalar ve dizinler başarıyla kopyalandı!" << std::endl;
    std::cout << "Toplam kopyalanan dosya sayısı: " << fileCount.load() << std::endl;

    return 0;
}

