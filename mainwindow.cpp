#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <QDebug>
#include <QFileDialog>

using std::endl;
using std::cout;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    is_single = true;
    ui->radioButton->setChecked(true);
    ui->comboBox_2->setEnabled(false);
    ui->comboBox_3->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::read_from_file(const std::string &file_name)
{
    access.clear();
    std::fstream f(file_name);
    if (!f.is_open())
    {
        LOG_ERROR("file open error...");
        return false;
    }
    std::string tmp;
    while (getline(f, tmp))
    {
        std::string mode;
        std::string address;
        std::stringstream ss(tmp);
        ss >> mode >> address;
        if (mode.empty() || address.empty() || std::stoi(mode) > 4 || std::stoi(mode) < 0 || std::stoi(address) < 0)
            continue;
        access.push_back(std::make_pair(mode, address));
    }
    return true;
}

int64_t MainWindow::to_real_address(int64_t line_size, int64_t addr)
{
    if (line_size <= 0)
        return -1;
    return addr / line_size * line_size;
}

bool MainWindow::check_first()
{
    if (!ui->lineEdit->isEnabled())
        return false;
    else
    {
        if (!read_from_file(file_name))
        {
            LOG_ERROR("file format error...");
            ui->lineEdit->setText("");
            is_processing = false;
            return true;
        }
        ui->radioButton->setEnabled(false);
        ui->radioButton_2->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
        ui->comboBox_3->setEnabled(false);
        ui->comboBox_4->setEnabled(false);
        ui->comboBox_5->setEnabled(false);
        ui->comboBox_6->setEnabled(false);
        ui->comboBox_7->setEnabled(false);
        ui->comboBox_8->setEnabled(false);
        ui->comboBox_9->setEnabled(false);
        ui->pushButton->setEnabled(false);
        ui->lineEdit->setEnabled(false);

        is_processing = true;
        access_cnt = 0;
        total_access_times = 0;
        instr_fetch = 0;
        data_read = 0;
        data_write = 0;
        instr_miss = 0;
        data_read_miss = 0;
        data_write_miss = 0;

        line_size = LINE_SIZE[ui->comboBox_4->currentIndex()];
        associate = ASSOCIATE[ui->comboBox_5->currentIndex()];
        replace_strategy = ui->comboBox_6->currentIndex();
        prefetch_strategy = ui->comboBox_7->currentIndex();
        write_strategy = ui->comboBox_8->currentIndex();
        write_miss_strategy = ui->comboBox_9->currentIndex();

        if (is_single)
        {
            data_size = SINGLE_DATA_CACHE[ui->comboBox->currentIndex()];
            data_group_size = data_size / line_size / associate;
            data_cache = std::vector<std::vector<int64_t>>(data_group_size, std::vector<int64_t>(associate, -1));
        }
        else
        {
            data_size = DATA_CACHE[ui->comboBox_3->currentIndex()];
            instr_size = INSTR_CACHE[ui->comboBox_2->currentIndex()];
            data_group_size = data_size / line_size / associate;
            instr_group_size = instr_size / line_size / associate;
            data_cache = std::vector<std::vector<int64_t>>(data_group_size, std::vector<int64_t>(associate, -1));
            instr_cache = std::vector<std::vector<int64_t>>(instr_group_size, std::vector<int64_t>(associate, -1));
        }

        ui->lineEdit_2->setText("0");
        ui->lineEdit_3->setText("0");
        ui->lineEdit_4->setText("0.00%");
        ui->lineEdit_5->setText("0");
        ui->lineEdit_6->setText("0");
        ui->lineEdit_7->setText("0.00%");
        ui->lineEdit_8->setText("0");
        ui->lineEdit_9->setText("0");
        ui->lineEdit_10->setText("0.00%");
        ui->lineEdit_11->setText("0");
        ui->lineEdit_12->setText("0");
        ui->lineEdit_13->setText("0.00%");
    }
    return true;
}

