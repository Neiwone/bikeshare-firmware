#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/shell/shell.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#ifdef CONFIG_NETWORKING
#include <zephyr/net/socket.h>
#endif

#include "bike_config.h"

#define BIKE_TEST_PATH "/health"

LOG_MODULE_REGISTER(bike_config, LOG_LEVEL_INF);

static struct {
	char id[BIKE_ID_MAX_LEN];
	char token[BIKE_TOKEN_MAX_LEN];
	char url[BIKE_URL_MAX_LEN];
} cfg;

/* ---------- settings handler ---------- */

static int settings_set_cb(const char *key, size_t len,
			    settings_read_cb read_cb, void *cb_arg)
{
	const char *next;
	int rc;

	if (settings_name_steq(key, "id", &next) && !next) {
		if (len >= sizeof(cfg.id)) {
			return -EINVAL;
		}
		rc = read_cb(cb_arg, cfg.id, len);
		if (rc >= 0) {
			cfg.id[rc] = '\0';
		}
		return rc < 0 ? rc : 0;
	}

	if (settings_name_steq(key, "token", &next) && !next) {
		if (len >= sizeof(cfg.token)) {
			return -EINVAL;
		}
		rc = read_cb(cb_arg, cfg.token, len);
		if (rc >= 0) {
			cfg.token[rc] = '\0';
		}
		return rc < 0 ? rc : 0;
	}

	if (settings_name_steq(key, "url", &next) && !next) {
		if (len >= sizeof(cfg.url)) {
			return -EINVAL;
		}
		rc = read_cb(cb_arg, cfg.url, len);
		if (rc >= 0) {
			cfg.url[rc] = '\0';
		}
		return rc < 0 ? rc : 0;
	}

	return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(bike, "bike", NULL, settings_set_cb, NULL, NULL);

/* ---------- public API ---------- */

int bike_config_init(void)
{
	int rc = settings_load();

	if (rc) {
		LOG_ERR("Falha ao carregar configurações: %d", rc);
	}
	return rc;
}

const char *bike_config_get_id(void)    { return cfg.id; }
const char *bike_config_get_token(void) { return cfg.token; }
const char *bike_config_get_url(void)   { return cfg.url; }

static int save_field(const char *settings_key, const char *value,
		      char *buf, size_t buf_len)
{
	size_t len = strlen(value);

	if (len >= buf_len) {
		return -EINVAL;
	}
	memcpy(buf, value, len + 1);

	/* settings_runtime_set() stores in RAM and triggers the set_cb.
	 * For persistent backends (NVS, ZMS) swap to settings_save_one(). */
#if defined(CONFIG_SETTINGS_RUNTIME)
	return settings_runtime_set(settings_key, buf, len);
#else
	return settings_save_one(settings_key, buf, len);
#endif
}

int bike_config_set_id(const char *id)
{
	return save_field("bike/id", id, cfg.id, sizeof(cfg.id));
}

int bike_config_set_token(const char *token)
{
	return save_field("bike/token", token, cfg.token, sizeof(cfg.token));
}

int bike_config_set_url(const char *url)
{
	return save_field("bike/url", url, cfg.url, sizeof(cfg.url));
}

/* ---------- shell commands ---------- */

static int cmd_bike_set_id(const struct shell *sh, size_t argc, char **argv)
{
	int rc = bike_config_set_id(argv[1]);

	if (rc) {
		shell_error(sh, "Erro ao salvar ID: %d", rc);
		return rc;
	}
	shell_print(sh, "ID configurado: %s", argv[1]);
	return 0;
}

static int cmd_bike_set_token(const struct shell *sh, size_t argc, char **argv)
{
	int rc = bike_config_set_token(argv[1]);

	if (rc) {
		shell_error(sh, "Erro ao salvar token: %d", rc);
		return rc;
	}
	shell_print(sh, "Token configurado.");
	return 0;
}

static int cmd_bike_set_url(const struct shell *sh, size_t argc, char **argv)
{
	int rc = bike_config_set_url(argv[1]);

	if (rc) {
		shell_error(sh, "Erro ao salvar URL: %d", rc);
		return rc;
	}
	shell_print(sh, "URL configurada: %s", argv[1]);
	return 0;
}

static int cmd_bike_get(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "ID:    %s", cfg.id[0]    ? cfg.id    : "(nao definido)");
	shell_print(sh, "Token: %s", cfg.token[0] ? cfg.token : "(nao definido)");
	shell_print(sh, "URL:   %s", cfg.url[0]   ? cfg.url   : "(nao definido)");
	return 0;
}

/* ---------- bike test ---------- */

#ifdef CONFIG_NETWORKING

struct parsed_url {
	char     host[64];
	char     path[BIKE_URL_MAX_LEN];
	uint16_t port;
};

