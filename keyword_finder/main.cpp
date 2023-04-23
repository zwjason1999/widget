#include <fstream>
#include <string>

#include <cstdlib>
#include <sys/stat.h>
#include <dirent.h>

#include <jsoncpp/json/json.h>

using namespace std;

/**
 * 环境 Linux g++ 4.8.5
*/

// 与配置文件字段对应
string INPUT;
string OUTPUT;
vector<string> IDS;

// 其他必要全局变量
string INPUT_COMMAND_FILE_NAME = "tp-ssh-cmd.txt";
string OUTPUT_COMMAND_FILE_NAME = "tp-sshdeltime.dat";

// 根据业务场景
// 文件路径的前缀只有可能是以下3个中的一个
vector<string> OPTIONAL_PREFIX {
    "/home/kuangan/Videos/Master/replay/ssh",
    "/home/kuangan/Videos/Slave/replay/ssh",
    "/home/kuangan/Videos/replay/ssh"
};


/**
 * @brief 判断给定字符串的后缀是否与指定后缀相同
 * @param s 待判断的字符串
 * @param suffix 指定的后缀
 * @return 如果s以suffix结尾返回true, 否则返回false
*/
bool endsWith(const string& s, const string& suffix)
{
    return s.length() >= suffix.length()
        && s.substr(s.length() - suffix.length()) == suffix;
}


/**
 * @brief 判断路径是否存在
 * @param path 待判断的路径
 * @return 存在返回true，否则返回false
 */
bool is_exist(const char* path) {
    struct stat buf;
    return stat(path, &buf) == 0;
}


/**
 * @brief 判断路径是否为文件夹
 * @param path 路径
 * @return 文件夹返回true，否则返回false
 */
bool is_directory(const char* path) {
    struct stat buf;
    return is_exist(path) && S_ISDIR(buf.st_mode);
}


/**
 * @brief 解析配置文件
 * @param config_file_path 配置文件路径
 * @return void
 */
void parseConfig(const string& config_file_path)
{
    /* 打开配置文件 */
    ifstream ifs;
    ifs.open(config_file_path);
    if (!ifs.is_open()) {
        printf("error: failed to open config file, path is not exist.\n");
        exit(-1);
    }
    
    // 使用jsoncpp读取配置文件的通用方法
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root, false)) {
        printf("error: failed to parse config file, syntax error.\n");
        exit(-1);
    }

    ifs.close();    // 用不到了就关闭

    INPUT = root["input"].asString();
    OUTPUT = root["output"].asString();
    // 根据业务说明, 配置文件中的id可能是1-11位数字
    // 如果id的位数少于9, 则通过在前面补0格式化为9位
    int ids_cnt = root["ids"].size();
    for (int i = 0; i < ids_cnt; ++i) {
        string id = root["ids"][i].asString();
        if (id.length() > 11) {
            printf("warning: The length of id: %s is %lu, more than 11.\n", id.c_str(), id.length());
        }
        
        IDS.emplace_back(id);
    }
    // 若IDS为空, 则默认遍历所有id
    // 此时不需要规范化id
    // if (IDS.empty()) {    
    //     DIR* prefix_dir = opendir(prefix.c_str());
    //     if (!prefix_dir) {
    //         printf("error: failed to open %s.\n", prefix.c_str());
    //         exit(-1);
    //     }
    //     struct dirent* prefix_ptr;
    //     while ((prefix_ptr = readdir(prefix_dir)) != nullptr) {
    //         // 排除掉两个隐藏目录
    //         if (strcmp(prefix_ptr->d_name, ".") != 0 && strcmp(prefix_ptr->d_name, "..") != 0) {
    //             IDS.emplace_back(prefix_ptr->d_name);
    //         }
    //     }
    //     closedir(prefix_dir);
    // }
}


/**
 * @brief 在指定路径表示的文件中查找指定关键字
 * @param keyword 关键字
 * @param path 路径
 * @return 如果找到了返回true, 否则返回false
 */
bool find_keyword(const string& keyword, const string& path)
{
    // printf("111");
    if (path.empty()) { 
        return false; 
    }
    // printf("222");
    ifstream ifs;
    ifs.open(path);
    if (!ifs.is_open()) {
        // printf("failed to open %s\n", path.c_str());
        return false;
    }
    // printf("333");
    string line;
    bool found = false;
    while (getline(ifs, line)) {
        if (line.find(keyword) != string::npos) {
            found = true;
            break;
        }
    }
    ifs.close();
    return found;
}



int main(int argc, char* argv[])
{
    // 参数校验
    if (argc < 3 || string(argv[1]) != "-config" || !argv[2]) {
        printf("usage: ./kashell -config config_file_path.\n");
        exit(-1);
    }
    const string config_file_path(argv[2]);
    if (!endsWith(config_file_path, ".json")) {
        printf("error: The format of config file must be json.\n");
        exit(0);
    }

    parseConfig(config_file_path);  // 解析配置文件
    
    // printf("size=%lu\n", IDS.size());
    // printf("INPUT=%s\n", INPUT.c_str());
    // printf("OUTPUT=%s\n", OUTPUT.c_str());

    
    vector<string> match_ids;
    for (size_t i = 0; i < IDS.size(); ++i) {
        // 格式化id
        string format_id = IDS[i];
        if (format_id.length() < 9) {
            format_id = string(9 - format_id.length(), '0') + format_id;
        }
        for (size_t j = 0; j < OPTIONAL_PREFIX.size(); ++j) {
            // 先判断路径是否存在
            string prifix_plus_id = OPTIONAL_PREFIX[j] + "/" + format_id;
            if (!is_exist(prifix_plus_id.c_str())) { continue; }
            
            const string input_full_path = prifix_plus_id + "/" + INPUT_COMMAND_FILE_NAME;
            const string output_full_path = prifix_plus_id + "/" + OUTPUT_COMMAND_FILE_NAME;

            bool find_input = !INPUT.empty() && find_keyword(INPUT, input_full_path);
            bool find_output = !OUTPUT.empty() && find_keyword(OUTPUT, output_full_path);

            // 1.指定了INPUT并且找到了, 没有指定OUTPUT
            // 2.指定了OUTPUT并且找到了, 没有指定INPUT
            // 3.都指定了并且都找到了
            if ((find_input && OUTPUT.empty())
                || (find_output && INPUT.empty())
                || (find_input && find_output)) 
            {
                match_ids.emplace_back(IDS[i]);
                // printf("%s\n", IDS[i].c_str());
                break;  // 找到了就可以跳过该id了
            }
        }
        
    }
    
    for (size_t i = 0; i < match_ids.size(); ++i) {
        printf("%s\n", match_ids[i].c_str());
    }

    // printf("total %lu\n", match_ids.size());
    return 0;
}