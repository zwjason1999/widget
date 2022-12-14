#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_KEYWORD_SIZE 50         // 关键字最大长度
#define MAX_FILE_PATH_SIZE 256      // 文件路径最大长度
#define MAX_ID_SIZE 20              // 一个ID的最大长度
#define MAX_IDS_SIZE 5000           // ID数组的最大长度
#define MAX_COMMAND_SIZE 1024       // 一条命令的最大长度


/**
 * @brief 在ID数组指定的文件夹下的文件中查找关键字
 * @param ssh_input_file_path ssh指令所在目录
 * @param ssh_ids ID数组
 * @param keyword 关键字
 * @param ssh_output_file_path 输出目录
 * @return void 
 */
void find_keyword_by_ids(
    const char* ssh_input_file_path, 
    char *ssh_ids[MAX_IDS_SIZE], 
    const char* keyword, 
    const char* ssh_output_file_path);

/**
 * @brief 在指定目录下所有的文件中查找关键字
 * @param ssh_input_file_path ssh指令所在目录
 * @param keyword 关键字
 * @param ssh_output_file_path 输出目录
 * @return void 
 */
void find_keyword_default(const char* ssh_input_file_path, 
    const char* keyword, 
    const char* ssh_output_file_path);

/**
 * @brief 读取配置文件
 * @param config_file_path 配置文件路径
 * @param keyword 关键字
 * @param ssh_input_file_path ssh指令所在目录
 * @param ssh_output_file_path 输出目录 
 * @param ssh_ids ID数组
 * is_ssh_ids_empty ID数组是否为空
 * @return void
 */
void read_config(
    const char* config_file_path, 
    char* keyword, 
    char* ssh_input_file_path, 
    char* ssh_output_file_path, 
    char *ssh_ids[MAX_IDS_SIZE],
    int* is_ssh_ids_empty);

/**
 * @brief 判断文件或目录是否存在
 * @param path 文件或目录的路径
 * @return 存在为0，否则为-1
 */
int is_exist(const char* path);

/**
 * @brief 判断文路径是否为目录
 * @param path 路径
 * @return 目录为0，否则为-1
 */
int is_directory(const char* path);

int main() {
    clock_t start = clock();    // 计时

    const char* config_file_path = "./config.txt";                     // 配置文件路径 
    /*与配置文件一致的字段*/
    char *keyword = (char*)malloc(MAX_KEYWORD_SIZE);
    char *ssh_input_file_path = (char*)malloc(MAX_FILE_PATH_SIZE);
    char *ssh_output_file_path = (char*)malloc(MAX_FILE_PATH_SIZE);
    char *ssh_ids[MAX_IDS_SIZE]; 
    size_t i;       
    for (i = 0; i < MAX_IDS_SIZE; ++i) {
        ssh_ids[i] = NULL;
    }
    // 读配置文件
    int is_ssh_ids_empty = 0;
    read_config(config_file_path, keyword, ssh_input_file_path, ssh_output_file_path, ssh_ids, &is_ssh_ids_empty);     
    if (is_ssh_ids_empty == 0) {    // 指定了ID数组且不为空
        find_keyword_by_ids(ssh_input_file_path, ssh_ids, keyword, ssh_output_file_path);
    } else {
        find_keyword_default(ssh_input_file_path, keyword, ssh_output_file_path);
    }
    // printf("keyword=%s\n", keyword);
    // printf("ssh_input_file_path=%s\n", ssh_input_file_path);
    // printf("ssh_output_file_path=%s\n", ssh_output_file_path);

    

    clock_t end = clock();
    printf("Total time: %fs\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Tips: All matching IDs are listed in result.txt\n");
    printf("--------------------\n");

    // 释放内存
    free(keyword);
    free(ssh_input_file_path);
    free(ssh_output_file_path);
    for (i = 0; i < MAX_IDS_SIZE; ++i) {
        free(ssh_ids[i]);
    }

    return 0;
}


int is_exist(const char* path) {
    struct stat buf;   
    if (stat(path, &buf) == 0) {
        return 0;
    }
    return -1;
}


int is_directory(const char* path) {
    struct stat buf;
    if (stat(path, &buf) == 0 && S_ISDIR(buf.st_mode)) {
        return 0;
    }
    return -1;
}


