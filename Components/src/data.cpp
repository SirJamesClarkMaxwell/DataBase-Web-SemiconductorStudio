#include "pch.hpp"
#include "data.hpp"
using namespace UI::Data;

void UI::Data::Characteristic::readData()
{
    std::string pathString = m_path.string();
    m_temperature = read_temperature(pathString);
    name = m_path.filename().string();
    json readCharacteristic;
    try
    {
        readCharacteristic = createJSONObject(pathString);
        from_json(readCharacteristic, *this);
    }
    catch (const std::exception &e)
    {
    }
}

std::vector<double> Characteristic::getLogCurrent()
{
    auto logf = [](double I)
    { return std::log(std::abs(I)); };
    std::vector<double> logI{I};
    std::transform(I.begin(), I.end(), logI.begin(), logf);
    return logI;
}

bool UI::Data::checkExistence(const std::vector<Characteristic> &destination, const std::string &item)
{
    auto found = std::find_if(destination.begin(), destination.end(),
                              [&item](const Characteristic &element)
                              {
                                  return element.name == item;
                              });
    bool f = found != destination.end();
    return f;
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
    if (!checkExistence(characteristics, item.name))

        characteristics.push_back(item);

    item.selected = true;
}

void PlotData::removeCharacteristic(Characteristic &item)
{
    if (checkExistence(characteristics, item.name))
    {
        auto it = std::find(characteristics.begin(), characteristics.end(), item);
        if (it != characteristics.end())
            characteristics.erase(it);
    }
    item.selected = false;
}

Characteristic &UI::Data::PlotData::operator[](const std::string &name)
{

    auto it = std::find_if(characteristics.begin(), characteristics.end(),
                           [&name](const Characteristic &element)
                           {
                               return element.name == name;
                           });
    if (it != characteristics.end())
        return *it;
}

void UI::Data::PlotData::setColorsOfCharacteristics()
{
    ImColor colorStep = (endColor - startColor) / (characteristics.size() - 1);
    for (const auto &[index, item] : std::views::enumerate(characteristics))
        item.m_color = startColor + index * colorStep;
}

void UI::Data::PlotData::setColorsOfGraph()
{
    plotProperties.colors.resize(characteristics.size());
    std::transform(characteristics.begin(), characteristics.end(), plotProperties.colors.begin(), [&](const Characteristic &item)
                   { return item.m_color; });
}

void ContentBrowserData::readCharacteristic(const std::filesystem::path &path)
{
    Characteristic characteristic(path);
    if (!checkExistence(characteristics, characteristic.name))
        characteristics.push_back(characteristic);
}
