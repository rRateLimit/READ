#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <iomanip>

void print_usage(const char* program_name) {
    std::cout << "使用方法: " << program_name << " [オプション] <ファイルパス>" << std::endl;
    std::cout << "オプション:" << std::endl;
    std::cout << "  -b SIZE    バッファサイズを指定（デフォルト: 4096バイト）" << std::endl;
    std::cout << "  -o OFFSET  読み取り開始位置を指定（デフォルト: 0）" << std::endl;
    std::cout << "  -l LENGTH  読み取る最大バイト数を指定（デフォルト: ファイル全体）" << std::endl;
    std::cout << "  -x         16進数ダンプ表示" << std::endl;
    std::cout << "  -s         統計情報を表示" << std::endl;
    std::cout << "  -h         このヘルプを表示" << std::endl;
}

void print_hex_dump(const char* data, size_t size, off_t offset) {
    const int bytes_per_line = 16;
    
    for (size_t i = 0; i < size; i += bytes_per_line) {
        // オフセット表示
        std::cout << std::setfill('0') << std::setw(8) << std::hex << (offset + i) << "  ";
        
        // 16進数表示
        for (int j = 0; j < bytes_per_line; j++) {
            if (i + j < size) {
                std::cout << std::setfill('0') << std::setw(2) << std::hex 
                          << static_cast<unsigned int>(static_cast<unsigned char>(data[i + j])) << " ";
            } else {
                std::cout << "   ";
            }
            if (j == 7) std::cout << " ";
        }
        
        std::cout << " |";
        
        // ASCII表示
        for (int j = 0; j < bytes_per_line && i + j < size; j++) {
            char c = data[i + j];
            if (c >= 32 && c <= 126) {
                std::cout << c;
            } else {
                std::cout << ".";
            }
        }
        
        std::cout << "|" << std::endl;
    }
    std::cout << std::dec;
}

int main(int argc, char* argv[]) {
    // デフォルト値
    size_t buffer_size = 4096;
    off_t offset = 0;
    ssize_t max_length = -1;
    bool hex_dump = false;
    bool show_stats = false;
    
    // オプション解析
    int opt;
    while ((opt = getopt(argc, argv, "b:o:l:xsh")) != -1) {
        switch (opt) {
            case 'b':
                buffer_size = std::stoul(optarg);
                if (buffer_size == 0) {
                    std::cerr << "エラー: バッファサイズは0より大きくなければなりません" << std::endl;
                    return 1;
                }
                break;
            case 'o':
                offset = std::stol(optarg);
                if (offset < 0) {
                    std::cerr << "エラー: オフセットは0以上でなければなりません" << std::endl;
                    return 1;
                }
                break;
            case 'l':
                max_length = std::stol(optarg);
                if (max_length <= 0) {
                    std::cerr << "エラー: 長さは0より大きくなければなりません" << std::endl;
                    return 1;
                }
                break;
            case 'x':
                hex_dump = true;
                break;
            case 's':
                show_stats = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // ファイルパスの確認
    if (optind >= argc) {
        std::cerr << "エラー: ファイルパスが指定されていません" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    const char* filename = argv[optind];
    
    // ファイルを開く
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        std::cerr << "エラー: ファイルを開けません: " << filename << std::endl;
        std::cerr << "理由: " << strerror(errno) << std::endl;
        return 1;
    }
    
    // 指定されたオフセットにシーク
    if (offset > 0) {
        if (lseek(fd, offset, SEEK_SET) == -1) {
            std::cerr << "エラー: シークに失敗しました" << std::endl;
            std::cerr << "理由: " << strerror(errno) << std::endl;
            close(fd);
            return 1;
        }
    }
    
    // バッファの準備
    std::vector<char> buffer(buffer_size);
    ssize_t total_bytes = 0;
    ssize_t bytes_read;
    int read_count = 0;
    
    if (show_stats) {
        std::cout << "ファイル: " << filename << std::endl;
        std::cout << "バッファサイズ: " << buffer_size << " バイト" << std::endl;
        std::cout << "開始オフセット: " << offset << " バイト" << std::endl;
        if (max_length > 0) {
            std::cout << "最大読み取りバイト数: " << max_length << " バイト" << std::endl;
        }
        std::cout << std::endl;
    }
    
    // ファイルを読み取る
    while ((bytes_read = read(fd, buffer.data(), buffer_size)) > 0) {
        read_count++;
        
        // 最大長の確認
        if (max_length > 0 && total_bytes + bytes_read > max_length) {
            bytes_read = max_length - total_bytes;
        }
        
        if (hex_dump) {
            print_hex_dump(buffer.data(), bytes_read, offset + total_bytes);
        } else {
            // 標準出力に書き込む
            ssize_t written = 0;
            while (written < bytes_read) {
                ssize_t result = write(STDOUT_FILENO, buffer.data() + written, bytes_read - written);
                if (result == -1) {
                    std::cerr << "エラー: 出力に失敗しました" << std::endl;
                    std::cerr << "理由: " << strerror(errno) << std::endl;
                    close(fd);
                    return 1;
                }
                written += result;
            }
        }
        
        total_bytes += bytes_read;
        
        // 最大長に達したら終了
        if (max_length > 0 && total_bytes >= max_length) {
            break;
        }
    }
    
    if (bytes_read == -1) {
        std::cerr << "エラー: 読み取りに失敗しました" << std::endl;
        std::cerr << "理由: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }
    
    close(fd);
    
    // 統計情報の表示
    if (show_stats) {
        if (!hex_dump) std::cout << std::endl;
        std::cout << "\n=== 統計情報 ===" << std::endl;
        std::cout << "read()呼び出し回数: " << read_count << std::endl;
        std::cout << "読み取った総バイト数: " << total_bytes << std::endl;
        if (read_count > 0) {
            std::cout << "平均読み取りバイト数: " << (total_bytes / read_count) << std::endl;
        }
    }
    
    return 0;
}