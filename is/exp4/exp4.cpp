#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>

// --- Forward Declarations ---
void print_map(const std::map<char, char>& m);
std::pair<std::map<char, char>, std::map<char, char>> generate_substitution_key(std::string keyword);
std::pair<std::string, std::string> sanitize_text_keep_original(const std::string& plaintext);
std::string substitution_encrypt(const std::string& text, const std::map<char, char>& key_map);
std::string substitution_decrypt(const std::string& text, const std::map<char, char>& rev_map);
std::string transpose_encrypt(std::string text, const std::vector<int>& key);
std::string transpose_decrypt(const std::string& text, const std::vector<int>& key);
std::string reinsert_spacing(const std::string& original_text, const std::string& continuous_text);

// --- Main Program ---
int main() {
    // Hardcoded transposition key (same as Python example)
    std::vector<int> transposition_key = {3, 1, 4, 2};
    std::string keyword;
    std::string plaintext_input;

    std::cout << "Enter a keyword for substitution key: ";
    std::getline(std::cin, keyword);

    std::cout << "Enter plaintext: ";
    std::getline(std::cin, plaintext_input);

    // 1. Prepare keys and text
    auto [sub_key, rev_sub_key] = generate_substitution_key(keyword);
    auto [original_plain, cleaned_plain] = sanitize_text_keep_original(plaintext_input);

    // 2. Encryption (on cleaned text)
    std::string substituted = substitution_encrypt(cleaned_plain, sub_key);
    std::string ciphertext_continuous = transpose_encrypt(substituted, transposition_key);
    std::string ciphertext_spaced = reinsert_spacing(original_plain, ciphertext_continuous);

    // 3. Decryption (on continuous ciphertext)
    std::string rev_transposed = transpose_decrypt(ciphertext_continuous, transposition_key);
    std::string decrypted = substitution_decrypt(rev_transposed, rev_sub_key);
    
    // 4. Trim decrypted text to original length to remove padding
    std::string decrypted_trimmed = decrypted.substr(0, cleaned_plain.length());
    std::string decrypted_with_spacing = reinsert_spacing(original_plain, decrypted_trimmed);

    // --- Show results ---
    std::cout << "\n--- Results ---\n";
    std::cout << "Generated Substitution Key: ";
    print_map(sub_key);
    std::cout << "Original Plaintext (as entered):  " << original_plain << std::endl;
    std::cout << "Plaintext used for encryption   :  " << cleaned_plain << std::endl;
    std::cout << "After Substitution (clean)      :  " << substituted << std::endl;
    std::cout << "Ciphertext (continuous)         :  " << ciphertext_continuous << std::endl;
    std::cout << "Ciphertext (with original spacing):  " << ciphertext_spaced << std::endl;
    std::cout << "Decrypted (cleaned, padded)     :  " << decrypted << std::endl;
    std::cout << "Decrypted (trimmed to original) :  " << decrypted_trimmed << std::endl;
    std::cout << "Decrypted (with original spacing):  " << decrypted_with_spacing << std::endl;

    return 0;
}

// --- Function Implementations ---

/**
 * @brief Generates substitution and reverse-substitution maps from a keyword.
 */
std::pair<std::map<char, char>, std::map<char, char>> generate_substitution_key(std::string keyword) {
    std::string cipher_alphabet = "";
    std::set<char> seen;

    std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
    for (char ch : keyword) {
        if (std::isalpha(ch) && seen.find(ch) == seen.end()) {
            cipher_alphabet += ch;
            seen.insert(ch);
        }
    }

    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        if (seen.find(ch) == seen.end()) {
            cipher_alphabet += ch;
        }
    }

    std::map<char, char> sub_key;
    std::map<char, char> rev_sub_key;
    char plain_char = 'A';
    for (char cipher_char : cipher_alphabet) {
        sub_key[plain_char] = cipher_char;
        rev_sub_key[cipher_char] = plain_char;
        plain_char++;
    }

    return {sub_key, rev_sub_key};
}

/**
 * @brief Cleans plaintext to keep only uppercase letters, but also returns the original.
 */
std::pair<std::string, std::string> sanitize_text_keep_original(const std::string& plaintext) {
    std::string cleaned = "";
    for (char ch : plaintext) {
        if (std::isalpha(ch)) {
            cleaned += std::toupper(ch);
        }
    }
    return {plaintext, cleaned};
}

/**
 * @brief Encrypts/decrypts text using a given substitution map.
 */
std::string substitution_encrypt(const std::string& text, const std::map<char, char>& key_map) {
    std::string result = "";
    for (char ch : text) {
        // .count checks if the key exists in the map
        if (key_map.count(ch)) {
            result += key_map.at(ch);
        } else {
            result += ch; // Append character as-is if not in map (e.g., padding)
        }
    }
    return result;
}

std::string substitution_decrypt(const std::string& text, const std::map<char, char>& rev_map) {
    return substitution_encrypt(text, rev_map); // The logic is identical, just uses a different map
}

/**
 * @brief Encrypts text using a columnar transposition cipher.
 */
std::string transpose_encrypt(std::string text, const std::vector<int>& key) {
    int block_size = key.size();
    if (block_size == 0) return text;
    
    int padding = (block_size - (text.length() % block_size)) % block_size;
    text.append(padding, 'X');

    std::string result = "";
    for (size_t i = 0; i < text.length(); i += block_size) {
        std::string block = text.substr(i, block_size);
        for (int j = 0; j < block_size; ++j) {
            // Key is 1-based, so subtract 1 for 0-based C++ indexing
            result += block[key[j] - 1];
        }
    }
    return result;
}

/**
 * @brief Decrypts text from a columnar transposition cipher.
 */
std::string transpose_decrypt(const std::string& text, const std::vector<int>& key) {
    int block_size = key.size();
    if (block_size == 0) return text;

    std::string result = "";
    for (size_t i = 0; i < text.length(); i += block_size) {
        std::string block = text.substr(i, block_size);
        std::string temp(block_size, ' ');
        for (int j = 0; j < block_size; ++j) {
            // Key is 1-based, so subtract 1
            temp[key[j] - 1] = block[j];
        }
        result += temp;
    }
    return result;
}

/**
 * @brief Reinserts spaces and punctuation from an original text into a continuous text.
 */
std::string reinsert_spacing(const std::string& original_text, const std::string& continuous_text) {
    std::string result = "";
    auto continuous_iter = continuous_text.begin();
    
    for (char original_char : original_text) {
        if (std::isalpha(original_char)) {
            if (continuous_iter != continuous_text.end()) {
                result += *continuous_iter;
                ++continuous_iter;
            } else {
                result += 'X'; // Fallback if continuous text is shorter
            }
        } else {
            result += original_char; // Preserve space/punctuation
        }
    }
    // Append any leftover characters from continuous text (i.e., padding)
    result.append(continuous_iter, continuous_text.end());
    return result;
}

/**
 * @brief Helper function to print a map for verification.
 */
void print_map(const std::map<char, char>& m) {
    std::cout << "{";
    auto it = m.begin();
    while (it != m.end()) {
        std::cout << "'" << it->first << "': '" << it->second << "'";
        if (++it != m.end()) {
            std::cout << ", ";
        }
    }
    std::cout << "}\n";
}