#include <iostream>
#include <string>

void removeLastOccurrence(std::string &str, const std::string &word) {
    size_t pos = str.rfind(word);

    if (pos != std::string::npos) {
        // Erase the last occurrence of the word
        str.erase(pos, word.length());
    }
}

int main() {
    std::string myString = "This is a sample string. This string has a sample word.";

    std::cout << "Original String: " << myString << std::endl;

    std::string wordToRemove = "sample";
    removeLastOccurrence(myString, wordToRemove);

    std::cout << "String after removing last occurrence of '" << wordToRemove << "': " << myString << std::endl;

    return 0;
}