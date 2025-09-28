#include <bits/stdc++.h>
using namespace std;

//Helpers 

//Normalize key
int normalizeKey(int k) {
    return (k % 26 + 26) % 26;
}

//Shift a single alphabetic char by key (0..25). Non-letters unchanged.
char shiftChar(char ch, int key) {
    unsigned char uch = static_cast<unsigned char>(ch);
    if (!isalpha(uch)) return ch;
    char base = isupper(uch) ? 'A' : 'a';
    int idx = (static_cast<int>(uch) - base + key) % 26;
    return static_cast<char>(base + idx);
}

//Apply Caesar transform (positive key = forward/encrypt)
string caesarTransform(const string &s, int key) {
    key = normalizeKey(key);
    string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(shiftChar(static_cast<char>(c), key));
    return out;
}

//decrypt with given encryption-key 
string decryptWithKey(const string &cipher, int k) {
    return caesarTransform(cipher, 26 - normalizeKey(k));
}

// Tokenization & dictionary

// Split text into lowercase words (keep letters and digits as part of words)
vector<string> splitWordsLower(const string &s) {
    vector<string> words;
    string cur;
    cur.reserve(16);
    for (unsigned char uc : s) {
        char c = static_cast<char>(tolower(uc));
        if (isalnum(static_cast<unsigned char>(c))) {
            cur.push_back(c);
        } else {
            if (!cur.empty()) { words.push_back(move(cur)); cur.clear(); }
        }
    }
    if (!cur.empty()) words.push_back(move(cur));
    return words;
}

// Load dictionary file (one word per line). Keep only alphabetic characters, lowercase.
unordered_set<string> loadDictFile(const string &path) {
    unordered_set<string> dict;
    ifstream in(path);
    if (!in) return dict;
    string w;
    while (in >> w) {
        string cleaned;
        cleaned.reserve(w.size());
        for (unsigned char uc : w) {
            char c = static_cast<char>(tolower(uc));
            if (isalpha(static_cast<unsigned char>(c))) cleaned.push_back(c);
        }
        if (!cleaned.empty()) dict.insert(cleaned);
    }
    return dict;
}

// small builtin fallback
unordered_set<string> tinyBuiltinDict() {
    return {"the","and","to","of","in","is","it","that","for","on","with","as",
            "this","be","are","or","not","you","he","she","they","we","have",
            "has","but","all","can","if","so","one","about","there","what","when",
            "where","who","how","why","which","hello","world","i","a","an"};
}

// Very common words for tie-break heuristic
const unordered_set<string> COMMON_WORDS = {
    "the","and","to","of","in","is","it","that","for","on","with","as"
};

// ------------------ Letter-frequency chi-square ------------------
// English expected frequencies (percent)
// source: standard English frequency table (stable for this use)
const double EN_FREQ_PERCENT[26] = {
    8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966,
    0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929, 0.095, 5.987,
    6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074
};

double chiSquareForText(const string &text) {
    array<int,26> counts{};
    int totalLetters = 0;
    for (unsigned char uc : text) {
        char c = static_cast<char>(tolower(uc));
        if (isalpha(static_cast<unsigned char>(c))) {
            counts[c - 'a']++;
            totalLetters++;
        }
    }
    if (totalLetters == 0) return 1e9; // no letters -> extremely bad

    double chi2 = 0.0;
    for (int i = 0; i < 26; ++i) {
        double expected = EN_FREQ_PERCENT[i] * totalLetters / 100.0;
        double observed = static_cast<double>(counts[i]);
        double diff = observed - expected;
        // if expected is 0 (shouldn't happen), skip
        if (expected > 0.0) chi2 += diff * diff / expected;
    }
    return chi2;
}

// ------------------ Scoring & candidate structure ------------------

struct Score {
    int matches = 0;        // dictionary word matches count
    int totalWords = 0;
    int commonHits = 0;     // hits among very common words
    double ratio = 0.0;     // matches / totalWords
    double chi2 = 1e9;      // lower is better
};

