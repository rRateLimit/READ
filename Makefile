CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
TARGETS = read_example file_reader

.PHONY: all clean run-example run-reader

all: $(TARGETS)

read_example: read_example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

file_reader: file_reader.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

run-example: read_example
	./read_example

run-reader: file_reader sample.txt
	@echo "=== 通常の読み取り ==="
	./file_reader sample.txt
	@echo "\n=== 統計情報付き ==="
	./file_reader -s sample.txt
	@echo "\n=== 16進数ダンプ ==="
	./file_reader -x -l 100 sample.txt
	@echo "\n=== オフセット指定 ==="
	./file_reader -o 10 -l 50 -s sample.txt

sample.txt:
	@echo "サンプルテキストファイルを作成しています..."
	@echo "これはreadシステムコールのテスト用ファイルです。" > sample.txt
	@echo "2行目：日本語も含まれています。" >> sample.txt
	@echo "3行目：複数行のテキストファイルです。" >> sample.txt
	@echo "4行目：1234567890" >> sample.txt
	@echo "5行目：ABCDEFGHIJKLMNOPQRSTUVWXYZ" >> sample.txt
	@echo "6行目：abcdefghijklmnopqrstuvwxyz" >> sample.txt
	@echo "7行目：!@#$$%^&*()_+-=[]{}|;':\",./<>?" >> sample.txt
	@for i in {1..10}; do echo "長い行 $$i: これは長いテキストラインです。読み取りバッファのテストのために繰り返しテキストを含んでいます。" >> sample.txt; done

clean:
	rm -f $(TARGETS) test_read.txt sample.txt

help:
	@echo "使用可能なターゲット:"
	@echo "  make all         - すべてのプログラムをビルド"
	@echo "  make clean       - 生成されたファイルを削除"
	@echo "  make run-example - read_exampleを実行"
	@echo "  make run-reader  - file_readerのデモを実行"
	@echo "  make sample.txt  - テスト用サンプルファイルを作成"
	@echo "  make help        - このヘルプを表示"