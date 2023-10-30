#include "src/base.h"
#include "src/monitor.h"
#include "src/logger.h"

int main() 
{
    SetLogLevel(LogLevel::DEBUG);
    SetLogFile("./monitoring.log");
    Init();
    MonitorHotplug();
    Cleanup();
    return 0;
}