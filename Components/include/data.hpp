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
		Characteristic() = default;
		Characteristic(std::filesystem::path path)
			: m_path(path) { readData(); };
		void readData();
		std::vector<double> getVoltage() { return V; };
		std::vector<double> getCurrent() { return I; };
		std::vector<double> getLogCurrent();
		std::vector<double> getDensityCurrent() { return J; };
		double getTemperature() { return m_temperature; };
		const double getTemperature() const { return m_temperature; };
		bool comparePath(const Characteristic &other) { return this->m_path == other.m_path; };
		bool operator==(const Characteristic &other) const { return this->m_temperature == other.m_temperature; };
		bool operator!=(const Characteristic &other) const { return this->m_temperature != other.m_temperature; };
		bool operator<(const Characteristic &other) const { return this->m_temperature < other.m_temperature; };
		operator bool() { return selected; };
		bool selected{true};

		Characteristic(const Characteristic &) = default;
		Characteristic &operator=(const Characteristic &) = default;

		Characteristic(Characteristic &&) = default;
		Characteristic &operator=(Characteristic &&) = default;
		std::string name;
		// ImVec4 m_color;
		ImColor m_color;

		// void setColor(const ImVec4 &color) { m_color = color; };

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
	bool checkExistence(const std::vector<Characteristic> &destination, const std::string &item);

	struct PlotProperties
	{

		bool lin_x_scale = true;
		bool lin_y_scale = false;
		std::pair<std::string, std::string> axis{"V", "I"};
		std::vector<ImColor> colors;
		std::vector<ImU32> colorMap;
		ImU32 *colorMapPointer;
		int customRGMMap;
		// ImGuiPlotFlags plotFlags
	};
	struct PlotData
	{
		PlotData(){};
		PlotProperties plotProperties{};
		void addCharacteristic(Characteristic &item);
		void removeCharacteristic(Characteristic &item);
		Characteristic &operator[](const std::string &name);
		void setColorsOfGraph();
		void setColorsOfCharacteristics();
		int numberOfCharacteristics = 0;
		std::vector<Characteristic> characteristics{};
		ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
								ImGuiTableFlags_Sortable |
								ImGuiTableFlags_ScrollY;
		ImGuiTableFlags baseFlags = ImGuiTabBarFlags_None;
		ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB;
		ImColor startColor{1, 0, 0};
		ImColor endColor{0, 0, 1};
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