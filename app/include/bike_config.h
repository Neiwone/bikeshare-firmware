#pragma once

#define BIKE_ID_MAX_LEN     32
#define BIKE_TOKEN_MAX_LEN  128
#define BIKE_URL_MAX_LEN    128

int bike_config_init(void);

const char *bike_config_get_id(void);
const char *bike_config_get_token(void);
const char *bike_config_get_url(void);

int bike_config_set_id(const char *id);
int bike_config_set_token(const char *token);
int bike_config_set_url(const char *url);
