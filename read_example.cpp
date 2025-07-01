#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

int main() {
    std::cout << "=== read()システムコールの例 ===" << std::endl;
    
    // 例1: 標準入力から読み取る
    std::cout << "\n1. 標準入力からの読み取り" << std::endl;
    std::cout << "何か入力してEnterを押してください: ";
    std::cout.flush();
    
    char stdin_buffer[256];
    ssize_t bytes_read = read(STDIN_FILENO, stdin_buffer, sizeof(stdin_buffer) - 1);
    
    if (bytes_read == -1) {
        std::cerr << "標準入力の読み取りエラー: " << strerror(errno) << std::endl;
        return 1;
    }
    
    stdin_buffer[bytes_read] = '\0';
    std::cout << "読み取ったバイト数: " << bytes_read << std::endl;
    std::cout << "読み取った内容: " << stdin_buffer;
    
    // 例2: ファイルから読み取る
    std::cout << "\n2. ファイルからの読み取り" << std::endl;
    
    // テスト用ファイルを作成
    const char* test_filename = "test_read.txt";
    const char* test_content = "これはreadシステムコールのテストです。\n日本語も含まれています。\n複数行のテキストです。";
    
    int write_fd = open(test_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (write_fd == -1) {
        std::cerr << "テストファイルの作成エラー: " << strerror(errno) << std::endl;
        return 1;
    }
    
    if (write(write_fd, test_content, strlen(test_content)) == -1) {
        std::cerr << "テストファイルへの書き込みエラー: " << strerror(errno) << std::endl;
        close(write_fd);
        return 1;
    }
    close(write_fd);
    
    // ファイルを開いて読み取る
    int read_fd = open(test_filename, O_RDONLY);
    if (read_fd == -1) {
        std::cerr << "ファイルオープンエラー: " << strerror(errno) << std::endl;
        return 1;
    }
    
    char file_buffer[1024];
    bytes_read = read(read_fd, file_buffer, sizeof(file_buffer) - 1);
    
    if (bytes_read == -1) {
        std::cerr << "ファイル読み取りエラー: " << strerror(errno) << std::endl;
        close(read_fd);
        return 1;
    }
    
    file_buffer[bytes_read] = '\0';
    std::cout << "ファイル名: " << test_filename << std::endl;
    std::cout << "読み取ったバイト数: " << bytes_read << std::endl;
    std::cout << "読み取った内容:\n" << file_buffer << std::endl;
    
    close(read_fd);
    
    // 例3: 部分的な読み取り（小さなバッファで複数回読み取る）
    std::cout << "\n3. 小さなバッファで複数回読み取る" << std::endl;
    
    read_fd = open(test_filename, O_RDONLY);
    if (read_fd == -1) {
        std::cerr << "ファイルオープンエラー: " << strerror(errno) << std::endl;
        return 1;
    }
    
    char small_buffer[10];
    int read_count = 0;
    std::cout << "10バイトずつ読み取り:" << std::endl;
    
    while ((bytes_read = read(read_fd, small_buffer, sizeof(small_buffer) - 1)) > 0) {
        small_buffer[bytes_read] = '\0';
        std::cout << "  読み取り" << ++read_count << ": " << bytes_read << "バイト [" << small_buffer << "]" << std::endl;
    }
    
    if (bytes_read == -1) {
        std::cerr << "読み取りエラー: " << strerror(errno) << std::endl;
    }
    
    close(read_fd);
    
    // 例4: ノンブロッキングI/O
    std::cout << "\n4. ノンブロッキングI/Oの例" << std::endl;
    
    // 標準入力をノンブロッキングモードに設定
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    std::cout << "ノンブロッキングモードで標準入力を読み取り（すぐに戻ります）:" << std::endl;
    
    char nonblock_buffer[256];
    bytes_read = read(STDIN_FILENO, nonblock_buffer, sizeof(nonblock_buffer) - 1);
    
    if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "データがありません（EAGAIN/EWOULDBLOCK）" << std::endl;
        } else {
            std::cerr << "読み取りエラー: " << strerror(errno) << std::endl;
        }
    } else {
        nonblock_buffer[bytes_read] = '\0';
        std::cout << "読み取ったバイト数: " << bytes_read << std::endl;
        std::cout << "読み取った内容: " << nonblock_buffer << std::endl;
    }
    
    // 標準入力を元に戻す
    fcntl(STDIN_FILENO, F_SETFL, flags);
    
    std::cout << "\nプログラムを終了します。" << std::endl;
    return 0;
}