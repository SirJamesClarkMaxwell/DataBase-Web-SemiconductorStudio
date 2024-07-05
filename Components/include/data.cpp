#include "pch.hpp"
#include "data.hpp"

void UI::Data::PlotData::addCharacteristic(std::filesystem::path path)
{
    numberOfCharacteristics++;
    Characteristic characteristic;
    characteristic.readData(path);
    m_characteristics.push_back(characteristic);
}