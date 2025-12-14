#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Simple XOR encryption/decryption
std::string xor_encrypt_decrypt(const std::string& input, char key = 'K') {
    std::string output = input;
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = input[i] ^ key;
    }
    return output;
}

// Store encrypted file
void store_file() {
    std::string country, state, district, description;

    std::cin.ignore();
    std::cout << "Enter Country: ";
    std::getline(std::cin, country);

    std::cout << "Enter State: ";
    std::getline(std::cin, state);

    std::cout << "Enter District: ";
    std::getline(std::cin, district);

    std::cout << "Enter Description: ";
    std::getline(std::cin, description);

    std::string folder = "./data/" + country + "/" + state + "/" + district;

    try {
        fs::create_directories(folder);
    } catch (...) {
        std::cout << "Error creating directories!\n";
        return;
    }

    std::string file_path = folder + "/info.txt";

    
    std::string content =
        "Country: " + country + "\n" +
        "State: " + state + "\n" +
        "District: " + district + "\n" +
        "Description: " + description + "\n";

    // Encrypt content
    std::string encrypted = xor_encrypt_decrypt(content);

    // Save encrypted file
    std::ofstream file(file_path, std::ios::binary);
    if (!file) {
        std::cout << "Error writing file!\n";
        return;
    }
    file << encrypted;
    file.close();

    std::cout << "\nFile saved at: " << file_path << "\n";
}

// Retrieve
void retrieve_file() {
    std::cin.ignore();
    std::string country, state, district;

    std::cout << "Enter Country: ";
    std::getline(std::cin, country);

    std::cout << "Enter State: ";
    std::getline(std::cin, state);

    std::cout << "Enter District: ";
    std::getline(std::cin, district);

    std::string file_path = "./data/" + country + "/" + state + "/" + district + "/info.txt";

    if (!fs::exists(file_path)) {
        std::cout << "File not found!\n";
        return;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cout << "Error reading file!\n";
        return;
    }

    std::string encrypted((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string decrypted = xor_encrypt_decrypt(encrypted);

    std::cout << "\n---- Decrypted File Content ----\n";
    std::cout << decrypted << "\n";
}

int main() {
    int choice;

    while (true) {
        std::cout << "\n===== ENCRYPTED LOCATION STORAGE SYSTEM =====\n";
        std::cout << "1. Store a new file\n";
        std::cout << "2. Retrieve and view a file\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        if (choice == 1) {
            store_file();
        } else if (choice == 2) {
            retrieve_file();
        } else if (choice == 3) {
            break;
        } else {
            std::cout << "Invalid choice! Try again.\n";
        }
    }

    return 0;
}
