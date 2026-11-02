#include "menus.h"
#include "parking_service.h"
#include "storage.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    auto path = std::filesystem::temp_directory_path() / "parking_menu_test_e05.txt";
    std::filesystem::remove(path); std::filesystem::remove(path.string() + ".bak");
    ParkingService service; Storage storage(path);
    if (!storage.load(service).ok) return 1;
    std::istringstream input("2\n4\nAA1\nSUV\nWhite\n2020-01-01 00:00\n5\nAA1\nBB2\nSedan\nRed\n2020-01-01 00:00\n0\n1\n1\n2\nBB2\n4\n0\n2\n6\nBB2\n0\n0\n");
    std::ostringstream output; runMenu(input, output, service, storage);
    auto text = output.str();
    if (text.find("Updated and saved") == std::string::npos || text.find("Plate: BB2") == std::string::npos || text.find("Fee:") == std::string::npos || text.find("Invalid choice.") == std::string::npos || service.count() != 0) return 2;
    ParkingService reloaded; Storage reload(path);
    if (!reload.load(reloaded).ok || reloaded.count() != 0) return 3;
    Storage blocked(path); ParkingService unsaved;
    std::istringstream failInput("2\n4\nCC3\nSUV\nBlue\n2020-01-01 00:00\n0\n0\n");
    std::ostringstream failOutput; runMenu(failInput, failOutput, unsaved, blocked);
    if (failOutput.str().find("Updated in memory, but save failed") == std::string::npos || unsaved.count() != 1) return 4;
    std::filesystem::remove(path); std::filesystem::remove(path.string() + ".bak");
    std::cout << "Menu workflow and save failure tests passed.\n";
}
