#include  <iostream>

#include <string>
std::string raw_txt = "";

constexpr uint8_t IP[64] = {
    58,50,42,34,26,18,10, 2,
    60,52,44,36,28,20,12, 4,
    62,54,46,38,30,22,14, 6,
    64,56,48,40,32,24,16, 8,
    57,49,41,33,25,17, 9, 1,
    59,51,43,35,27,19,11, 3,
    61,53,45,37,29,21,13, 5,
    63,55,47,39,31,23,15, 7
};

uint64_t permute(uint64_t in, const uint8_t* table, int n_out, int n_in) {
    uint64_t out = 0;
    for (int i = 0; i < n_out; i++) {
        uint64_t bit = (in >> (n_in - table[i])) & 1;  
        out = (out << 1) | bit;                        
    return out;
}

int main() {

}


