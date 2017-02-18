#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>

class Record {
private:
    std::vector<std::string> col_name;
    std::vector<char> record_type;
    std::vector<std::string> record_lst;
};

#endif //MAIN_RECORD_H
