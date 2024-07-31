#pragma once

#include "pch.hpp"
#include "imgui.h"
#include "implot.h"
#include <iostream>
#include <complex>
#include "TestingFittingAndMC.hpp"
// #include "LambertW.h"

namespace UI::Data
{
	using json = nlohmann::json;
	class Characteristic;
	struct FourParameters
	{
		FourParameters() = default;
		double A = 1;
		double I0 = std::pow(10, -19);
		double Rs = 10;
		double Rch = 1000;
		double Temperature = 300;
	};
	struct SixParameters
	{
		SixParameters() = default;
		double A = 1;
		double I0 = std::pow(10, -10);
		double Rs = 0.10;
		double Rch = 100000;
		double Rch2 = 0;
		double alpha = 0;
		double Temperature = 210;
	};
	void from_json(const json &jsonObject, Characteristic &characteristic);
	void to_json(json &jsonObject, Characteristic &characteristic);

	class Characteristic
	{
	public:
		Characteristic() = default;
		Characteristic(std::filesystem::path path)
			: m_path(path) { readData(); };
		void readData();
		std::vector<double> &getVoltage() { return V; };
		std::vector<double> &getCurrent() { return I; };
		std::vector<double> &getLogCurrent();
		std::vector<double> &getDensityCurrent() { return J; };
		double getTemperature() { return m_temperature; };
		const double getTemperature() const { return m_temperature; };
		bool comparePath(const Characteristic &other) { return this->m_path == other.m_path; };
		bool operator==(const Characteristic &other) const { return this->m_temperature == other.m_temperature; };
		bool operator!=(const Characteristic &other) const { return this->m_temperature != other.m_temperature; };
		bool operator<(const Characteristic &other) const { return this->m_temperature < other.m_temperature; };
		operator bool() { return selected; };
		bool selected{true};
		FourParameters fourParameters;
		SixParameters sixParameters;
		Characteristic(const Characteristic &) = default;
		Characteristic &operator=(const Characteristic &) = default;

		Characteristic(Characteristic &&) = default;
		Characteristic &operator=(Characteristic &&) = default;
		std::string name{"characteristic "};
		// ImVec4 m_color;
		ImVec4 m_color{1, 0, 0, 1};

		// void setColor(const ImVec4 &color) { m_color = color; };
		double m_temperature{};

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
		int addedColorMap{1};
		ImU32 name;
		// ImGuiPlotFlags plotFlags
	};
	struct PlotData
	{
		PlotData() {};
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
		DataPreview() {};
		// private:
		std::filesystem::path currentPath = "./../Characteristics/";
		std::filesystem::path rootPath = "./../Characteristics/";
		PlotData plotData;
		ContentBrowserData contentBrowserData;
	};

	struct GeneratingCharacteristicsPanelData
	{
		std::vector<Characteristic> characteristics;
		GeneratingCharacteristicsPanelData() = default;
		FourParameters fourParameters{};
		SixParameters sixParameters{};
		PlotProperties plotProperties;
		static void CalculateCurrent(Characteristic &characteristic, double A, double I0, double Rs, double Rch, double Temperature);
		static void CalculateCurrent(Characteristic &characteristic, double A, double I0, double Rs, double Rch, double Rch2, double alpha, double Temperature);
	};
	struct MyData
	{
	public:
		MyData() = default;
		DataPreview dataPreview{};
		GeneratingCharacteristicsPanelData generatingCharacteristicsData;
		// using  FittingTesting = JunctionFitMaster::FittingTesting;
		UI::Data::JunctionFitMasterUI::FittingTesting fittingTestingData;
	};

	std::vector<double> generate_range(double start, double end, double step);
	double diode_equation(double I, double V, double I_L, double I_0, double R_s, double R_sh, double n, double V_t);
	void current(double &V, double &I, double &I0, double &A, double &Rsch, double &Rs, double T);

}
namespace LamberWN
{

	using namespace std;
	complex<double> zexpz(complex<double> z);
	complex<double> zexpz_d(complex<double> z);
	complex<double> zexpz_dd(complex<double> z);
	complex<double> InitPoint(complex<double> z, int k);
	complex<double> LambertW(complex<double> z, int k = 0);
}
