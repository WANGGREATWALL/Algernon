#pragma once
#include <string>
#include <vector>

#define CHECK_ERR(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return;}
#define CHECK_RET(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return err;}

//! @brief timer, scaling by ms
//! @return get current time
double timer();

//! @brief Read data from a file
//! @param filePath: 绝对或相对路径, 包含完整的文件名
//! @param fileSize: 读取的字节大小, 默认为 0 时读取文件全部数据
//! @return 返回一个 string 存放数据, 若 string 的 size = 0 则读取失败
std::string read_data_from_file(std::string filePath, size_t fileSize = 0);

//! @brief Write data to a file
//! @param filePath: 绝对或相对路径, 包含完整的文件名
//! @param pData: 待写入的数据指针
//! @param fileSize: 待写入的字节大小
//! @return return 0 if success, elsewise -1
int write_data_to_file(std::string filePath, const char* pData, size_t fileSize);