#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <regex>
#include <algorithm>

class PythonToCppDecompiler {
private:
    std::string pythonCode;
    bool hasClasses = false;
    std::vector<std::string> definedFunctions;
    std::vector<std::string> definedClasses;
    int currentClassIndent = 0;
    bool inClassMethod = false;
    bool inMainGuard = false;
    
    std::map<std::string, std::string> typeMap = {
        {"int", "int"},
        {"str", "std::string"},
        {"float", "double"},
        {"list", "std::vector"},
        {"dict", "std::map"},
        {"bool", "bool"},
        {"tuple", "std::tuple"},
        {"set", "std::set"}
    };

    // Helper functions
    std::string indentation(int level) {
        return std::string(level * 4, ' ');
    }

    std::string processIndentation(const std::string& line) {
        int indent = 0;
        while (indent < line.length() && line[indent] == ' ') {
            indent++;
        }
        return std::string(indent / 4 * 4, ' ');
    }

    std::string convertPythonType(const std::string& pythonType) {
        auto it = typeMap.find(pythonType);
        return it != typeMap.end() ? it->second : "auto";
    }

    std::string convertStringFormatting(const std::string& line) {
        std::string result = line;
        std::regex fstringRegex("f\"([^\"]*)\\{([^}]*)\\}(.*)\"");
        std::smatch matches;
        
        while (std::regex_search(result, matches, fstringRegex)) {
            std::string before = matches[1].str();
            std::string var = matches[2].str();
            std::string after = matches[3].str();
            
            result = "std::cout << \"" + before + "\" << " + var + " << \"" + after + "\"";
        }
        
        return result;
    }

    std::string convertListComprehension(const std::string& line) {
        std::regex listCompRegex("\\[(.*?) for (.*?) in (.*?)\\]");
        std::smatch matches;
        if (std::regex_search(line, matches, listCompRegex)) {
            std::string expr = matches[1].str();
            std::string var = matches[2].str();
            std::string container = matches[3].str();
            
            return "([&]() {\n"
                   "    std::vector<auto> result;\n"
                   "    for(const auto& " + var + " : " + container + ") {\n"
                   "        result.push_back(" + expr + ");\n"
                   "    }\n"
                   "    return result;\n"
                   "})()";
        }
        return line;
    }

    std::string convertDictComprehension(const std::string& line) {
        std::regex dictCompRegex("\\{(.*?):(.*?) for (.*?) in (.*?)\\}");
        std::smatch matches;
        if (std::regex_search(line, matches, dictCompRegex)) {
            std::string key = matches[1].str();
            std::string value = matches[2].str();
            std::string var = matches[3].str();
            std::string container = matches[4].str();
            
            return "([&]() {\n"
                   "    std::map<std::string, auto> result;\n"
                   "    for(const auto& " + var + " : " + container + ") {\n"
                   "        result[" + key + "] = " + value + ";\n"
                   "    }\n"
                   "    return result;\n"
                   "})()";
        }
        return line;
    }

    std::string convertPythonFunction(const std::string& line) {
        std::string result = line;
        
        // Convert print function
        std::regex printRegex("print\\((.*)\\)");
        result = std::regex_replace(result, printRegex, "std::cout << $1 << std::endl");

        // Convert len function
        std::regex lenRegex("len\\((.*)\\)");
        result = std::regex_replace(result, lenRegex, "$1.size()");

        // Convert super() call
        std::regex superRegex("super\\(\\).__init__\\((.*)\\)");
        if (std::regex_search(result, superRegex)) {
            std::string baseClass = definedClasses[definedClasses.size() - 2];
            result = std::regex_replace(result, superRegex, baseClass + "::__init__($1)");
        }

        // Convert self.attribute to this->attribute
        std::regex selfRegex("self\\.");
        result = std::regex_replace(result, selfRegex, "this->");

        return result;
    }

    std::string convertPythonOperators(const std::string& line) {
        std::string result = line;
        
        // Convert Python's not to C++'s !
        std::regex notRegex("\\bnot\\b");
        result = std::regex_replace(result, notRegex, "!");

        // Convert and to &&
        std::regex andRegex("\\band\\b");
        result = std::regex_replace(result, andRegex, "&&");

        // Convert or to ||
        std::regex orRegex("\\bor\\b");
        result = std::regex_replace(result, orRegex, "||");

        // Convert is to ==
        std::regex isRegex("\\bis\\b");
        result = std::regex_replace(result, isRegex, "==");

        // Convert is not to !=
        std::regex isNotRegex("\\bis not\\b");
        result = std::regex_replace(result, isNotRegex, "!=");

        return result;
    }