void read_config(
    const char* config_file_path,
    char* keyword,
    char* ssh_input_file_path,
    char* ssh_output_file_path,
    char *ssh_ids[MAX_IDS_SIZE],
    int* is_ssh_ids_empty)
{
    FILE* cfh = fopen(config_file_path, "r");   // Config File Handle
    if (cfh == NULL) {
        printf("fail to open config file: %s.\n", config_file_path);
        exit(-1);
    }
    // 按行读
    char* line = (char*)malloc(MAX_IDS_SIZE * MAX_ID_SIZE + MAX_FILE_PATH_SIZE);
    while (fgets(line, MAX_IDS_SIZE * MAX_ID_SIZE + MAX_FILE_PATH_SIZE - 1, cfh) != NULL) {
        // 忽略注释
        if (line[0] == '#') {
            continue;
        }
        // 按等号分割键值对
        char key[MAX_COMMAND_SIZE] = {0};
        char* value = (char*)malloc(MAX_IDS_SIZE * MAX_ID_SIZE);    // value可能要存ID数组，可能比较大
        size_t i = 0;
        while (i <  MAX_COMMAND_SIZE && line[i] != '=') {
            key[i] = line[i];
            ++i;
        }
        key[i++] = '\0';    // 加入终止符并跳过等号

        // 提取value
        size_t j = 0;
        while (line[i] != '\0') {
            if (line[i] != '\n') {      // 忽略换行符
                value[j++] = line[i];
            }
            ++i;
        }
        value[j] = '\0';
        // printf("j=%lu\n", j);
        // printf("key=%s\n", key);
        // printf("value=%s\n", value);
        if (strcmp(key, "keyword") == 0) {
            strcpy(keyword, value);
        } else if (strcmp(key, "ssh_input_file_path") == 0) {
            strcpy(ssh_input_file_path, value);
        } else if (strcmp(key, "ssh_output_file_path") == 0) {
            strcpy(ssh_output_file_path, value);
        } else if (strcmp(key, "ssh_ids") == 0) {
            size_t idx = 0;
            size_t begin = 1;
            if (j < 3) {    
                *is_ssh_ids_empty = 1;  // 空的ssh_ids数组
                continue; 
            }     
            while (begin < j) {
                size_t k = 0;
                char id[MAX_ID_SIZE] = {0};
                size_t end = begin;
                while (end < j && value[end] != ',' && value[end] != ']') { 
                    id[k++] = value[end++];
                }
                if (end < j) {
                    ssh_ids[idx] = (char*)malloc(MAX_ID_SIZE);
                    strcpy(ssh_ids[idx++], id);     // 复制并递增下标
                    begin = end + 1;                // 跳过逗号
                }
            }
        } else {
            printf("wrong key in config file.\n");
        }
        free(value);    // 释放空间
    }
    // 关闭文件
    fclose(cfh);
    free(line);
    // for (size_t i = 0; i < MAX_IDS_SIZE; ++i) {
    //     if (ssh_ids[i] != NULL) {
    //         printf("id=%s\n", ssh_ids[i]);
    //     }
    // }

    // 判断ssh_input_file_path是否存在
    if (is_exist(ssh_input_file_path) != 0) {
        printf("%s is not exist.\n", ssh_input_file_path);
        exit(-1);
    }
    // 判断ssh_input_file_path是否为目录
    if (is_directory(ssh_input_file_path) != 0) {
        printf("%s is not a directory.\n", ssh_input_file_path);
        exit(-1);
    }
    // 判断ssh_input_file_path是否存在
    if (is_exist(ssh_output_file_path) != 0) {
        printf("%s is not exist.\n", ssh_output_file_path);
        exit(-1);
    }
    // 判断ssh_output_file_path是否为目录
    if (is_directory(ssh_output_file_path) != 0) {
        printf("%s is not a directory.\n", ssh_output_file_path);
        exit(-1);
    }

}


void find_keyword_by_ids(
    const char* ssh_input_file_path, 
    char* ssh_ids[MAX_IDS_SIZE],
    const char* keyword,
    const char* ssh_output_file_path) 
{
    // 拼接输出文件路径
    char outfile_path[MAX_FILE_PATH_SIZE] = {0};
    strcat(outfile_path, ssh_output_file_path);
    strcat(outfile_path, "/result.txt");
    // 打开
    FILE* outfile = fopen(outfile_path, "w");
    if (outfile == NULL) {
        printf("fail to open file: %s\n", outfile_path);
    }

    char original_path[MAX_FILE_PATH_SIZE] = {0};
    strcat(original_path, ssh_input_file_path);
    strcat(original_path, "/");
    // 遍历ssh_ids数组
    size_t i;
    for (i = 0; i < MAX_IDS_SIZE; ++i) {
        // 空ID表示结束
        if (ssh_ids[i] == NULL) {
            break;
        }
        // 带ID的路径
        char path_with_id[MAX_FILE_PATH_SIZE] = {0};
        strcat(path_with_id, original_path);
        strcat(path_with_id, ssh_ids[i]);
        // 打开
        DIR* infile_dir;
        if(!(infile_dir = opendir(path_with_id))) {
            printf("fail to open diectory: %s\n", path_with_id);
            exit(-1);
        }
        // 遍历所有文件
        // 目前只有一份文件
        struct dirent* ptr;
        while ((ptr = readdir(infile_dir)) != 0) {
            // 排除掉两个隐藏目录
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
                // 拼接输入文件的完整路径
                char full_path[MAX_FILE_PATH_SIZE] = {0};
                strcat(full_path, path_with_id);
                strcat(full_path, "/");
                strcat(full_path, ptr->d_name);
                // 打开
                FILE* infile = fopen(full_path, "rt");
                if (infile == NULL) {
                    printf("fail to open file: %s\n", full_path);
                }
                // 按行读取
                char line[MAX_COMMAND_SIZE] = {0};
                while (fgets(line, MAX_COMMAND_SIZE - 1, infile)) {
                    if (strstr(line, keyword) != NULL) {
                        // printf("line=%s\n", line);
                        fputs(ssh_ids[i], outfile);
                        fputs("\n", outfile);
                        break;  // 找到了关键字就可以跳出循环了
                    }
                }       
                // 关闭文件
                fclose(infile);
            }
        }
        closedir(infile_dir);
    }
    fclose(outfile);
}

