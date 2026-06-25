#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "bike_config.h"

LOG_MODULE_REGISTER(bikeshare, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("Bikeshare Firmware iniciando...");

	bike_config_init();

	LOG_INF("ID: %s", bike_config_get_id()[0]
		? bike_config_get_id() : "(nao configurado)");

	return 0;
}