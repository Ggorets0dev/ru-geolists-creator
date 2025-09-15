# RuGeolistsCreator

## Назначение

Программное обеспечение для автоматизации сборки списков фильтрации XRay VPN. Язык CLI: **Английский**.

## Источники

В качестве источников для сборки списков используются следующие материалы:

* [ReFilter](https://github.com/1andrevich/Re-filter-lists)

* [V2Ray official](https://github.com/Loyalsoldier/v2ray-rules-dat)

* [Ruadlist](https://github.com/easylist/ruadlist)

* [Antifilter](https://antifilter.download/)

## Системы сборки списков

Для сборки списков используется официальное ПО V2Ray:

* [v2fly/domain-list-community](https://github.com/v2fly/domain-list-community) (CC-BY-SA-4.0)

* [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

Для корректной работы программного обеспечения **требуется установить Golang**.

## Файл конфигурации

### Инициализация ПО

Для создания файла конфигурации требуется провести инициализацию ПО с помощью флага ```--init```, расположение файла: ```home/***/.config/ru-geolists-creator/config.json```. В процессе иницализации может быть указан API ключ для доступа GitHub (увеличивает допустимое количество запросов). Ключ может быть установлен и позднее вручную в файле конфигурации в поле ```apiToken```. 

### Пример файла конфигурации

Автоматически сформированный файл конфигурации обладает всеми перечисленными на примере ключами, но изначально массив дополнительных источников **(extra)** пуст. 

```json
{
  "apiToken" : "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",
  "dlcRootPath" : ".//v2fly-domain-list-community-adcff6c/",
  "extra" : 
  [
    {
      "section" : "discord",
      "type" : "ip",
      "url" : "https://raw.githubusercontent.com/GhostRooter0953/discord-voice-ips/refs/heads/master/voice_domains/discord-voice-ip-list"
    }
  ],
  "refilterTime" : 1736426783,
  "ruadlistTime" : 1754766886,
  "v2ipRootPath" : ".//v2fly-geoip-9711ad4/",
  "v2rayTime" : 1736449948
}
```

### Добавление дополнительных источников

Дополнительные источники могут быть добавлены в файл конфигурации вручную или с помощью флага ```-a, --add``` при запуске ПО. Далее потребуется заполнить все свойства, присущие такому источнику.

## Справка ПО (```--help```)

```
RuGeolistsCreator - Software for automatic assembly of geoip.dat and geosite.dat files for VPN server XRay. Software is focused on blocking in the Russian Federation
Usage: RuGeolistsCreator [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --force                     Starts source download and build even if no updates are detected
  --about                     Displaying software information
  --check                     Checking access of all source's URLs from config
  --child                     Sending release notes to parent proccess (for work in chain)
  --init                      Initializing software by creating config and downloading all dependencies
  --show                      Displaying all extra sources from configuration files
  -a,--add                    Adding extra source to download list
  -r,--remove UINT            Removing extra source from download list
  -o,--out TEXT               Path to out DIR with all lists to create

Notice: When running without arguments, the update-checked mode is used
```

## Зависимости

В проекте используются следующие зависимости:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [libcurl](https://curl.se/libcurl/)

* [libarchive](https://libarchive.org/)

* [CLI11](https://github.com/CLIUtils/CLI11)
