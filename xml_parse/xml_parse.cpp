#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <string>
#include <filesystem>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 	


using namespace std;


struct Coordinate {
    string ordNmb;
    string x;
    string y;
};


string getValueFromTag(const string& line, const string& tag) {
    string start_tag = "<" + tag + ">";
    string end_tag = "</" + tag + ">";
    size_t start_position = line.find(start_tag);
    size_t end_position = line.find(end_tag);
    if (start_position != string::npos && end_position != string::npos) {
        start_position += start_tag.length();
        return line.substr(start_position, end_position - start_position);
    }
    return "";
}

bool extractCoordinatesFromXML(const string& xml_file, const string& txt_file) {
    ifstream inputFile(xml_file);
    if (!inputFile.is_open()) {
        cerr << "Ошибка открытия XML файла\n";
        return false;
    }

    ofstream outputFile(txt_file);
    if (!outputFile.is_open()) {
        cerr << "Ошибка открытия или создания TXT файла\n";
        inputFile.close();
        return false;
    }

    vector<string> spatial_elements;
    string line;
    while (getline(inputFile, line)) {
        if (line.find("<spatial_element>") != string::npos) {
            spatial_elements.push_back(line);
        }
    }

    for (const auto& spatialElement : spatial_elements) {
        vector<Coordinate> coordinates;

        size_t start_position = spatialElement.find("<ordinate>");
        size_t end_position = spatialElement.find("</ordinate>", start_position);

        while (start_position != string::npos && end_position != string::npos) {
            string o_data = spatialElement.substr(start_position, end_position - start_position + 11);
            vector<string> ordinate_lines;
            istringstream iss(o_data);
            string ordinate_line;

            while (getline(iss, ordinate_line)) {
                ordinate_lines.push_back(ordinate_line);
            }

            Coordinate coordinate;
            for (const auto& ordinateLine : ordinate_lines) {
                string xValue = getValueFromTag(ordinateLine, "x");
                string yValue = getValueFromTag(ordinateLine, "y");
                if (!xValue.empty()) {
                    coordinate.x = xValue;
                }
                if (!yValue.empty()) {
                    coordinate.y = yValue;
                }
            }

            string ord_nmb_value = getValueFromTag(o_data, "ord_nmb");
            if (!ord_nmb_value.empty()) {
                coordinate.ordNmb = ord_nmb_value;
            }

            coordinates.push_back(coordinate);

            start_position = spatialElement.find("<ordinate>", end_position);
            end_position = spatialElement.find("</ordinate>", start_position);
        }

        for (const auto& coordinate : coordinates) {
            outputFile << coordinate.ordNmb << ' ' << coordinate.x << '\t' << coordinate.y << '\n';
        }

    }

    inputFile.close();
    outputFile.close();
}

void splitSections(const string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);

    outputFile << "1 spatial_element" << endl;
    string line;
    vector<string> sections;
    stringstream currentSection;

    int index_spatial_element = 2;

    bool is_new_section = true;

    while (getline(inputFile, line)) {

        if (line.empty()) {
            continue;
        }

        currentSection << line << '\n';

        if (line.find("1 ") == 0) {
            if (!is_new_section) {
                sections.push_back(currentSection.str());
                currentSection.str("");
            }
            is_new_section = true;
        }
        else {
            is_new_section = false;
        }
    }

    if (!currentSection.str().empty()) {
        sections.push_back(currentSection.str());
    }

    for (size_t i = 0; i < sections.size(); ++i) {
        outputFile << sections[i];
        if (i < sections.size() - 1) {
            outputFile << '\n' << to_string(index_spatial_element) << " spatial_element" << '\n';
            index_spatial_element++;
        }

    }

    inputFile.close();
    outputFile.close();

    cout << "Извлеченные координаты сохранены в " << outputFileName << endl;
}

vector<string> getXmlFilesInDirectory(const string& directoryPath) {
    vector<string> xmlFiles;

    if (!filesystem::exists(directoryPath) || !filesystem::is_directory(directoryPath)) {
        cout << "Указанная директория не существует или не является директорией." << endl;
        return xmlFiles;
    }

    for (const auto& entry : filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".xml") {
            xmlFiles.push_back(entry.path().string());
        }
    }

    return xmlFiles;
}


int main() {
    setlocale(LC_ALL, "rus");
    string flag ;
    cout << "Если хотите обработать 1 файл XML, введите 0. Если директорию, то 1: ";
    cin >> flag;
    if (flag == "0") {
        string xmlFile = "";
        cout << "Введите путь к xml(без кавычек): ";
        cin >> xmlFile;

        string txtFile_tmp = "output_tmp.txt";
        string txtFile = xmlFile.substr(0, xmlFile.size() - 4) + ".txt";

        if (extractCoordinatesFromXML(xmlFile, txtFile_tmp)) {
            splitSections(txtFile_tmp, txtFile);
        }
        remove("output_tmp.txt");
        system("pause > 0");
    }

    if (flag == "1") {
        string directory_path = "";
        cout << endl << "Введите путь к директории без кавычек ";
        cin >> directory_path;
        vector<string> xmlFiles = getXmlFilesInDirectory(directory_path);
        string output_txt_file_tmp = "output_tmp.txt";
        for (const auto& file : xmlFiles) {
            if (extractCoordinatesFromXML(file, output_txt_file_tmp)) {
                splitSections(output_txt_file_tmp, file.substr(0, file.length() - 4) + ".txt");
            }
            remove("output_tmp.txt");
        }
        
        system("pause > 0");
    }
    if (flag != "0" || flag != "0") {
        cerr <<endl<< "Ошибка";
        system("pause > 0");
        return 1;
    }
    return 0;
}