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


// The & parameters let these functions update the caller's left and right.
void encrypt(unsigned int& left, unsigned int& right, const unsigned int keys[ROUNDS]) {
    unsigned int prevright = right;
    unsigned int prevleft = left;

    for (int i = 0; i < ROUNDS; i++) {
        left = prevright;
        right = prevleft ^ roundFunction(prevright, keys[i]);
        prevright = right;
        prevleft = left;
    }
}

void decrypt(unsigned int& left, unsigned int& right, const unsigned int keys[ROUNDS]) {
    for (int i = ROUNDS - 1; i >= 0; i--) {
        unsigned int prevleft = right ^ roundFunction(left, keys[i]);
        unsigned int prevright = left;
        right = prevright;
        left = prevleft;
    }
}

int main() {
    unsigned int left = 5;
    unsigned int right = 9;
    unsigned int masterKey = 30;
    unsigned int keys[ROUNDS] = {0};


    std::cout << "Before: " << left << " | " << right << '\n';


    assignKeys(masterKey, keys);

    encrypt(left, right, keys);
    std::cout << "Encrypt:  " << left << " | " << right << '\n';

    decrypt(left, right, keys);
    std::cout << "decrypt:  " << left << " | " << right << '\n';


    return 0;
}
