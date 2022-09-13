#pragma once

#define CHECK_ERR(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return;}
#define CHECK_RET(err, format, ...) if (err != 0) {printf("Error: %d " format " [%s:%s:%d]\n", err, ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__); return err;}

//! @brief timer, scaling by ms
//! @return get current time
double timer();
