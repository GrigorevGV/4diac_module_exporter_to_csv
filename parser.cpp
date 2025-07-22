#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <set>

using namespace std;

struct Block {
    string name;
    string type;
};

struct Connection {
    string source;
    string destination;
    string type;
};

vector<Block> blocks;
vector<Connection> connections;

string extractBlockName(const string& line) {
    regex nameRegex("Name=\"([^\"]+)\"");
    smatch match;
    if (regex_search(line, match, nameRegex)) {
        return match[1];
    }
    return "";
}

string extractBlockType(const string& line) {
    regex typeRegex("Type=\"([^\"]+)\"");
    smatch match;
    if (regex_search(line, match, typeRegex)) {
        return match[1];
    }
    return "";
}

string extractBlockNameFromConnection(const string& endpoint) {
    vector<string> parts;
    size_t start = 0, end = 0;
    while ((end = endpoint.find('.', start)) != string::npos) {
        parts.push_back(endpoint.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(endpoint.substr(start));
    if (parts.size() >= 2) {
        return parts[parts.size() - 2];
    } else if (parts.size() == 1) {
        return parts[0];
    }
    return "";
}

vector<string> extractBlockAndPort(const string& endpoint) {
    size_t lastDot = endpoint.rfind('.');
    if (lastDot == string::npos) {
        return {endpoint, ""};
    }
    size_t prevDot = endpoint.rfind('.', lastDot - 1);
    string block, port;
    if (prevDot == string::npos) {
        block = endpoint.substr(0, lastDot);
        port = endpoint.substr(lastDot + 1);
    } else {
        block = endpoint.substr(prevDot + 1, lastDot - prevDot - 1);
        port = endpoint.substr(lastDot + 1);
    }
    return {block, port};
}

void parseConnections(const vector<string>& lines, const string& connectionType) {
    cout << "Parsing " << connectionType << " connections..." << endl;
    
    for (const string& line : lines) {
        if (line.find("<Connection") != string::npos) {
            regex sourceRegex("Source=\"([^\"]+)\"");
            regex destRegex("Destination=\"([^\"]+)\"");
            smatch sourceMatch, destMatch;
            
            if (regex_search(line, sourceMatch, sourceRegex) && 
                regex_search(line, destMatch, destRegex)) {
                Connection conn;
                conn.source = sourceMatch[1];
                conn.destination = destMatch[1];
                conn.type = connectionType;
                connections.push_back(conn);
                cout << "  Found connection: " << conn.source << " -> " << conn.destination << endl;
            }
        }
    }
}

void findBlockConnections(const string& blockName) {
    cout << "Block: " << blockName << endl;
    cout << "  Inputs:" << endl;
    for (const Connection& conn : connections) {
        if (conn.destination.find(blockName + ".") == 0) {
            string input = conn.destination.substr(blockName.length() + 1);
            cout << "    " << input << " <- " << conn.source << endl;
        }
    }
    cout << "  Outputs:" << endl;
    for (const Connection& conn : connections) {
        if (conn.source.find(blockName + ".") == 0) {
            string output = conn.source.substr(blockName.length() + 1);
            cout << "    " << output << " -> " << conn.destination << endl;
        }
    }
    cout << endl;
}

void printBlocks() {
    cout << "BLOCK LIST:" << endl << endl;
    for (const Block& block : blocks) {
        if (!block.name.empty()) {
            cout << "Block: " << block.name << endl;
        }
    }
    cout << endl;
}

void printConnections() {
    cout << "CONNECTIONS:" << endl << endl;
    for (const Connection& conn : connections) {
        cout << conn.source << " -> " << conn.destination << " (" << conn.type << ")" << endl;
    }
    cout << endl;
}

void printBlockDetails() {
    cout << "DETAILED BLOCK INFORMATION:" << endl << endl;
    vector<string> blockNames;
    for (const Block& block : blocks) {
        if (!block.name.empty() && std::find(blockNames.begin(), blockNames.end(), block.name) == blockNames.end()) {
            blockNames.push_back(block.name);
        }
    }
    vector<string> allNames = blockNames;
    for (const Connection& conn : connections) {
        string srcBlock = extractBlockNameFromConnection(conn.source);
        if (!srcBlock.empty() && std::find(allNames.begin(), allNames.end(), srcBlock) == allNames.end()) allNames.push_back(srcBlock);
        string dstBlock = extractBlockNameFromConnection(conn.destination);
        if (!dstBlock.empty() && std::find(allNames.begin(), allNames.end(), dstBlock) == allNames.end()) allNames.push_back(dstBlock);
    }
    for (const string& name : allNames) {
        cout << "Block: " << name << endl;
        cout << "  Inputs:" << endl;
        for (const Connection& conn : connections) {
            std::string dstBlock = extractBlockNameFromConnection(conn.destination);
            if (dstBlock == name) {
                auto srcParts = extractBlockAndPort(conn.source);
                auto dstParts = extractBlockAndPort(conn.destination);
                std::string srcBlock = srcParts[0];
                std::string srcPort = srcParts[1];
                std::string dstPort = dstParts[1];
                cout << "    " << dstPort << " <- " << srcBlock;
                if (!srcPort.empty()) cout << "." << srcPort;
                cout << endl;
            }
        }
        cout << "  Outputs:" << endl;
        for (const Connection& conn : connections) {
            std::string srcBlock = extractBlockNameFromConnection(conn.source);
            if (srcBlock == name) {
                auto srcParts = extractBlockAndPort(conn.source);
                auto dstParts = extractBlockAndPort(conn.destination);
                std::string srcPort = srcParts[1];
                std::string dstBlock = dstParts[0];
                std::string dstPort = dstParts[1];
                cout << "    " << srcPort << " -> " << dstBlock;
                if (!dstPort.empty()) cout << "." << dstPort;
                cout << endl;
            }
        }
        cout << endl;
    }
}

void parseFile(const string& filename) {
    cout << "Opening file: " << filename << endl;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: could not open file " << filename << endl;
        return;
    }
    string line;
    cout << "Reading file line by line..." << endl;
    while (getline(file, line)) {
        if (line.find("<FB") != string::npos || line.find("<SubApp") != string::npos) {
            Block block;
            block.name = extractBlockName(line);
            block.type = extractBlockType(line);
            if (!block.name.empty()) {
                blocks.push_back(block);
                cout << "Found block: " << block.name << " of type " << block.type << endl;
            }
        }
        if (line.find("<Connection") != string::npos) {
            regex sourceRegex("Source=\"([^\"]+)\"");
            regex destRegex("Destination=\"([^\"]+)\"");
            smatch sourceMatch, destMatch;
            if (regex_search(line, sourceMatch, sourceRegex) && 
                regex_search(line, destMatch, destRegex)) {
                Connection conn;
                conn.source = sourceMatch[1];
                conn.destination = destMatch[1];
                conn.type = "";
                connections.push_back(conn);
                cout << "  Found connection: " << conn.source << " -> " << conn.destination << endl;
            }
        }
    }
    file.close();
    cout << "File read. Blocks found: " << blocks.size() << endl;
}

void writeToCSV() {
    ofstream csvFile("blocks_and_connections.csv");
    if (!csvFile.is_open()) {
        cerr << "Error: could not create CSV file" << endl;
        return;
    }
    
    // Записываем BOM для корректного отображения в Excel
    csvFile << "\xEF\xBB\xBF";
    
    // Собираем все уникальные имена блоков
    set<string> blockNames;
    for (const Block& block : blocks) {
        if (!block.name.empty()) {
            blockNames.insert(block.name);
        }
    }
    
    // Добавляем блоки из связей
    for (const Connection& conn : connections) {
        string srcBlock = extractBlockNameFromConnection(conn.source);
        string dstBlock = extractBlockNameFromConnection(conn.destination);
        if (!srcBlock.empty()) blockNames.insert(srcBlock);
        if (!dstBlock.empty()) blockNames.insert(dstBlock);
    }
    
    // Для каждого блока записываем его входа и выхода
    for (const string& blockName : blockNames) {
        csvFile << blockName << endl;
        csvFile << "Source;Destination;Source;Destination" << endl;
        
        // Создаем векторы для хранения входов и выходов
        vector<pair<string, string>> inputConnections;  // (source, destination)
        vector<pair<string, string>> outputConnections; // (source, destination)
        
        // Находим все входа (destination содержит этот блок)
        for (const Connection& conn : connections) {
            string dstBlock = extractBlockNameFromConnection(conn.destination);
            if (dstBlock == blockName) {
                auto srcParts = extractBlockAndPort(conn.source);
                auto dstParts = extractBlockAndPort(conn.destination);
                string srcBlock = srcParts[0];
                string srcPort = srcParts[1];
                string dstPort = dstParts[1];
                
                if (!dstPort.empty()) {
                    inputConnections.push_back(make_pair(srcBlock + "." + srcPort, dstPort));
                }
            }
        }
        
        // Находим все выхода (source содержит этот блок)
        for (const Connection& conn : connections) {
            string srcBlock = extractBlockNameFromConnection(conn.source);
            if (srcBlock == blockName) {
                auto srcParts = extractBlockAndPort(conn.source);
                auto dstParts = extractBlockAndPort(conn.destination);
                string srcPort = srcParts[1];
                string dstBlock = dstParts[0];
                string dstPort = dstParts[1];
                
                if (!srcPort.empty()) {
                    outputConnections.push_back(make_pair(srcPort, dstBlock + "." + dstPort));
                }
            }
        }
        
        // Записываем данные, объединяя входы и выходы
        size_t maxSize = max(inputConnections.size(), outputConnections.size());
        for (size_t i = 0; i < maxSize; i++) {
            string inputSource = "", inputDest = "", outputSource = "", outputDest = "";
            
            if (i < inputConnections.size()) {
                inputSource = inputConnections[i].first;
                inputDest = inputConnections[i].second;
            }
            
            if (i < outputConnections.size()) {
                outputSource = outputConnections[i].first;
                outputDest = outputConnections[i].second;
            }
            
            csvFile << inputSource << ";" << inputDest << ";" << outputSource << ";" << outputDest << endl;
        }
        
        csvFile << endl; // Пустая строка между блоками
    }
    
    csvFile.close();
    cout << "CSV file 'blocks_and_connections.csv' created successfully!" << endl;
}

int main() {
    string filename;
    cout << "Enter path to file: ";
    cin >> filename;
    parseFile(filename);
    printBlocks();
    printConnections();
    printBlockDetails();
    writeToCSV();
    return 0;
} 
