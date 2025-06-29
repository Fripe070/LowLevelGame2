#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>


std::string fileToVarName(const std::string& fileName) {
    if (fileName.empty()) throw std::invalid_argument("File name must not be empty");
    std::string result;
    for (const unsigned char ch : fileName) {
        if (std::isalnum(ch))
            result += static_cast<char>(std::toupper(ch));
        else
            result += '_';
    }
    return "BINARY_" + result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Invalid number of arguments.\n"
                  << "Usage: " << argv[0] << " input_directory output_directory\n";
        return 1;
    }
    std::filesystem::path inputDir{argv[1]};
    std::filesystem::path outputDir{argv[2]};

    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (!entry.is_regular_file()) continue;

        std::filesystem::path inputPath = entry.path();
        std::filesystem::path outputPath = outputDir / (inputPath.filename().string() + ".h");

        std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate); // Ate to get size
        if (!inFile) {
            std::cerr << "Error opening input file: " << inputPath.string() << "\n";
            return 1;
        }

        std::streamsize fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);
        std::vector<unsigned char> data(fileSize);
        if (!inFile.read(reinterpret_cast<char*>(data.data()), fileSize)) {
            std::cerr << "Error reading input file: " << inputPath.string() << "\n";
            return 1;
        }

        std::ofstream outFile(outputPath, std::ios::trunc);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputPath.string() << "\n";
            return 1;
        }
        outFile << "#pragma once\n"
                << "#include <array>\n"
                << "constexpr std::array<unsigned char, " << fileSize << "> "
                << fileToVarName(inputPath.filename().string())
                << " = {\n    ";
        outFile << std::hex << std::uppercase << std::setfill('0');
        for (size_t i = 0; i < static_cast<size_t>(fileSize); ++i) {
            outFile << "0x" << std::setw(2) << static_cast<int>(data[i]);
            if (i != static_cast<size_t>(fileSize) - 1) {
                outFile << ",";
            }
        }
        outFile << "\n};\n";

        std::cout << "Transformed binary file " << inputPath.filename().string()
                  << " to header file " << outputPath.filename().string() << "\n";
    }
    std::cout << "All headers generated successfully.\n";

    return 0;
}
