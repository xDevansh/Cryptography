#include <iostream>
#include <string>
using namespace std;

string caesarCipher(string text, int key) {
    string result = "";
    
    for (char c : text) {
        if (isalpha(c)) { 
            char base = isupper(c) ? 'A' : 'a';  
            c = (c - base + key) % 26 + base;  
        }
        result += c; 
    }
    return result;
}

int main() {
    string message;
    int key;
    cout << "60009220195 Devansh Jollani";

    cout << "Enter the plaintext: ";
    getline(cin, message);  

    cout << "Enter the key value(shift value): ";
    cin >> key;

    key = (key % 26 + 26) % 26; 

    string encrypted = caesarCipher(message, key);
    cout << "Ciphertext is: " << encrypted << endl;

    return 0;
}