Score scorePlaintext(const string &pt, const unordered_set<string> &dict) {
    auto words = splitWordsLower(pt);
    Score s;
    s.totalWords = static_cast<int>(words.size());
    if (s.totalWords == 0) {
        s.chi2 = chiSquareForText(pt);
        return s;
    }
    for (const auto &w : words) {
        // dictionary contains only alphabetic words; if candidate has digits too, dictionary won't match.
        if (dict.find(w) != dict.end()) s.matches++;
        if (COMMON_WORDS.find(w) != COMMON_WORDS.end()) s.commonHits++;
    }
    s.ratio = static_cast<double>(s.matches) / s.totalWords;
    s.chi2 = chiSquareForText(pt);
    return s;
}

struct Candidate {
    int key = -1;
    string plaintext;
    Score score;
};

// ------------------ Cracker ------------------

// Primary selection: highest ratio. Tie-break: more commonHits, then more matches, then lower chi2 (closer to English).
Candidate pickBestCandidate(const string &cipher, const unordered_set<string> &dict) {
    Candidate best;
    double bestRatio = -1.0;
    for (int key = 0; key < 26; ++key) {
        string pt = decryptWithKey(cipher, key);
        Score sc = scorePlaintext(pt, dict);

        bool better = false;
        if (sc.ratio > bestRatio) better = true;
        else if (fabs(sc.ratio - bestRatio) < 1e-12) {
            if (sc.commonHits > best.score.commonHits) better = true;
            else if (sc.commonHits == best.score.commonHits) {
                if (sc.matches > best.score.matches) better = true;
                else if (sc.matches == best.score.matches) {
                    // prefer smaller chi2 (closer to english by letter frequency)
                    if (sc.chi2 < best.score.chi2) better = true;
                    else if (fabs(sc.chi2 - best.score.chi2) < 1e-9 && key < best.key) better = true;
                }
            }
        }

        if (better) {
            best.key = key;
            best.plaintext = move(pt);
            best.score = sc;
            bestRatio = sc.ratio;
        }
    }
    return best;
}

// ------------------ CLI ------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Path to SCOWL dictionary (press Enter to use built-in tiny dict):\n> ";
    string path;
    getline(cin, path);

    unordered_set<string> dict;
    if (!path.empty()) {
        dict = loadDictFile(path);
        if (dict.empty()) {
            cerr << "Warning: could not load dictionary or file empty. Using tiny builtin dictionary.\n";
            dict = tinyBuiltinDict();
        } else {
            cout << "Loaded dictionary with " << dict.size() << " words.\n";
        }
    } else {
        dict = tinyBuiltinDict();
        cout << "Using built-in tiny dictionary (" << dict.size() << " words).\n";
    }

    cout << "\nEnter ciphertext (one line). The program will try all 26 keys and auto-pick best.\n> ";
    string cipher;
    getline(cin, cipher);

    if (cipher.empty()) {
        cout << "No input provided. Exiting.\n";
        return 0;
    }

    // Show all 26 decryptions
    cout << "\nAll 26 decryptions (key = encryption shift):\n";
    for (int k = 0; k < 26; ++k) {
        cout << "Key " << setw(2) << k << ": " << decryptWithKey(cipher, k) << '\n';
    }

    // Auto-pick using dictionary scoring + chi-square tie breaking
    Candidate best = pickBestCandidate(cipher, dict);

    cout << "\nBest guess (auto-picked):\n";
    cout << "  Key (encryption shift): " << best.key << '\n';
    cout << "  Plaintext: " << best.plaintext << '\n';
    cout << "  Score: matches=" << best.score.matches
         << ", totalWords=" << best.score.totalWords
         << ", ratio=" << best.score.ratio
         << ", commonHits=" << best.score.commonHits
         << ", chi2=" << best.score.chi2 << '\n';

    return 0;
}
