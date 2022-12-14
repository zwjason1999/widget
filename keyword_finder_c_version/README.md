# 需求说明

这是一个简单的需求：在指定目录下的所有文本文件中，搜索指定关键字并记录其上一级目录的名字，指定的目录格式是这样的`.../ssh`，它有很多子目录，以`ID`号命名，如`9527`,`9528`等等，在`ID`目录下才是许多文本文件。

![image-20221214200918475](https://img-bed-1304092357.cos.ap-guangzhou.myqcloud.com/keyword_finder_1.png)

# 配置文件说明

配置文件为当前目录下的`config.txt`。

因为C语言不具备动态扩容的机制，所以许多数组大小都以宏的方式定义在`main.c`的开头，如有需要请自行修改，当前默认值如下。

```c++
main.c
#define MAX_KEYWORD_SIZE 50         // 关键字最大长度
#define MAX_FILE_PATH_SIZE 256      // 文件路径最大长度
#define MAX_ID_SIZE 20              // 一个ID的最大长度
#define MAX_IDS_SIZE 5000           // ID数组的最大长度
#define MAX_COMMAND_SIZE 1024       // 一条命令的最大长度
```

按照`key-value`的形式给出，格式要求`"key=value"`，并且`key`不可以更改，`value`可更改。
1. `keyword`：要查找的关键字，为空不会执行查找。
2. `ssh_input_file_path`：ssh指令存放路径，必须已存在，必须是一个目录。
3. `ssh_output_file_path`：查找结果的存放路径，必须已存在，必须是一个目录。
4. `ssh_ids`：ID数组，格式[id1,id2,id3]，英文逗号隔开，不能有空格，为空时默认查找所有ID。

当前读配置文件的容错率较差，要严格按照格式编写。
1. 等号之间不能存在空格。
2. 注释行必须以`#`开头。
3. 结尾不能有多余的空格。
4. 目录路径最后不要多加/，如`...../ssh`即可，不能是`...../ssh/`

# 输出文件格式说明
输出目录由`output_file_path`指定，输出文件名为`result.txt`，内容为包含`keyword`的所有ID。

# 编译运行
环境要求：`gcc 4.8.5`或以上，支持`make`
1. 解压；
2. 配置文件名为`config.txt`，编写好配置文件；
3. 执行命令`make`，生成可执行文件`main`；
4. 执行`./main`。