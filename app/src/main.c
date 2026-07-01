#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "bike_config.h"
#include "bike_state.h"

LOG_MODULE_REGISTER(bikeshare, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("Bikeshare Firmware iniciando...");

	bike_config_init();
	bike_state_init();

	LOG_INF("ID: %s", bike_config_get_id()[0]
		? bike_config_get_id() : "(nao configurado)");
	LOG_INF("Estado: %s", bike_state_name(bike_state_get()));

	return 0;
}
