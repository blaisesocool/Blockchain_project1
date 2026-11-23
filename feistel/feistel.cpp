#include <iostream>


const unsigned int ROUNDS = 16;



unsigned int roundFunction(unsigned int right, unsigned int key) {
    return right^key; // just an XOR
}

void assignKeys(unsigned int masterKey, unsigned int keys[ROUNDS]) {
    for (int i = 0; i < ROUNDS; i++) {
        keys[i] = masterKey + i;
    }
}



int main() {
    unsigned int left = 5;
    unsigned int right = 9;
    unsigned int masterKey = 30;
    unsigned int keys[ROUNDS] = {0};
    unsigned int prevright = right;
    unsigned int prevleft = left;


    std::cout << "Before: " << left << " | " << right << '\n';


    assignKeys(masterKey, keys);

    // encrypt
    for (int i = 0; i<ROUNDS; i++){
        left = prevright; 
        right = prevleft^roundFunction(prevright, keys[i]);
        prevright = right;
        prevleft = left;
    }
    std::cout << "Encrypt:  " << left << " | " << right << '\n';

    // decrypt
    for (int i = ROUNDS - 1; i >= 0; i--){
        prevleft = right^roundFunction(left, keys[i]); 
        prevright = left;
        right = prevright;
        left = prevleft;
    }
    std::cout << "decrypt:  " << left << " | " << right << '\n';


    return 0;
}
