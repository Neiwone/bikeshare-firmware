#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "button_input.h"
#include "bike_config.h"
#include "bike_state.h"
#include "led_status.h"

LOG_MODULE_REGISTER(bikeshare, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("Bikeshare Firmware iniciando...");

	bike_config_init();
	led_status_init();
	button_input_init();
	bike_state_init();

	LOG_INF("ID: %s", bike_config_get_id()[0]
		? bike_config_get_id() : "(nao configurado)");
	LOG_INF("Estado: %s", bike_state_name(bike_state_get()));

	return 0;
}
