# Bikeshare Firmware

Firmware embarcado para o sistema de bicicletas compartilhadas, desenvolvido em C com Zephyr RTOS.

## Requisitos

- CMake
- West
- Zephyr

## Instalação

Inicialize o workspace usando este repositório como manifest:

```bash
west init -l bikeshare-firmware
```

Baixe o Zephyr e os módulos:

```bash
west update
west zephyr-export
pip install -r zephyr/scripts/requirements.txt
```

Instale o Zephyr SDK:

```bash
west sdk install
```

## Rodando com native_sim

A partir da raiz do workspace:

```bash
cd ~/bikeshare-workspace
source .venv/bin/activate
```

Build para `native_sim`:

```bash
west build -b native_sim bikeshare-firmware/app -d build/native_sim -p always
```

Executar:

```bash
west build -d build/native_sim -t run
```