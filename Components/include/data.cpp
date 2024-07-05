#include "data.hpp"

void UI::Data::PlotData::addCharacteristic(std::filesystem::path path)
{

    Characteristic characteristic(path);
    auto begin = m_characteristics.begin();
    auto end = m_characteristics.end();
    if ((std::find(begin, end, characteristic)) == end)
        m_characteristics.push_back(characteristic);
}