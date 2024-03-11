#include "../logger.h"

#include <unistd.h>

using namespace std;

int main()
{
    Logger::Instance().Init("log.txt", LogLevel::INFO);

    for (int i = 0; i < 100; ++i) {
        DEBUG("hello %s, I'am %d years old.", "zwjason", 25);
        INFO("hello %s, I'am %d years old.", "zwjason", 25);
        WARN("hello %s, I'am %d years old.", "zwjason", 25);
        ERROR("hello %s, I'am %d years old.", "zwjason", 25);
        // std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    return 0;
}