# RuGeolistsCreator

👉 [Русская версия](README.md)

## Purpose

Software for automating the assembly of XRay VPN filtering lists. CLI language: **English**.

## Sources

The following materials are used as sources for building the lists:

* [ReFilter](https://github.com/1andrevich/Re-filter-lists)

* [V2Ray official](https://github.com/Loyalsoldier/v2ray-rules-dat)

* [Ruadlist](https://github.com/easylist/ruadlist)

* [Antifilter](https://antifilter.download/)

## List Building Systems

The official V2Ray software is used for building the lists:

* [v2fly/domain-list-community](https://github.com/v2fly/domain-list-community) (CC-BY-SA-4.0)

* [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

The following software is used for converting the lists:

* [Geo](https://github.com/MetaCubeX/geo) (GPL-3.0)

To work correctly, **Golang must be installed**.

## Configuration File

### Initializing the Software

To create a configuration file, initialization must be performed using the ```--init``` flag. File location: ```home/***/.config/ru-geolists-creator/config.json```. During initialization, a GitHub API key can be specified (increases the allowed number of requests). The key can also be set later manually in the configuration file in the ```apiToken``` field.

### Example Configuration File

The automatically generated configuration file contains all the keys listed in the example, but initially the additional sources array **(extra)** is empty.

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
  "v2rayTime" : 1736449948,
  "geoMgrBinaryPath" : "/home/uav/.local/lib/geo-linux-amd64"
}
```

### Adding Additional Sources

Additional sources can be added to the configuration file manually or using the ```-a, --add``` flag when running the software. All properties relevant to such a source must then be filled in.

## Software Help (```--help```)

```
RuGeolistsCreator - Software for automatic assembly of geoip.dat and geosite.dat files for VPN server XRay. Software is focused on blocking in the Russian Federation
Usage: ./RuGeolistsCreator [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --force                     Starts source download and build even if no updates are detected
  --about                     Displaying software information
  --check                     Checking access of all source's URLs from config
  --child                     Sending release notes to parent proccess (for work in chain)
  --init                      Initializing software by creating config and downloading all dependencies
  --show                      Displaying all extra sources from configuration files
  -a,--add                    Adding extra source to download list
  -f,--format TEXT ...        Formats of geolists to generate
  -r,--remove UINT            Removing extra source from download list
  -o,--out TEXT               Path to out DIR with all lists to create

Notice: When running without arguments, the update-checked mode is used

Available formats of geolists: v2ray, sing
```

## Dependencies

The project uses the following dependencies:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [libcurl](https://curl.se/libcurl/)

* [libarchive](https://libarchive.org/)

* [CLI11](https://github.com/CLIUtils/CLI11)
