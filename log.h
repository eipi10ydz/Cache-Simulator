#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <cstdlib>

// DESCRIPTION should be char*

#define LOG_ERROR(DESCRIPTION, ...) fprintf(stderr, "[ERROR] (%s:%d error message: " DESCRIPTION ")\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(DESCRIPTION, ...) fprintf(stderr, "[WARN] (%s:%d warn message: " DESCRIPTION ")\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(DESCRIPTION, ...) fprintf(stderr, "[INFO] (%s:%d info message: " DESCRIPTION ")\n", __FILE__, __LINE__, ##__VA_ARGS__)

/*
 * swap more than one statement with "do { } while(0)"
 * see below for more details
 * https://github.com/miloyip/json-tutorial/blob/master/tutorial01/tutorial01.md#%E5%AE%8F%E7%9A%84%E7%BC%96%E5%86%99%E6%8A%80%E5%B7%A7
 */

#define CHECK_ERROR(CONDITION, DESCRIPTION) do { if (!(CONDITION)) { LOG_ERROR(DESCRIPTION); } } while(0)

#define CHECK_WARN(CONDITION, DESCRIPTION) do { if (!(CONDITION)) { LOG_WARN(DESCRIPTION); } } while(0)

#define CHECK_INFO(CONDITION, DESCRIPTION) do { if (!(CONDITION)) { LOG_INFO(DESCRIPTION); } } while(0)

#define CHECK_ERROR_EXIT(CONDITION, DESCRIPTION) do { if (!(CONDITION)) { LOG_ERROR(DESCRIPTION); exit(1); }  } while(0)

#endif // LOG_H