bool MainWindow::process()
{
    if (is_single)
    {
        return single_cache();
    }
    else
    {
        return double_cache();
    }
}

bool MainWindow::single_cache()
{
    bool miss;
    switch (replace_strategy)
    {
        case REPLACE_LRU:
            miss = LRU(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
        case REPLACE_FIFO:
            miss = FIFO(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
        case REPLACE_RANDOM:
            miss = RANDOM(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
    }
    return miss;
}

bool MainWindow::double_cache()
{
    bool miss;
    switch (replace_strategy)
    {
        case REPLACE_LRU:
            if (std::stoi(access[access_cnt].first) == INSTRUCTION_FETCH)
                miss = LRU(instr_cache, std::stoi(access[access_cnt].second, 0, 16));
            else
                miss = LRU(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
        case REPLACE_FIFO:
            if (std::stoi(access[access_cnt].first) == INSTRUCTION_FETCH)
                miss = FIFO(instr_cache, std::stoi(access[access_cnt].second, 0, 16));
            else
                miss = FIFO(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
        case REPLACE_RANDOM:
            if (std::stoi(access[access_cnt].first) == INSTRUCTION_FETCH)
                miss = RANDOM(instr_cache, std::stoi(access[access_cnt].second, 0, 16));
            else
                miss = RANDOM(data_cache, std::stoi(access[access_cnt].second, 0, 16));
            break;
    }
    return miss;
}

bool MainWindow::LRU(std::vector<std::vector<int64_t>> &cache, int64_t address)
{
    int64_t tmp_addr = to_real_address(line_size, address);
    int64_t group_num;
    if (&cache == &data_cache)
        group_num = tmp_addr / line_size % data_group_size;
    else if (&cache == &instr_cache)
        group_num = tmp_addr / line_size % instr_group_size;
    // cout << "tmp_addr: " << tmp_addr << endl;
    // cout << "group_num: " << group_num << endl;
    int64_t i, size = cache[group_num].size();
    for (i = 0; i < size; ++i)
    {
        if (tmp_addr == cache[group_num][i])
        {
            int64_t j;
            for (j = i + 1; j < size; ++j)
            {
                if (cache[group_num][j] != -1)
                    cache[group_num][j-1] = cache[group_num][j];
            }
            cache[group_num][j-1] = tmp_addr;
            return false;
        }
        else if (cache[group_num][i] == -1)
        {
            if (access[access_cnt].first == "1" && write_miss_strategy == NOT_DISTRIBUTE_ON_WRITE)
                return true;
            cache[group_num][i] = tmp_addr;
            return true;
        }
    }
    if (access[access_cnt].first == "1" && write_miss_strategy == NOT_DISTRIBUTE_ON_WRITE)
        return true;
    if (i == size)
    {
        for (int64_t k = 0; k < size - 1; ++k)
            cache[group_num][k] = cache[group_num][k+1];
        cache[group_num][size-1] = tmp_addr;
    }
    return true;
}

bool MainWindow::FIFO(std::vector<std::vector<int64_t> > &cache, int64_t address)
{
    int64_t tmp_addr = to_real_address(line_size, address);
    int64_t group_num;
    if (&cache == &data_cache)
        group_num = tmp_addr / line_size % data_group_size;
    else if (&cache == &instr_cache)
        group_num = tmp_addr / line_size % instr_group_size;
    // cout << "tmp_addr: " << tmp_addr << endl;
    // cout << "group_num: " << group_num << endl;
    int64_t i, size = cache[group_num].size();
    for (i = 0; i < size; ++i)
    {
        if (tmp_addr == cache[group_num][i])
        {
            return false;
        }
        else if(cache[group_num][i] == -1)
        {
            if (access[access_cnt].first == "1" && write_miss_strategy == NOT_DISTRIBUTE_ON_WRITE)
                return true;
            cache[group_num][i] = tmp_addr;
            return true;
        }
    }
    if (access[access_cnt].first == "1" && write_miss_strategy == NOT_DISTRIBUTE_ON_WRITE)
        return true;
    if (i == size)
    {
        for (int64_t k = 0; k < size - 1; ++k)
            cache[group_num][k] = cache[group_num][k+1];
        cache[group_num][size-1] = tmp_addr;
    }
    return true;
}

bool MainWindow::RANDOM(std::vector<std::vector<int64_t> > &cache, int64_t address)
{
    int64_t tmp_addr = to_real_address(line_size, address);
    int64_t group_num;
    if (&cache == &data_cache)
        group_num = tmp_addr / line_size % data_group_size;
    else if (&cache == &instr_cache)
        group_num = tmp_addr / line_size % instr_group_size;
    // cout << "tmp_addr: " << tmp_addr << endl;
    // cout << "group_num: " << group_num << endl;
    int64_t pos = u(e) % associate, size = cache[group_num].size();
    for (int64_t i = 0; i < size; ++i)
    {
        if (tmp_addr == cache[group_num][i])
            return false;
    }
    if (access[access_cnt].first == "1" && write_miss_strategy == NOT_DISTRIBUTE_ON_WRITE)
        return true;
    cache[group_num][pos] = tmp_addr;
    return true;
}

void MainWindow::show_current_access()
{
    QString tmp((!access_cnt) ? access[0].first.c_str() : access[access_cnt-1].first.c_str());
    tmp.append(' ');
    tmp.append(QString((!access_cnt) ? access[0].second.c_str() : access[access_cnt-1].second.c_str()));
    ui->lineEdit_14->setText(tmp);
    int64_t tmp_all = data_read + data_write + instr_fetch;
    int64_t tmp_miss = data_read_miss + data_write_miss + instr_miss;
    ui->lineEdit_2->setText(QString::number(tmp_all));
    ui->lineEdit_3->setText(QString::number(tmp_miss));
    ui->lineEdit_4->setText(QString::number((!tmp_all) ? 0 : (double)tmp_miss / tmp_all * 100, 'f', 2).append('%'));
    ui->lineEdit_8->setText(QString::number(data_read));
    ui->lineEdit_9->setText(QString::number(data_read_miss));
    ui->lineEdit_10->setText(QString::number((!data_read) ? 0 : (double)data_read_miss / data_read * 100, 'f', 2).append('%'));
    ui->lineEdit_11->setText(QString::number(data_write));
    ui->lineEdit_12->setText(QString::number(data_write_miss));
    ui->lineEdit_13->setText(QString::number((!data_write) ? 0 : (double)data_write_miss / data_write * 100, 'f', 2).append('%'));
    ui->lineEdit_5->setText(QString::number(instr_fetch));
    ui->lineEdit_6->setText(QString::number(instr_miss));
    ui->lineEdit_7->setText(QString::number((!instr_fetch) ? 0 : (double)instr_miss / instr_fetch * 100, 'f', 2).append('%'));
}

void MainWindow::on_radioButton_clicked()
{
    is_single = true;
    ui->comboBox->setEnabled(true);
    ui->comboBox_2->setEnabled(false);
    ui->comboBox_3->setEnabled(false);
}

void MainWindow::on_radioButton_2_clicked()
{
    is_single = false;
    ui->comboBox->setEnabled(false);
    ui->comboBox_2->setEnabled(true);
    ui->comboBox_3->setEnabled(true);
}

void MainWindow::on_action_triggered()
{
    is_single = true;
    instr_size = 0;
    ui->radioButton->setChecked(true);
    ui->radioButton->setEnabled(true);
    ui->radioButton_2->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->comboBox_2->setEnabled(false);
    ui->comboBox_3->setEnabled(false);
    ui->comboBox_4->setEnabled(true);
    ui->comboBox_5->setEnabled(true);
    ui->comboBox_6->setEnabled(true);
    ui->comboBox_7->setEnabled(true);
    ui->comboBox_8->setEnabled(true);
    ui->comboBox_9->setEnabled(true);
    ui->pushButton->setEnabled(true);
    ui->lineEdit->setEnabled(true);
    is_processing = false;
    ui->lineEdit->setText("");
}

void MainWindow::on_pushButton_clicked()
{
    QString tmp = QFileDialog::getOpenFileName(this, "地址流文件", QDir::homePath(), "Din file (*.din);;Any file (*.*)");
    ui->lineEdit->setText(tmp);
    file_name = tmp.toStdString();
    // cout << file_name << endl;
}

void MainWindow::on_pushButton_2_clicked()
{
    bool miss;
    if (check_first() && is_processing)
    {
        // cout << "type: " << access[access_cnt].first << " address: " << std::stoi(access[access_cnt].second, 0, 16) << endl;
        miss = process();
        switch(std::stoi(access[access_cnt].first))
        {
            case READ_DATA:
                if (miss)
                    ++data_read_miss;
                ++data_read;
                break;
            case WRITE_DATA:
                if (miss)
                    ++data_write_miss;
                ++data_write;
                break;
            case INSTRUCTION_FETCH:
                if (miss)
                    ++instr_miss;
                ++instr_fetch;
                break;
        }
        show_current_access();
        ++access_cnt;
    }
    else if (is_processing)
    {
        // cout << "type: " << access[access_cnt].first << " address: " << std::stoi(access[access_cnt].second, 0, 16) << endl;
        // cout << "data_group_size: " << data_group_size << endl;
        miss = process();
        switch(std::stoi(access[access_cnt].first))
        {
            case READ_DATA:
                if (miss)
                    ++data_read_miss;
                ++data_read;
                break;
            case WRITE_DATA:
                if (miss)
                    ++data_write_miss;
                ++data_write;
                break;
            case INSTRUCTION_FETCH:
                if (miss)
                    ++instr_miss;
                ++instr_fetch;
                break;
        }
        ++access_cnt;
        show_current_access();
    }
    if (miss && prefetch_strategy == YES_WHEN_MISS)
    {
        --access_cnt;
        std::stringstream ss;
        ss << std::hex << to_real_address(line_size, std::stoi(access[access_cnt].second, 0, 16)) + line_size;
        access[access_cnt].second = std::string(ss.str());
        miss = process();
        switch(std::stoi(access[access_cnt].first))
        {
            case READ_DATA:
                if (miss)
                    ++data_read_miss;
                ++data_read;
                break;
            case WRITE_DATA:
                if (miss)
                    ++data_write_miss;
                ++data_write;
                break;
            case INSTRUCTION_FETCH:
                if (miss)
                    ++instr_miss;
                ++instr_fetch;
                break;
        }
        ++access_cnt;
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    bool miss;
    check_first();
    if (is_processing)
    {
        for (; access_cnt < access.size(); ++access_cnt)
        {
            miss = process();
            switch(std::stoi(access[access_cnt].first))
            {
                case READ_DATA:
                    if (miss)
                        ++data_read_miss;
                    ++data_read;
                    break;
                case WRITE_DATA:
                    if (miss)
                        ++data_write_miss;
                    ++data_write;
                    break;
                case INSTRUCTION_FETCH:
                    if (miss)
                        ++instr_miss;
                    ++instr_fetch;
                    break;
            }
            if (miss && prefetch_strategy == YES_WHEN_MISS)
            {
                std::stringstream ss;
                ss << std::hex << to_real_address(line_size, std::stoi(access[access_cnt].second, 0, 16)) + line_size;
                access[access_cnt].second = std::string(ss.str());
                miss = process();
                switch(std::stoi(access[access_cnt].first))
                {
                    case READ_DATA:
                        if (miss)
                            ++data_read_miss;
                        ++data_read;
                        break;
                    case WRITE_DATA:
                        if (miss)
                            ++data_write_miss;
                        ++data_write;
                        break;
                    case INSTRUCTION_FETCH:
                        if (miss)
                            ++instr_miss;
                        ++instr_fetch;
                        break;
                }
            }
        }
        show_current_access();
    }
}
