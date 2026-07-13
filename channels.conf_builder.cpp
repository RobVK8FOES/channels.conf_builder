#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>

// Helper function to trim whitespace from strings
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper function to aggressively normalize strings
// Removes all whitespace and converts entirely to uppercase
std::string normalizeKey(const std::string& str) {
    std::string res;
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            res += std::toupper(static_cast<unsigned char>(c));
        }
    }
    return res;
}

// Helper function to split a string by a delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    // Ensure the user provides at least one input and one output file path
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input1.ini> [input2.ini ...] <output_channels.conf>\n";
        return 1;
    }

    // The last argument is always the output file
    std::string outputFile = argv[argc - 1];

    // 1. Read the ENTIRE existing channels.conf into memory for a bulletproof global search
    std::string existingData = "";
    std::ifstream checkFile(outputFile);
    if (checkFile.is_open()) {
        std::stringstream buffer;
        buffer << checkFile.rdbuf();
        // Normalize the entire file contents immediately
        existingData = normalizeKey(buffer.str());
        checkFile.close();
    }

    // 2. Open output file for appending
    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for writing: " << outputFile << "\n";
        return 1;
    }

    // 3. Loop through all provided .ini files
    for (int i = 1; i < argc - 1; ++i) {
        std::string inputFile = argv[i];
        std::ifstream inFile(inputFile);
        
        if (!inFile.is_open()) {
            std::cerr << "Warning: Could not open input file: " << inputFile << ". Skipping to next file...\n";
            continue;
        }

        std::cout << "Processing: " << inputFile << "...\n";

        std::string line;
        std::string currentSection = "";
        std::string satPos = "";
        std::string satName = "";
        bool satHeaderWritten = false;

        while (std::getline(inFile, line)) {
            line = trim(line);
            if (line.empty()) continue;

            // Detect section headers like [SATTYPE] or [DVB]
            if (line.front() == '[' && line.back() == ']') {
                currentSection = line;
                // Reset the header flag when a new satellite section begins
                if (currentSection == "[SATTYPE]") {
                    satHeaderWritten = false; 
                }
                continue;
            }

            // Process [SATTYPE] section
            if (currentSection == "[SATTYPE]") {
                size_t eqPos = line.find('=');
                if (eqPos != std::string::npos) {
                    std::string key = trim(line.substr(0, eqPos));
                    std::string value = trim(line.substr(eqPos + 1));
                    
                    if (key == "1") {
                        satPos = value;
                        // Substitute trailing '0' with 'E' for the orbital slot
                        if (!satPos.empty() && satPos.back() == '0') {
                            satPos.back() = 'E';
                        }
                    }
                    else if (key == "2") {
                        satName = value;
                    }
                }
            } 
            // Process [DVB] section
            else if (currentSection == "[DVB]") {
                size_t eqPos = line.find('=');
                if (eqPos != std::string::npos) {
                    std::string key = trim(line.substr(0, eqPos));
                    std::string value = trim(line.substr(eqPos + 1));

                    // Skip the count line
                    if (key == "0") continue;

                    std::vector<std::string> parts = split(value, ',');
                    if (parts.size() >= 3) {
                        std::string freq = parts[0];
                        std::string pol = parts[1];
                        std::string sr = parts[2];

                        // Force polarity to uppercase to ensure the header matches properly
                        std::string polUpper = pol;
                        if (!polUpper.empty()) polUpper[0] = std::toupper(polUpper[0]);

                        std::string header = "[" + freq + "-" + polUpper + "-" + sr + "]";
                        std::string satComment = "# " + satPos + " " + satName;
                        
                        // Create the highly robust transponder search key
                        std::string transKey = normalizeKey(header);

                        // Search the entire normalized file string. If not found, write it out.
                        if (existingData.find(transKey) == std::string::npos) {
                            
                            // Write two line breaks before the satellite comment
                            if (!satHeaderWritten) {
                                outFile << "\n\n" << satComment << "\n\n"; 
                                satHeaderWritten = true;
                            }

                            std::string fullPol = "UNKNOWN";
                            if (polUpper == "V") fullPol = "VERTICAL";
                            else if (polUpper == "H") fullPol = "HORIZONTAL";

                            // Format and append to channels.conf
                            outFile << header << "\n";
                            outFile << "    DELIVERY_SYSTEM = DVBS2\n";
                            outFile << "    FREQUENCY       = " << freq << "000\n";
                            outFile << "    SYMBOL_RATE     = " << sr << "000\n";
                            outFile << "    POLARIZATION    = " << fullPol << "\n\n";

                            // Add to the in-memory string to prevent duplicates within the same run
                            existingData += transKey;
                        }
                    }
                }
            }
        }
        inFile.close();
    }

    outFile.close();

    std::cout << "All files processed successfully. Output appended to " << outputFile << "\n";
    return 0;
}
