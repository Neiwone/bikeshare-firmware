#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bikeshare, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Hello from Bikeshare Firmware!");

    while (1) {
        LOG_INF("Firmware running...");
        k_sleep(K_SECONDS(5));
    }

    return 0;
}