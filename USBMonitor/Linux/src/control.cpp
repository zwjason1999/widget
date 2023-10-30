#include "control.h"
#include "logger.h"

#include <cstdio>
#include <cstdlib>


void RemoveAuthorization(char *tree_path)
{
    char cmd[128];
    if (sprintf(cmd, "echo 0 | tee /sys/bus/usb/devices/%s/authorized > /dev/null", tree_path) < 0) {
        ERROR("Buffer size is too small.\n");
        return;
    }
    if (system(cmd) == -1) {
        ERROR("Failed to run cmd: %s\n", cmd);
        return;
    }
    INFO("Succeed in running cmd: %s\n", cmd);
}