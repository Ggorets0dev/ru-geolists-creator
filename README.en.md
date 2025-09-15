# RuGeolistsCreator
👉 [Русская версия](README.md)

## Purpose

Software for automating the assembly of filtering lists for XRay VPN. CLI language: **English**.

## Sources

The following materials are used as sources for list generation:

* [ReFilter](https://github.com/1andrevich/Re-filter-lists)

* [V2Ray official](https://github.com/Loyalsoldier/v2ray-rules-dat)

* [Ruadlist](https://github.com/easylist/ruadlist)

* [Antifilter](https://antifilter.download/)

## List Building Systems

Official V2Ray software is used for building lists:

* [v2fly/domain-list-community](https://github.com/v2fly/domain-list-community) (CC-BY-SA-4.0)

* [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

For proper operation, **Golang must be installed**.

## Configuration File

### Software Initialization

To create a configuration file, initialize the software with the ```--init``` flag. File location: ```home/***/.config/ru-geolists-creator/config.json```.  
During initialization, a GitHub API key may be specified (increases the number of allowed requests).  
The key can also be set later manually in the configuration file under the ```apiToken``` field.

### Example Configuration File

The automatically generated configuration file contains all the keys listed in the example, but the array of additional sources **(extra)** is initially empty.

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

### Adding Additional Sources

Additional sources can be added to the configuration file manually or with the ```-a, --add``` flag when running the software. All properties for such a source must then be filled in.

## Software Help (```--help```)

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

## Dependencies

The project uses the following dependencies:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [libcurl](https://curl.se/libcurl/)

* [libarchive](https://libarchive.org/)

* [CLI11](https://github.com/CLIUtils/CLI11)
