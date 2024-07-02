#include <iostream>
#include <fstream>
#include "json.hpp"
#include <string>

using json = nlohmann::json;
namespace ns
{
    struct IV
    {
        void resize(int value)
        {
            V.resize(value);
            I.resize(value);
            J.resize(value);
        };
        std::vector<double> V;
        std::vector<double> I;
        std::vector<double> J;
    };
    struct person
    {
        std::string name;
        std::string address;
        int age;
    };
    void to_json(json &j, const person &p)
    {
        j = json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
    }
    void from_json(const json &j, person &p)
    {
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
    }

    void to_json(json &j, const IV &iv)
    {
        j = json{{"voltage", iv.V}, {"current", iv.I}, {"density current", iv.J}};
    }

    void from_json(const json &j, IV &iv)
    {

        auto strToDouble = [](const std::string &str)
        { return std::stod(str); };
        auto transform = [&](const std::vector<std::string> &str, std::vector<double> &vec)
        {
            std::transform(str.begin(), str.end(), vec.begin(), strToDouble);
        };
        iv.resize(j.at("voltage").size());

        transform(j.at("voltage"), iv.V);
        transform(j.at("current"), iv.I);
        transform(j.at("density current"), iv.J);
    }

} // namespace ns

void testingSImpleJSONstructure();
void testingIVJSON();
void testingJSON()
{
    // testingSImpleJSONstructure();
    testingIVJSON();
}
void testingSImpleJSONstructure()
{
    ns::person p = {"Ned Flanders", "744 Evergreen Terrace", 60};

    json j;
    // j["name"] = p.name;
    // j["address"] = p.address;
    // j["age"] = p.age;

    const char *filePath = "C:/Users/Franek/Desktop/STUDIA/SemiconductorStudio-All/DataBase-Web-SemiconductorStudio/Web-DataBase-SemiconductorStudio/src/person.json";
    std::ofstream file{filePath};
    ns::to_json(j, p);
    std::string text = j.dump(4);
    file << text;
    file.close();

    std::ifstream filef{filePath};

    // Check if the file was opened successfully
    if (!filef.is_open())
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }

    // Read and print the contents of the file
    std::string jsonString;
    std::string line;
    while (std::getline(filef, line))
    {
        jsonString += line;
    }
    std::cout << jsonString << std::endl;
    //     // Close the file
    filef.close();
    json j1 = json::parse(jsonString, nullptr, false);
    ns::person p1;
    ns::from_json(j1, p1);
    std::cout << p.address << " " << p1.age << " " << p1.name << std::endl;
};
void testingIVJSON()
{
    const char *fileName = "C:/Users/Franek/Desktop/STUDIA/SemiconductorStudio-All/DataBase-Web-SemiconductorStudio/Characteristics/JSON/ivd_HZB25_T181_L0.json";
    std::ifstream file(fileName);
    std::string jsonString;
    std::string line;
    while (std::getline(file, line))
        jsonString += line;
    // std::cout << jsonString << std::endl;
    json jsonObject = json::parse(jsonString, nullptr, false);
    ns::IV characteristic;
    ns::from_json(jsonObject, characteristic);
    for (auto &item : characteristic.V)
        std::cout << item << " ";
    std::cout << std::endl;
};
