#include <errno.h>
#include <string.h>

#include <zephyr/ztest.h>

#include "bike_config.h"
#include "bike_state.h"

ZTEST(bike_config, test_config_validation)
{
	struct bike_config cfg = { 0 };
	uint16_t port;

	zassert_false(bike_config_is_valid(&cfg));

	zassert_ok(bike_config_parse_mqtt_port("1883", &port));
	zassert_equal(port, 1883);
	zassert_equal(bike_config_parse_mqtt_port("0", &port), -EINVAL);
	zassert_equal(bike_config_parse_mqtt_port("65536", &port), -EINVAL);
	zassert_equal(bike_config_parse_mqtt_port("abc", &port), -EINVAL);

	strcpy(cfg.id, "BIKE_001");
	strcpy(cfg.device_token, "TOKEN");
	strcpy(cfg.mqtt_host, "broker.example.com");
	cfg.mqtt_port = 1883;
	strcpy(cfg.apn, "internet");

	zassert_true(bike_config_is_valid(&cfg));
}

ZTEST(bike_state, test_core_state_transitions)
{
	zassert_ok(bike_config_init());
	zassert_ok(bike_state_init());
	zassert_equal(bike_state_get(), BIKE_STATE_UNREGISTERED);

	zassert_ok(bike_config_set_id("BIKE_001"));
	zassert_ok(bike_config_set_device_token("TOKEN"));
	zassert_ok(bike_config_set_mqtt_host("broker.example.com"));
	zassert_ok(bike_config_set_mqtt_port(1883));
	zassert_ok(bike_config_set_apn("internet"));
	zassert_ok(bike_state_refresh_config());
	zassert_equal(bike_state_get(), BIKE_STATE_AVAILABLE);

	zassert_equal(bike_state_cancel("RENTAL_000"), -EACCES);
	zassert_ok(bike_state_authorize("RENTAL_001"));
	zassert_equal(bike_state_get(), BIKE_STATE_RESERVED);
	zassert_equal(strcmp(bike_state_get_rental_id(), "RENTAL_001"), 0);

	zassert_equal(bike_state_authorize("RENTAL_002"), -EACCES);
	zassert_equal(bike_state_cancel("RENTAL_002"), -EINVAL);
	zassert_equal(bike_state_get(), BIKE_STATE_RESERVED);

	zassert_ok(bike_state_button_press());
	zassert_equal(bike_state_get(), BIKE_STATE_IN_USE);
	zassert_equal(bike_state_cancel("RENTAL_001"), -EACCES);

	zassert_ok(bike_state_button_press());
	zassert_equal(bike_state_get(), BIKE_STATE_AVAILABLE);
	zassert_equal(bike_state_get_rental_id()[0], '\0');
}

ZTEST_SUITE(bike_config, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(bike_state, NULL, NULL, NULL, NULL, NULL);