    bool isClassDefinition(const std::string& line) {
        return line.find("class ") == 0;
    }

    bool isClassMethod(const std::string& line, int indent) {
        return indent > currentClassIndent && line.find("def ") == 0;
    }

    std::string convertClass(const std::string& line) {
        std::regex classRegex("class (\\w+)(?:\\((.*)\\))?:");
        std::smatch matches;
        if (std::regex_search(line, matches, classRegex)) {
            std::string className = matches[1].str();
            std::string inheritance = matches[2].str();
            definedClasses.push_back(className);
            
            std::string result = "class " + className;
            if (!inheritance.empty()) {
                result += " : public " + inheritance;
            }
            result += " {\npublic:";
            return result;
        }
        return line;
    }

    std::string convertClassMethod(const std::string& line) {
        std::regex methodRegex("def (\\w+)\\(self(?:,\\s*(.*))?\\):");
        std::smatch matches;
        if (std::regex_search(line, matches, methodRegex)) {
            std::string methodName = matches[1].str();
            std::string params = matches[2].str();
            inClassMethod = true;
            
            if (methodName == "__init__") {
                return definedClasses.back() + "(" + (params.empty() ? "" : params) + ") {";
            }
            
            return "auto " + methodName + "(" + (params.empty() ? "" : params) + ") {";
        }
        return line;
    }

    std::string handleExceptions(const std::string& line) {
        if (line.find("try:") == 0) {
            return "try {";
        }
        std::regex exceptRegex("except(?: (\\w+))?(?: as (\\w+))?:");
        std::smatch matches;
        if (std::regex_search(line, matches, exceptRegex)) {
            std::string exceptionType = matches[1].str();
            std::string exceptionVar = matches[2].str();
            
            if (exceptionType.empty()) {
                return "} catch (...) {";
            }
            
            return "} catch (const std::" + exceptionType + "& " + 
                   (exceptionVar.empty() ? "e" : exceptionVar) + ") {";
        }
        return line;
    }

    bool isMainGuard(const std::string& line) {
        return line.find("if __name__ == \"__main__\":") == 0;
    }

public:
    PythonToCppDecompiler(const std::string& code) : pythonCode(code) {}

