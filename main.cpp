#include <iostream>
#include <fstream>

#include "network.hpp"
#include "dlc_toolchain.hpp"
#include "archive.hpp"

int main() {
    downloadDlcSourceCode();

    auto dlcRootPath = extractTarGz("/home/uav/Projects/C++/build-RuGeolistsCreator-Desktop-Debug/dlc_src.tar.gz", "/home/uav/Projects/C++/build-RuGeolistsCreator-Desktop-Debug");
}
