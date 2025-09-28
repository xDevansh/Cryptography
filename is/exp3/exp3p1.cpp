#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <algorithm>

//Caesar Cipher Encrypt
std::string caesarEncrypt(const std::string& text, int key) {
    std::string result;
    for (char c : text) {
        if (isupper(c)) {
            result += char((c - 'A' + key) % 26 + 'A');
        } else if (islower(c)) {
            result += char((c - 'a' + key) % 26 + 'a');
        } else {
            result += c; 
        }
    }
    return result;
}

//Caesar Cipher Decrypt
std::string caesarDecrypt(const std::string& text, int key) {
    std::string result;
    for (char c : text) {
        if (isupper(c)) {
            result += char((c - 'A' - key + 26) % 26 + 'A');
        } else if (islower(c)) {
            result += char((c - 'a' - key + 26) % 26 + 'a');
        } else {
            result += c;
        }
    }
    return result;
}

//Load Dictionary tokenize
std::unordered_set<std::string> loadDictionary(const std::string& filename) {
    std::unordered_set<std::string> dict;
    std::ifstream file(filename);
    std::string word;
    while (file >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        dict.insert(word);
    }
    return dict;
}

//Score Decryption
//how many words are valid dictionary words
int scoreDecryption(const std::string& text, const std::unordered_set<std::string>& dict) {
    std::stringstream ss(text);
    std::string word;
    int score = 0;
    while (ss >> word) {
        
        word.erase(std::remove_if(word.begin(), word.end(),
                                  [](char c){ return ispunct(c); }), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (dict.count(word)) {
            score++;
        }
    }
    return score;
}

int main() {
    auto dict = loadDictionary("words.txt");
    std::cout << "60009220195 Devansh Jollani: ";


    std::string plaintext;
    std::cout << "Enter plaintext: ";
    std::getline(std::cin, plaintext);


    std::srand(std::time(0));
    int randomKey = 1 + std::rand() % 25; 

    //Encrypt plaintext
    std::string ciphertext = caesarEncrypt(plaintext, randomKey);

    //Show random key and ciphertext
    std::cout << "\n[Encryption]" << std::endl;
    std::cout << "Random Key Used: " << randomKey << std::endl;
    std::cout << "Ciphertext: " << ciphertext << std::endl;

    //Cryptanalysis using dictionary attack
    int bestKey = 0;
    int bestScore = -1;
    std::string bestDecryption;

    for (int key = 1; key < 26; key++) {
        std::string attempt = caesarDecrypt(ciphertext, key);
        int score = scoreDecryption(attempt, dict);
        if (score > bestScore) {
            bestScore = score;
            bestKey = key;
            bestDecryption = attempt;
        }
    }


    std::cout << "\n[Cryptanalysis result]" << std::endl;
    std::cout << "Predicted Key: " << bestKey << std::endl;
    std::cout << "Decrypted Text: " << bestDecryption << std::endl;

    return 0;
}
