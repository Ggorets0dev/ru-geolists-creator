# RuGeolistsCreator

## Purpose

Software for automating the assembly of XRay VPN filtering lists. CLI language: **English**.

## Sources

The following materials are used as sources for assembling the lists:

* [ReFilter](https://github.com/1andrevich/Re-filter-lists)

* [V2Ray official](https://github.com/Loyalsoldier/v2ray-rules-dat)

* [Ruadlist](https://github.com/easylist/ruadlist)

* [Antifilter](https://antifilter.download/)

## List Building Systems

The official V2Ray software is used to build the lists:

* [v2fly/domain-list-community](https://github.com/v2fly/domain-list-community) (CC-BY-SA-4.0)

* [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

**Golang installation is required** for the software to function correctly.

## Configuration File

The configuration file is created automatically upon the first run of the software without arguments. The program then exits before list generation, allowing the user to optionally add a GitHub API token. The **apiToken** key can be manually set with a GitHub API token to increase the request limit.

The automatically generated configuration file contains all the keys listed in the example, but the extra sources array **(extra)** is initially empty.

To add sources, use objects with the following keys:
* **section** - Text label for the future section upon inclusion
* **type** - ip/domain depending on the source type
* **url** - URL to the file containing the list. All addresses should be separated by a newline

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

### Adding additional sources

Additional sources can be added to the configuration file manually or by using the `-a, --add` flag when running the software. Next, you will need to fill in all the properties inherent in such a source.

## Help (``--help``)

```
RuGeolistsCreator - Software for automatic assembly of geoip.dat and geosite.dat files for VPN server XRay. Software is focused on blocking in the Russian Federation
Usage: rglc [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --force                     Starts source download and build even if no updates are detected
  --about                     Display software information
  --check                     Check access of all source's URLs from config
  --child                     Send release notes to parent proccess (for work in chain)
  --init                      Initialize software by creating config and downloading all dependencies
  --show                      Display all extra sources from configuration files
  -a,--add                    Add extra source to download list
  --no-whitelist              Disable whitelist filtering for current session
  --no-extra                  Disable adding extra sources to lists for current session
  -f,--format TEXT ...        Formats of geolists to generate
  -r,--remove UINT            Remove extra source from download list
  -o,--out TEXT               Path to out DIR with all lists to create

Notice: When running without arguments, the update-checked mode is used

Available formats of geolists: v2ray, sing
```

## Dependencies

Project uses the following dependencies:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [libcurl](https://curl.se/libcurl/)

* [libarchive](https://libarchive.org/)

* [CLI11](https://github.com/CLIUtils/CLI11)

* [libbgpdump](https://github.com/RIPE-NCC/bgpdump)

* [c-ares](https://github.com/c-ares/c-ares)