static int url_parse(const char *url, struct parsed_url *out)
{
	const char *p;

	if (strncmp(url, "https://", 8) == 0) {
		out->port = 443;
		p = url + 8;
	} else if (strncmp(url, "http://", 7) == 0) {
		out->port = 80;
		p = url + 7;
	} else {
		return -EINVAL;
	}

	/* advance to end of host */
	const char *host_end = p;

	while (*host_end && *host_end != ':' && *host_end != '/') {
		host_end++;
	}

	size_t hlen = host_end - p;

	if (hlen == 0 || hlen >= sizeof(out->host)) {
		return -EINVAL;
	}
	memcpy(out->host, p, hlen);
	out->host[hlen] = '\0';

	p = host_end;

	if (*p == ':') {
		p++;
		out->port = (uint16_t)atoi(p);
		while (*p && *p != '/') {
			p++;
		}
	}

	strncpy(out->path, *p == '/' ? p : "/", sizeof(out->path) - 1);
	out->path[sizeof(out->path) - 1] = '\0';

	return 0;
}

#endif /* CONFIG_NETWORKING */

static int cmd_bike_test(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

#ifndef CONFIG_NETWORKING
	shell_error(sh, "CONFIG_NETWORKING=y necessario no prj.conf");
	return -ENOTSUP;
#else
	const char *url = bike_config_get_url();

	if (!url[0]) {
		shell_error(sh, "URL nao configurada. Use: bike set url <URL>");
		return -EINVAL;
	}

	struct parsed_url purl;

	if (url_parse(url, &purl) != 0) {
		shell_error(sh, "Formato invalido: %s", url);
		return -EINVAL;
	}

	shell_print(sh, "Conectando a %s:%u...", purl.host, purl.port);

	int sock = -1;
	int rc   = -EIO;

	/* Para IPs numéricos, bypassa o getaddrinfo/DNS completamente e
	 * constrói o sockaddr diretamente com inet_pton. */
	struct sockaddr_in addr4 = { .sin_family = AF_INET,
				     .sin_port   = htons(purl.port) };

	if (zsock_inet_pton(AF_INET, purl.host, &addr4.sin_addr) == 1) {
		sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock >= 0) {
			struct zsock_timeval tv = { .tv_sec = 10 };

			zsock_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
			zsock_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
			rc = zsock_connect(sock, (struct sockaddr *)&addr4, sizeof(addr4));
		}
	} else {
		/* Hostname: usa getaddrinfo + DNS */
		char port_str[8];

		snprintf(port_str, sizeof(port_str), "%u", purl.port);

		struct zsock_addrinfo hints = { .ai_family   = AF_UNSPEC,
						.ai_socktype = SOCK_STREAM };
		struct zsock_addrinfo *res = NULL;

		rc = zsock_getaddrinfo(purl.host, port_str, &hints, &res);
		if (rc != 0) {
			shell_error(sh, "Falha na resolucao DNS (%s): %d", purl.host, rc);
			return -EIO;
		}
		sock = zsock_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock >= 0) {
			struct zsock_timeval tv = { .tv_sec = 10 };

			zsock_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
			zsock_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
			rc = zsock_connect(sock, res->ai_addr, res->ai_addrlen);
		}
		zsock_freeaddrinfo(res);
	}

	if (sock < 0) {
		shell_error(sh, "Falha ao criar socket");
		return -EIO;
	}
	if (rc != 0) {
		shell_error(sh, "Falha na conexao: %d", rc);
		zsock_close(sock);
		return -EIO;
	}

	char req[256];
	int req_len = snprintf(req, sizeof(req),
			       "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
			       BIKE_TEST_PATH, purl.host);

	if (zsock_send(sock, req, req_len, 0) < 0) {
		shell_error(sh, "Falha ao enviar requisicao");
		zsock_close(sock);
		return -EIO;
	}

	char resp[128];
	int n = zsock_recv(sock, resp, sizeof(resp) - 1, 0);

	zsock_close(sock);

	if (n <= 0) {
		shell_error(sh, "Sem resposta do servidor");
		return -EIO;
	}

	resp[n] = '\0';

	int status = 0;

	if (sscanf(resp, "HTTP/%*s %d", &status) == 1) {
		if (status >= 200 && status < 300) {
			shell_print(sh, "OK - HTTP %d", status);
		} else {
			shell_print(sh, "HTTP %d", status);
		}
	} else {
		shell_print(sh, "Resposta recebida (%d bytes)", n);
	}

	return 0;
#endif /* CONFIG_NETWORKING */
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_bike_set,
	SHELL_CMD_ARG(id,    NULL, "<ID> Identificador da bicicleta", cmd_bike_set_id,    2, 0),
	SHELL_CMD_ARG(token, NULL, "<TOKEN> Token de associacao",      cmd_bike_set_token, 2, 0),
	SHELL_CMD_ARG(url,   NULL, "<URL> Endereco da API",            cmd_bike_set_url,   2, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_bike,
	SHELL_CMD(set,  &sub_bike_set, "Configura parametros da bicicleta",  NULL),
	SHELL_CMD(get,  NULL,          "Exibe configuracao atual",            cmd_bike_get),
	SHELL_CMD(test, NULL,          "Testa comunicacao com o backend",     cmd_bike_test),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(bike, &sub_bike, "Gerenciamento da bicicleta", NULL);