void find_keyword_default(
    const char* ssh_input_file_path, 
    const char* keyword,
    const char* ssh_output_file_path) 
{
    // 拼接输出文件路径
    char outfile_path[MAX_FILE_PATH_SIZE] = {0};
    strcat(outfile_path, ssh_output_file_path);
    strcat(outfile_path, "/result.txt");
    // 打开
    FILE* outfile = fopen(outfile_path, "w");
    if (outfile == NULL) {
        printf("fail to open file: %s\n", outfile_path);
    }

    // 计数
    int total_id = 0;
    int total_file = 0;
    int total_match_id = 0;


    // 遍历第一级目录
    char first_path[MAX_FILE_PATH_SIZE] = {0};
    strcat(first_path, ssh_input_file_path);    // .../ssh
    DIR* first_dir;
    if(!(first_dir = opendir(first_path))) {
        printf("fail to open diectory: %s\n", first_path);
        exit(-1);
    }
    struct dirent* first_ptr;
    while ((first_ptr = readdir(first_dir)) != 0) {
        // 排除掉两个隐藏目录
        if (strcmp(first_ptr->d_name, ".") != 0 && strcmp(first_ptr->d_name, "..") != 0) {
            // 遍历第二级目录
            char second_path[MAX_FILE_PATH_SIZE] = {0};
            strcat(second_path, first_path);
            strcat(second_path, "/");
            strcat(second_path, first_ptr->d_name);     // first_ptr->d_name就是ID，...ssh/id
            DIR* second_dir;
            if(!(second_dir = opendir(second_path))) {
                printf("fail to open diectory: %s\n", second_path);
                exit(-1);
            }

            ++total_id;     // 累加一个ID目录

            int is_found = 0;
            struct dirent* second_ptr;
            while ((second_ptr = readdir(second_dir)) != 0 && is_found == 0) {
                if (strcmp(second_ptr->d_name, ".") != 0 && strcmp(second_ptr->d_name, "..") != 0) {
                    // 拼接输入文件的完整路径
                    char full_path[MAX_FILE_PATH_SIZE] = {0};
                    strcat(full_path, second_path);
                    strcat(full_path, "/");
                    strcat(full_path, second_ptr->d_name);  // ...ssh/id/filename

                    // 

                    // 打开
                    FILE* infile = fopen(full_path, "rt");
                    if (infile == NULL) {
                        printf("fail to open file: %s\n", full_path);
                    }
                    
                    ++total_file;   // 累加一份文本文件

                    // 按行读取
                    char line[MAX_COMMAND_SIZE] = {0};
                    while (fgets(line, MAX_COMMAND_SIZE - 1, infile)) {
                        if (strstr(line, keyword) != NULL) {    // 暴力find
                            // printf("line=%s\n", line);
                            ++total_match_id;   // 累加一个符合目录的
                            is_found = 1;       // 只要在一个文件中找到了关键字，就不用搜索该目录下其他文件了
                            fputs(first_ptr->d_name, outfile);
                            fputs("\n", outfile);
                            break;  // 找到了关键字就可以跳出循环了
                        }
                    }       
                    // 关闭文件
                    fclose(infile);
                }
            }
            closedir(second_dir);
        }
    }
    closedir(first_dir);
    // 写入统计数据
    fprintf(outfile, "--------------------\n");
    fprintf(outfile, "The keyword: %s\n", keyword);
    fprintf(outfile, "Total_id: %d\n", total_id);
    fprintf(outfile, "Total_file: %d\n", total_file);
    fprintf(outfile, "Total_match_id: %d\n", total_match_id);
    fprintf(outfile, "--------------------\n");
    fclose(outfile);

    // 向控制台也输出一下
    printf("--------------------\n");
    printf("The keyword: %s\n", keyword);
    printf("Total_id: %d\n", total_id);
    printf("Total_file: %d\n", total_file);
    printf("Total_match_id: %d\n", total_match_id);
}



