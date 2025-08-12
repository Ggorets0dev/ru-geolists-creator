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

Файл конфигурации создается автоматически при первом запуске ПО без аргументов. Далее происходит выход перед формированием списков, чтобы пользователь мог добавить токен GitHub API по желанию. Вручную для ключа **apiToken** может быть указан токен GitHub API для увеличения лимита запросов.

Автоматически сформированный файл конфигурации обладает всеми перечисленными на примере ключами, но изначально массив дополнительных источников **(extra)** пуст. 

Для добавления используются объекты со следующими ключами:
* **section** - Текстовое обозначение для будущей секции при подключении
* **type** - ip/domain в зависимости от типа источника
* **url** - Адрес к файлу со списком. Все адреса должны быть разделены переносом строки

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

## Зависимости

В проекте используются следующие зависимости:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [libcurl](https://curl.se/libcurl/)

* [libarchive](https://libarchive.org/)

* [CLI11](https://github.com/CLIUtils/CLI11)
