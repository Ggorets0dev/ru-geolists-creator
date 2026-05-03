# RuGeolistsCreator

👉 [English version](README.en.md)

## Назначение

Программное обеспечение для автоматизации сборки списков фильтрации XRay / SingBox VPN. Язык CLI: **Английский**.

## Системы сборки списков

> **Замечание:** Для работы ПО, которое собирает списки, требуется установка Golang

Для сборки списков используется официальное ПО V2Ray:

* [v2fly/domain-list-community](https://github.com/v2fly/domain-list-community) (CC-BY-SA-4.0)

* [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

Для конвертирования списков используется следующее ПО:

* [Geo](https://github.com/MetaCubeX/geo) (GPL-3.0)

## Сборка

### Описание скриптов

Для автоматизации сборки проекта под могут быть использованы следующие скрипты:

| Скрипт                                              | Назначение                                                 |
|:--------------------------------------------------- | ---------------------------------------------------------- |
| build.sh [--deps] [--appimg ][--type=debug/release/relwithdebinfo] | Сборка проекта с управлением зависимостями                 |
| prepare_appimage.sh [exec] [dir]                    | Сборка пакета AppImage (отдельно)                          |
| build_by_docker.sh                                  | Сборка программы и пакета AppImage с помощью docker образа |

### Алгоритм сборки AppImage пакета

Для сборки пакета в формате AppImage требуется:

```bash
# Загрузить appimagetool и сделать его доступным в PATH
wget https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage -O /usr/local/bin/appimagetool

# Создать Docker образ для сборки
docker build -t rglc-builder-dev .

# Запустить сборку
./scripts/build_by_docker.sh
```

В результате в корне проекта появится ```rglc-ARCH.AppImage```

## Файл конфигурации

### Инициализация ПО

Для создания файла конфигурации требуется провести инициализацию ПО с помощью флага ```--init```, расположение файла: ```home/***/.config/ru-geolists-creator/config.json```. В процессе иницализации может быть указан API ключ для доступа GitHub (увеличивает допустимое количество запросов). Ключ может быть установлен и позднее вручную в файле конфигурации в поле ```apiToken```. 

### Пример файла конфигурации

Автоматически сформированный файл конфигурации обладает всеми перечисленными на примере ключами, но изначально массивы дополнительных источников **(sources)** и пресетов **(presets)** пусты. 

```json
{
  "apiToken" : "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",
  "bgpDumpPath" : null,
  "dlcRootPath" : "/home/ggorets0/.local/lib/v2fly-dlc-toolchain",
  "geoMgrBinaryPath" : "/home/ggorets0/.local/lib/geo-linux-amd64",
  "presets" : [
      {
          "id": 1,
          "label": "client",
          "source_ids": [1, 2, 3]
      },
      {
          "id": 2,
          "label": "server",
          "source_ids": [1, 4]
      }
  ],
  "sources" : [
      {
          "id": 1,
          "storage_type": "file_loc",
          "inet_type": "domain",
          "url": "/home/user/RGLC/lists/gov.txt",
          "section": "gov"
      },
      {
            "id": 2,
            "section": "refilter",
            "storage_type": "github_release",
            "inet_type": "domain",
            "url": "https://github.com/1andrevich/Re-filter-lists",
            "assets": ["domains_all.lst"]
      },
      {
            "id": 3,
            "section": "refilter",
            "storage_type": "github_release",
            "inet_type": "ip",
            "url": "https://github.com/1andrevich/Re-filter-lists",
            "assets": ["ipsum.lst"]
      },
      {
            "id": 4,
            "storage_type": "file_loc",
            "inet_type": "domain",
            "section": "google",
            "url": "/home/user/RGLC/lists/google"
      }
  ],
  "v2ipRootPath" : "/home/ggorets0/.local/lib/v2fly-v2ip-toolchain",
  "whitelistPath" : null
}
```

### Добавление дополнительных пресетов/источников

Дополнительные пресеты/источники могут быть добавлены в файл конфигурации вручную с помощью редактирования файла конфигурации. Далее потребуется заполнить все свойства, присущие такому пресету/источнику.

Поддерживаются следующие виды источников:

* Локальный файл *(file_loc)*

* Удаленный файл *(file_remote)*

* Фрагмент релиза GitHub репозитория *(github_release)*

* Номер автомноной сети (ASN) *(as)*

### Примеры использования

```bash
# Собрать пресет "node-in" в форматах V2RAY, SING-DB, SING-RS и сохранить в папке "/home/user/RGLC/releases"
./rglc-x86_64.AppImage build -p node-in -f v2ray -f sing-db -f sing-rs -o /home/user/RGLC/releases

# Проверить доступность источников в пресете "node-in"
./rglc-x86_64.AppImage check -p node-in

# Отобразить список источников во всех пресетах
./rglc-x86_64.AppImage show
```

## Межпроцессное взаимодействие (IPC)

Для IPC может быть использован режим **service**. В данном режиме ПО принимает входящие запросы по протоколу gRPC.

## Справка ПО (```--help```)

```
RuGeolistsCreator - Software (service) in C++ for automatic assembly of geoip and geosite files for VPN server XRay / SingBox
Usage: ./rglc [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --about                     Display software information
  --init                      Initialize software by creating config and downloading all dependencies

Subcommands:
  service                     Settings for service mode
  show                        Display all extra sources from config files
  build                       Build geofiles with selected presets
  check                       Check access of all source's URLs from config
```

## Зависимости

Основные зависимости перечислены в файле [Conan](conanfile.txt).

Зависимости, которые должны быть установлены в систему без участия conan:

* [libbgpdump](https://github.com/RIPE-NCC/bgpdump)
