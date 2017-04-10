#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <random>
#include "log.h"

const std::vector<int8_t> ASSOCIATE = { 1, 2, 4, 8, 16, 32 };

const std::vector<int32_t> SINGLE_DATA_CACHE = { 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576 };
const std::vector<int32_t> DATA_CACHE = { 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288 };
const std::vector<int32_t> INSTR_CACHE = { 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288 };
const std::vector<int32_t> LINE_SIZE = { 16, 32, 64, 128, 256 };

// const std::vector<int8_t> REPLACE_STRATEGY = { 0, 1, 2 };
constexpr int8_t REPLACE_LRU = 0, REPLACE_FIFO = 1, REPLACE_RANDOM = 2;

// const std::vector<int8_t> PREFETCH_STRATEGY = { 0, 1 };
constexpr int8_t NO = 0, YES_WHEN_MISS = 1;

// const std::vector<int8_t> WRITE_STRATEGY = { 0, 1 };
constexpr int8_t WRITE_BACK = 0, WRITE_THROUGH = 1;

// const std::vector<int8_t> WRITE_MISS_STRATEGY = { 0, 1 };
constexpr int8_t DISTRIBUTE_ON_WRITE = 0, NOT_DISTRIBUTE_ON_WRITE = 1;

// const std::vector<int8_t> LABEL = { 0, 1, 2, 3, 4 };
constexpr int8_t READ_DATA = 0, WRITE_DATA = 1, INSTRUCTION_FETCH = 2, UNKNOWN = 3, CACHE_FLUSH = 4;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_action_triggered();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

private:
    bool read_from_file(const std::string &file_name);
    bool check_first();
    bool process();
    bool single_cache();
    bool double_cache();

    /// true stands for cache miss
    bool LRU(std::vector<std::vector<int64_t>> &cache, int64_t address);
    bool FIFO(std::vector<std::vector<int64_t>> &cache, int64_t address);
    bool RANDOM(std::vector<std::vector<int64_t>> &cache, int64_t address);
    void show_current_access();
    int64_t to_real_address(int64_t line_size, int64_t addr);
    std::vector<std::pair<std::string, std::string>> access;
    std::vector<std::vector<int64_t>> data_cache;
    std::vector<std::vector<int64_t>> instr_cache;
    std::vector<std::vector<int64_t>> data_recent;
    std::vector<std::vector<int64_t>> instr_recent;

    static std::uniform_int_distribution<uint32_t> u;
    //
    static std::default_random_engine e;

    Ui::MainWindow *ui;

    int64_t access_cnt;

    std::string file_name;
    int64_t total_access_times;
    bool is_single;
    bool is_processing;         // running

    int32_t instr_size;
    int32_t data_size;
    int32_t line_size;
    int32_t data_group_size;
    int32_t instr_group_size;
    int32_t associate;
    int32_t instr_fetch;
    int32_t data_read;
    int32_t data_write;
    int32_t instr_miss;
    int32_t data_read_miss;
    int32_t data_write_miss;

    int8_t replace_strategy;    // 0 1 2
    int8_t prefetch_strategy;   // 0 1
    int8_t write_strategy;      // 0 1
    int8_t write_miss_strategy; // 0 1
};

#endif // MAINWINDOW_H
