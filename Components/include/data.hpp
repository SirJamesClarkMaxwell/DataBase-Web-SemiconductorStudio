#pragma once

#include "pch.hpp"
#include "imgui.h"
#include "implot.h"
namespace UI::Data
{
	using json = nlohmann::json;
	class Characteristic;

	void from_json(const json &jsonObject, Characteristic &characteristic);
	void to_json(json &jsonObject, Characteristic &characteristic);

	class Characteristic
	{
	public:
		Characteristic(std::filesystem::path path)
			: m_path(path) { readData(); };
		void readData()
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
		};
		std::vector<double> getVoltage() { return V; };
		std::vector<double> getCurrent() { return I; };
		std::vector<double> getLogCurrent();
		std::vector<double> getDensityCurrent() { return J; };
		double getTemperature() { return m_temperature; };
		bool comparePath(const Characteristic &other) { return this->m_path == other.m_path; };
		bool operator==(const Characteristic &other) const { return this->m_temperature == other.m_temperature; };
		bool operator!=(const Characteristic &other) const { return this->m_temperature != other.m_temperature; };
		bool operator<(const Characteristic &other) const { return this->m_temperature < other.m_temperature; };
		bool selected{true};
		std::string name;

	private:
		void resize(int value);
		json createJSONObject(std::string &path);
		void to_json(json &j, const Characteristic characteristic);
		void from_json(const json &j, Characteristic &iv);
		double read_temperature(std::string &path);

	private:
		std::filesystem::path m_path;
		std::vector<double> V{};
		std::vector<double> I{};
		std::vector<double> J{};
		double m_temperature{};
	};
	bool checkExistence(const std::vector<Characteristic> &destination, const Characteristic &item);

	struct PlotProperties
	{

		bool lin_x_scale = true;
		bool lin_y_scale = false;
		std::pair<std::string, std::string> axis{"V", "I"};
		// ImGuiPlotFlags plotFlags
	};
	struct PlotData
	{
		PlotData(){};
		PlotProperties plotProperties{};
		void addCharacteristic(Characteristic &item);
		void removeCharacteristic(Characteristic &item);
		Characteristic operator[](int index) { return characteristics[index]; };
		int numberOfCharacteristics = 0;
		std::vector<Characteristic> characteristics{};
	};

	struct ContentBrowserData
	{
		ContentBrowserData() = default;
		std::vector<Characteristic> characteristics{};
		void readCharacteristic(const std::filesystem::path &path);
	};

	class DataPreview
	{
	public:
		DataPreview(){};
		// private:
		std::filesystem::path currentPath = "./../Characteristics/";
		std::filesystem::path rootPath = "./../Characteristics/";
		PlotData plotData;
		ContentBrowserData contentBrowserData;
	};

	struct MyData
	{
	public:
		MyData() = default;
		DataPreview dataPreview{};
	};
}