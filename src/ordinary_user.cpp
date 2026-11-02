#include "ordinary_user.h"
#include "menus.h"
#include <istream>
#include <ostream>
#include <string>

bool runOrdinaryUserSession(std::istream& in, std::ostream& out, const ParkingService& service) {
    std::string choice;
    while (true) {
        out << "Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout\n> ";
        if (!std::getline(in, choice)) return false;
        if (choice == "0") return true;
        if (!runReadChoice(choice, in, out, service)) return false;
    }
}
