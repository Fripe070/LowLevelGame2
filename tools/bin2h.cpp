#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

std::string sanitizeVarName(const std::string& name) {
    std::string result;
    for (const unsigned char ch : name) {
        if (std::isalnum(ch) || ch == '_')
            result += static_cast<char>(std::toupper(ch));
        else
            result += '_';
    }
    return "BIN_" + result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Invalid number of arguments.\n"
                  << "Usage: " << argv[0] << " input.bin output.h\n";
        return 1;
    }

    std::filesystem::path inputPath{argv[1]};
    std::filesystem::path outputPath{argv[2]};

    std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
    if (!inFile) {
        std::perror(("Error opening input file: " + inputPath.string()).c_str());
        return 1;
    }

    // Get file size and allocate buffer
    std::streamsize fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    std::vector<unsigned char> data(fileSize);

    if (!inFile.read(reinterpret_cast<char*>(data.data()), fileSize)) {
        std::cerr << "Error reading input file: " << inputPath << "\n";
        return 1;
    }

    std::ofstream outFile(outputPath);
    if (!outFile) {
        std::perror(("Error opening output file: " + outputPath.string()).c_str());
        return 1;
    }

    std::string varName = sanitizeVarName(outputPath.stem().string());

    outFile << "#pragma once\n"
            << "#include <array>\n"
            << "constexpr std::array<unsigned char, " << fileSize << "> "
            << varName << " = {\n    ";

    // Write each byte as a hex literal, 12 bytes per line.
    outFile << std::hex << std::uppercase << std::setfill('0');
    for (size_t i = 0; i < static_cast<size_t>(fileSize); ++i) {
        outFile << "0x" << std::setw(2) << static_cast<int>(data[i]);
        if (i != static_cast<size_t>(fileSize) - 1) {
            outFile << ", ";
        }
        if ((i + 1) % 12 == 0 && i != static_cast<size_t>(fileSize) - 1) {
            outFile << "\n    ";
        }
    }
    outFile << "\n};\n";

    return 0;
}
