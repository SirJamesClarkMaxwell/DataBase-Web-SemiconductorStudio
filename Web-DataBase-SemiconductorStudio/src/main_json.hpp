#include <iostream>
#include <fstream>
#include "json.hpp"
#include <string>

using json = nlohmann::json;
namespace ns
{

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
} // namespace ns

void testingJSON()
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