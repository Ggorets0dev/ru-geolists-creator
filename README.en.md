# RuGeolistsCreator

## Purpose

Software for automating the assembly of XRay / SingBox VPN filtering lists. CLI
language: **English**.

## List Build Systems

> **Note:** To run the software that builds lists, Golang must be
> installed.

The official V2Ray software is used to build lists:

- [v2fly/domain-list-community]([GitHub - v2fly/domain-list-community: Community managed domain list. Generate geosite.dat for V2Ray.](https://github.com/v2fly/domain-list-community)) (CC-BY-SA-4.0)

- [v2fly/geoip](https://github.com/v2fly/geoip) (MIT)

The following software is used to convert lists:

- [Geo](https://github.com/MetaCubeX/geo) (GPL-3.0)

## Build Automation

The following scripts can be used to automate project builds:

| Script                                              | Purpose                                                     |
|:--------------------------------------------------- | ----------------------------------------------------------- |
| build.sh [--deps] [--appimg ][--type=Debug/Release] | Build the project with dependency management                |
| prepare_appimage.sh [exec] [dir]                    | build the AppImage package separately                       |
| build_by_docker.sh                                  | build the program and AppImage package using a Docker image |

## Configuration File

### Software Initialization

To create a configuration file, initialize the software using the ```--init```
flag. File location: ```home/***/.config/ru-geolists-creator/config.json```.

During initialization you can specify a GitHub API key (increases
allowed number of requests). The key can also be added later manually in the config file in the ```apiToken``` field.

### Example Configuration File

The automatically generated config file contains all keys listed in the
example, but the additional sources (sources) and presets (presets)
arrays are initially empty.

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

### Adding Additional Presets/Sources

Additional presets and sources can be added manually by editing the
configuration file and filling in all required properties.

Supported source types:

- Local file *(file_loc)*
- Remote file *(file_remote)*
- GitHub repository release fragment *(github_release)*

## Interprocess Communication (IPC)

The **service** mode can be used for IPC. In this mode the software
accepts incoming requests via gRPC.

## Software Help (--help)

```
RuGeolistsCreator - Software (service) in C++ for automatic assembly of geoip and geosite files for VPN server XRay / SingBox
Usage: ./rglc [OPTIONS] [SUBCOMMAND]

Options:
 -h,--help Print this help message and exit
 --about Display software information
 --init Initialize software by creating config and downloading all dependencies

Subcommands:
 service Settings for service mode
 show Display all extra sources from config files
 build Build geofiles with selected presets
 check Check access of all source's URLs from config
```

## Dependencies

Main dependencies are listed in the Conan file (conanfile.txt).

Dependencies that must be installed manually in the system:

- libbgpdump
