#pragma once
#include <string>
#include <vector>

#define CHECK_ERR(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return;}
#define CHECK_RET(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return err;}

//! @brief timer, scaling by ms
//! @return get current time
double timer();

//! @brief Read data from a file
//! @param filePath: ���Ի����·��, �����������ļ���
//! @param fileSize: ��ȡ���ֽڴ�С, Ĭ��Ϊ 0 ʱ��ȡ�ļ�ȫ������
//! @return ����һ�� string �������, �� string �� size = 0 ���ȡʧ��
std::string read_data_from_file(std::string filePath, size_t fileSize = 0);

//! @brief Write data to a file
//! @param filePath: ���Ի����·��, �����������ļ���
//! @param pData: ��д�������ָ��
//! @param fileSize: ��д����ֽڴ�С
//! @return return 0 if success, elsewise -1
int write_data_to_file(std::string filePath, const char* pData, size_t fileSize);