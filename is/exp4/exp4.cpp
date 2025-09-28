#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::remove, std::unique

// A structure to hold the coordinates (row, col) of a character in the key table.
struct Point {
    int row;
    int col;
};

// Function to generate the 5x5 key table from a keyword.
void generateKeyTable(const std::string& key, char keyTable[5][5]) {
    // A boolean array to keep track of letters already added to the table.
    // 'a' corresponds to index 0, 'b' to 1, and so on.
    bool used[26] = {false};
    used['j' - 'a'] = true; // We treat 'j' as 'i'.

    int i = 0, j = 0; // Current position (row, col) in the keyTable.

    // 1. Fill the table with the unique characters of the key.
    for (char c : key) {
        if (c == 'j') c = 'i'; // Treat 'j' as 'i'.

        if (!used[c - 'a']) {
            keyTable[i][j] = c;
            used[c - 'a'] = true;
            j++;
            if (j == 5) {
                i++;
                j = 0;
            }
        }
    }

    // 2. Fill the remaining cells with the rest of the alphabet.
    for (char c = 'a'; c <= 'z'; ++c) {
        if (!used[c - 'a']) {
            keyTable[i][j] = c;
            j++;
            if (j == 5) {
                i++;
                j = 0;
            }
        }
    }
}

// Function to find the position of a character in the key table.
Point findPosition(char c, const char keyTable[5][5]) {
    if (c == 'j') c = 'i'; // Treat 'j' as 'i'.

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (keyTable[i][j] == c) {
                return {i, j}; // Return the Point struct {row, col}.
            }
        }
    }
    return {-1, -1}; // Should not happen with valid input.
}


// Function to encrypt the plaintext.
std::string encrypt(const std::string& plaintext, const char keyTable[5][5]) {
    std::string ciphertext = "";
    for (size_t i = 0; i < plaintext.length(); i += 2) {
        char a = plaintext[i];
        char b = plaintext[i + 1];

        Point pos_a = findPosition(a, keyTable);
        Point pos_b = findPosition(b, keyTable);

        // Case 1: Same row
        if (pos_a.row == pos_b.row) {
            ciphertext += keyTable[pos_a.row][(pos_a.col + 1) % 5];
            ciphertext += keyTable[pos_b.row][(pos_b.col + 1) % 5];
        }
        // Case 2: Same column
        else if (pos_a.col == pos_b.col) {
            ciphertext += keyTable[(pos_a.row + 1) % 5][pos_a.col];
            ciphertext += keyTable[(pos_b.row + 1) % 5][pos_b.col];
        }
        // Case 3: Rectangle
        else {
            ciphertext += keyTable[pos_a.row][pos_b.col];
            ciphertext += keyTable[pos_b.row][pos_a.col];
        }
    }
    return ciphertext;
}

// Function to decrypt the ciphertext (reverse of encryption).
std::string decrypt(const std::string& ciphertext, const char keyTable[5][5]) {
    std::string plaintext = "";
    for (size_t i = 0; i < ciphertext.length(); i += 2) {
        char a = ciphertext[i];
        char b = ciphertext[i + 1];

        Point pos_a = findPosition(a, keyTable);
        Point pos_b = findPosition(b, keyTable);

        // Case 1: Same row (move left)
        if (pos_a.row == pos_b.row) {
            plaintext += keyTable[pos_a.row][(pos_a.col - 1 + 5) % 5];
            plaintext += keyTable[pos_b.row][(pos_b.col - 1 + 5) % 5];
        }
        // Case 2: Same column (move up)
        else if (pos_a.col == pos_b.col) {
            plaintext += keyTable[(pos_a.row - 1 + 5) % 5][pos_a.col];
            plaintext += keyTable[(pos_b.row - 1 + 5) % 5][pos_b.col];
        }
        // Case 3: Rectangle (same as encryption)
        else {
            plaintext += keyTable[pos_a.row][pos_b.col];
            plaintext += keyTable[pos_b.row][pos_a.col];
        }
    }
    return plaintext;
}


// A helper function to prepare the text for encryption.
std::string prepareText(std::string text) {
    // 1. Remove non-alphabetic characters and convert to lowercase.
    text.erase(std::remove_if(text.begin(), text.end(), [](char c) { return !isalpha(c); }), text.end());
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    
    // 2. Replace 'j' with 'i'.
    std::replace(text.begin(), text.end(), 'j', 'i');

    // 3. Insert 'x' between identical characters in a digraph.
    for (size_t i = 0; i < text.length(); i += 2) {
        if (i + 1 < text.length() && text[i] == text[i+1]) {
            text.insert(i + 1, "x");
        }
    }

    // 4. If the length is odd, append 'x'.
    if (text.length() % 2 != 0) {
        text += 'x';
    }
    
    return text;
}


int main() {
    std::string key, text;
    char keyTable[5][5];

    std::cout << "Enter the keyword: ";
    std::getline(std::cin, key);

    std::cout << "Enter the text to encrypt: ";
    std::getline(std::cin, text);

    // Prepare the key (lowercase, no spaces/duplicates).
    key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    // Generate the key table.
    generateKeyTable(key, keyTable);

    // Prepare the plaintext.
    std::string preparedText = prepareText(text);

    // Encrypt the text.
    std::string ciphertext = encrypt(preparedText, keyTable);

    // Decrypt the text.
    std::string decryptedText = decrypt(ciphertext, keyTable);

    std::cout << "\nResults:" << std::endl;
    std::cout << "Key Table:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            std::cout << keyTable[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\nOriginal Text:    " << text << std::endl;
    std::cout << "Prepared Text:    " << preparedText << std::endl;
    std::cout << "Encrypted Cipher: " << ciphertext << std::endl;
    std::cout << "Decrypted Text:   " << decryptedText << std::endl;

    return 0;
}