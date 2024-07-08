#include "pch.hpp"
#include "data.hpp"
using namespace UI::Data;

std::vector<double> Characteristic::getLogCurrent()
{
    auto logf = [](double I)
    { return std::log(std::abs(I)); };
    std::vector<double> logI{I};
    std::transform(I.begin(), I.end(), logI.begin(), logf);
    return logI;
}

bool UI::Data::checkExistence(const std::vector<Characteristic> &destination, const Characteristic &item)
{
    auto begin = destination.begin();
    auto end = destination.end();
    bool checked = std::find(begin, end, item) == end;
    return !checked;
}

void Characteristic::resize(int value)
{
    V.resize(value);
    I.resize(value);
    J.resize(value);
}

json Characteristic::createJSONObject(std::string &path)
{
    const char *fileName = path.c_str();
    std::ifstream file(fileName);
    std::string jsonString;
    std::string line;
    while (std::getline(file, line))
        jsonString += line;
    return json::parse(jsonString, nullptr, false);
}

void Characteristic::to_json(json &j, const Characteristic characteristic)
{

    j = json{{"voltage", characteristic.V}, {"current", characteristic.I}, {"density current", characteristic.J}};
}

void Characteristic::from_json(const json &j, Characteristic &iv)
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

double Characteristic::read_temperature(std::string &path)
{

    std::stringstream ss(m_path.string());
    std::string token;
    std::vector<std::string> slices;
    char delimiter = '_';
    while (std::getline(ss, token, delimiter))
        slices.push_back(token);

    for (const auto &item : slices)
    {
        if (item[0] == 'T')
            return std::stod(item.substr(1));
        else
            continue;
    };
}

void PlotData::addCharacteristic(Characteristic &item)
{
    if (!checkExistence(characteristics, item))
        characteristics.push_back(item);
    item.selected = true;
}

void PlotData::removeCharacteristic(Characteristic &item)
{
    const auto &begin = characteristics.begin();
    const auto &end = characteristics.end();
    if (checkExistence(characteristics, item))
        characteristics.erase(std::remove(begin, end, item), end);
    item.selected = false;
}

void ContentBrowserData::readCharacteristic(const std::filesystem::path &path)
{
    Characteristic characteristic(path);
    if (!checkExistence(characteristics, characteristic))
        characteristics.push_back(characteristic);
}