    std::string decompile() {
        std::stringstream result;
        result << "#include <iostream>\n";
        result << "#include <string>\n";
        result << "#include <vector>\n";
        result << "#include <map>\n";
        result << "#include <set>\n";
        result << "#include <tuple>\n";
        result << "#include <stdexcept>\n";
        result << "#include <algorithm>\n\n";

        // First pass to collect functions and classes
        std::stringstream ss(pythonCode);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find("def ") == 0) {
                std::regex defRegex("def (\\w+)");
                std::smatch matches;
                if (std::regex_search(line, matches, defRegex)) {
                    definedFunctions.push_back(matches[1].str());
                }
            }
            else if (isClassDefinition(line)) {
                hasClasses = true;
            }
        }

        // Reset stream for second pass
        ss.clear();
        ss.str(pythonCode);

        int currentIndentLevel = 0;
        bool inClass = false;
        bool inFunction = false;
        std::vector<std::string> mainCode;

        while (std::getline(ss, line)) {
            if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
                result << "\n";
                continue;
            }

            std::string indent = processIndentation(line);
            int indentLevel = static_cast<int>(indent.length() / 4);
            line = line.substr(indent.length());

            // Skip comments
            if (line.find("#") == 0) {
                result << indentation(currentIndentLevel) << "//" << line.substr(1) << "\n";
                continue;
            }

            // Handle main guard
            if (isMainGuard(line)) {
                inMainGuard = true;
                continue;
            }
            
            // Store main code for later
            if (inMainGuard) {
                if (indentLevel > 0) {
                    mainCode.push_back(line);
                }
                continue;
            }
            
            // Handle class definitions
            if (isClassDefinition(line)) {
                line = convertClass(line);
                inClass = true;
                currentClassIndent = indentLevel;
            }
            // Handle class methods
            else if (isClassMethod(line, indentLevel)) {
                line = convertClassMethod(line);
            }
            // Convert basic Python constructs to C++
            else if (line.find("if ") == 0) {
                line = "if (" + line.substr(3) + ") {";
            }
            else if (line.find("else:") == 0) {
                line = "} else {";
            }
            else if (line.find("elif ") == 0) {
                line = "} else if (" + line.substr(5) + ") {";
            }
            else if (line.find("while ") == 0) {
                line = "while (" + line.substr(6) + ") {";
            }
            else if (line.find("for ") == 0) {
                std::regex forInRegex("for (.*) in (.*):");
                std::smatch matches;
                if (std::regex_search(line, matches, forInRegex)) {
                    std::string var = matches[1].str();
                    std::string container = matches[2].str();
                    
                    if (container.find("range(") == 0) {
                        std::regex rangeRegex("range\\((\\d+)(?:,\\s*(\\d+))?\\)");
                        std::smatch rangeMatches;
                        if (std::regex_search(container, rangeMatches, rangeRegex)) {
                            std::string start = rangeMatches[1].str();
                            std::string end = rangeMatches[2].str();
                            if (end.empty()) {
                                line = "for(int " + var + " = 0; " + var + " < " + start + "; ++" + var + ") {";
                            } else {
                                line = "for(int " + var + " = " + start + "; " + var + " < " + end + "; ++" + var + ") {";
                            }
                        }
                    } else {
                        line = "for(const auto& " + var + " : " + container + ") {";
                    }
                }
            }
            else if (line.find("def ") == 0 && !inClass) {
                std::regex defRegex("def (\\w+)\\((.*)\\):");
                std::smatch matches;
                if (std::regex_search(line, matches, defRegex)) {
                    std::string funcName = matches[1].str();
                    std::string params = matches[2].str();
                    line = "auto " + funcName + "(" + params + ") {";
                    inFunction = true;
                }
            }
            else if (line.find("return ") == 0) {
                line = "return " + line.substr(7) + ";";
            }
            else if (line.find("raise ") == 0) {
                std::regex raiseRegex("raise (\\w+)\\((.*)\\)");
                std::smatch matches;
                if (std::regex_search(line, matches, raiseRegex)) {
                    std::string exceptionType = matches[1].str();
                    std::string message = matches[2].str();
                    line = "throw std::" + exceptionType + "(" + message + ");";
                }
            }
            else if (line.find("try:") == 0 || line.find("except") == 0) {
                line = handleExceptions(line);
            }
            else if (line.find("pass") == 0) {
                line = "// pass";
            }
            else {
                line = convertPythonFunction(line);
                line = convertPythonOperators(line);
                line = convertStringFormatting(line);
                line = convertListComprehension(line);
                line = convertDictComprehension(line);
                if (!line.empty() && line.back() != '{' && line.find("//") != 0) {
                    line += ";";
                }
            }

            // Handle indentation and scope
            if (line.find("}") != std::string::npos) {
                currentIndentLevel--;
                if (inClass && currentIndentLevel == currentClassIndent) {
                    inClass = false;
                    line += ";";  // Class definition needs semicolon
                }
                if (inFunction && currentIndentLevel == (inClass ? currentClassIndent + 1 : 0)) {
                    inFunction = false;
                }
                if (inClassMethod && currentIndentLevel == currentClassIndent + 1) {
                    inClassMethod = false;
                }
            }

            result << indentation(currentIndentLevel) << line << "\n";

            if (line.find("{") != std::string::npos) {
                currentIndentLevel++;
            }
        }

        // Add main function with the collected main code
        if (!mainCode.empty()) {
            result << "\nint main() {\n";
            for (const auto& line : mainCode) {
                std::string processedLine = line;
                processedLine = convertPythonFunction(processedLine);
                processedLine = convertPythonOperators(processedLine);
                processedLine = convertStringFormatting(processedLine);
                processedLine = convertListComprehension(processedLine);
                processedLine = convertDictComprehension(processedLine);
                if (!processedLine.empty() && processedLine.back() != '{' && processedLine.find("//") != 0) {
                    processedLine += ";";
                }
                result << "    " << processedLine << "\n";
            }
            result << "    return 0;\n";
            result << "}\n";
        }
        else if (!hasClasses && definedFunctions.empty()) {
            result << "\nint main() {\n";
            result << "    return 0;\n";
            result << "}\n";
        }

        return result.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input_python_file> <output_cpp_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    // Read Python file
    std::ifstream pyFile(inputFile);
    if (!pyFile.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFile << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << pyFile.rdbuf();
    std::string pythonCode = buffer.str();
    pyFile.close();

    // Decompile Python to C++
    PythonToCppDecompiler decompiler(pythonCode);
    std::string cppCode = decompiler.decompile();

    // Write C++ file
    std::ofstream cppFile(outputFile);
    if (!cppFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFile << std::endl;
        return 1;
    }

    cppFile << cppCode;
    cppFile.close();

    std::cout << "Decompilation completed successfully!" << std::endl;
    return 0;
} 