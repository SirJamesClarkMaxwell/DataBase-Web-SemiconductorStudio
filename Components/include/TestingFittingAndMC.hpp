#pragma once
#include "pch.hpp"
#include "LambertW.h"
#include "imgui.h"
#include "implot.h"
#include ".\linerRegression.hpp"
#include "IVFitter.hpp"
// #include <NumericStorm.hpp>
// #include "../../NumericStorm/NumericStorm/headers/FittingSandBoxFunctions.hpp"
namespace UI::Data::JunctionFitMasterUI
{
	class Characteristic;
	using json = nlohmann::json;
	void from_json(const json &jsonObject, Characteristic &characteristic);
	void to_json(json &jsonObject, Characteristic &characteristic);

	bool checkExistence(const std::vector<Characteristic> &destination, const std::string &item);

	enum class ParametersNames
	{
		A,
		I0,
		Rs,
		Rp,
		Temperature
	};
	struct FourParameters
	{

		FourParameters() = default;
		typename NumericStorm::Fitting::Parameters<4> parameters;

		double &operator[](const ParametersNames &name) { return parameters[static_cast<int>(name)]; };
		// double A = 1;
		// double I0 = std::pow(10, -19);
		// double Rs = 10;
		// double Rp = 1000;
		double Temperature = 300;
		// operator NumericStorm::Parameters<4> () { return parameters; };
	};
	//! Remove after including NumericStorm

	class JFMData : public NumericStorm::Fitting::Data
	{
		using vector = std::vector<double>;

	public:
		enum class ReturningType
		{
			Voltage,
			Current,
			DensityCurrent
		};

		JFMData()
			: Data(3) {}
		JFMData(const vector &V, const vector &I, const vector &J)
		{
			m_data[static_cast<int>(ReturningType::Voltage)] = V;
			m_data[static_cast<int>(ReturningType::Current)] = I;
			m_data[static_cast<int>(ReturningType::DensityCurrent)] = J;
		};
		std::vector<double> &get(const ReturningType &name)
		{
			return m_data[static_cast<int>(name)];
		};
		std::vector<double> &getLog(const ReturningType item)
		{
			std::vector<double> originalItem(get(item));

			auto logf = [](double in)
			{ return std::log(std::abs(in)); };
			std::vector<double> logItem(originalItem.size());
			std::transform(originalItem.begin(), originalItem.end(), logItem.begin(), logf);
			return logItem;
		};
	};

	////////////////

	class Characteristic
	{
	private:
		using Data = JFMData;

	public:
		using ReturningType = JFMData::ReturningType;
		Characteristic() = default;
		Characteristic(const std::filesystem::path &path)
			: m_path(path) { readData(); };
		JFMData originalData;
		JFMData rangedData;
		double getTemperature() { return m_temperature; };
		std::string name;
		bool selected = true;
		ImVec4 m_color{1, 0, 0, 1};
		operator bool() { return selected; };
		size_t lowerIndex{0};
		size_t upperIndex{10};
		std::vector<double> getLog(const ReturningType &type, bool ranged = false)
		{
			return !ranged ? originalData.getLog(type) : rangedData.getLog(type);
		};
		std::vector<double> &get(const ReturningType &type, bool ranged = false)
		{
			return !ranged ? originalData.get(type) : rangedData.get(type);
		};
		void updateRangedCharacteristic();
		FourParameters parameters;
		void setAll(std::vector<double> item)
		{
			for (const auto &i : std::ranges::iota_view(0, 4))
				originalData.get(static_cast<ReturningType>(i)) = item;
		};

	private:
		double m_temperature{-1};
		std::filesystem::path m_path;

	private:
		json createJSONObject(std::string &path);
		void to_json(json &j, const Characteristic characteristic);
		double read_temperature(std::string &path);
		void from_json(const json &j, Characteristic &iv);
		void resize(int value);
		void readData();
	};
	struct PlotSettings
	{
		ImVec4 startColor{0, 0, 1, 1};
		ImVec4 endColor{1, 0, 0, 1};
		bool xLog = false;
		bool yLog = false;
		bool fitPlotArea = true;
		ImVec2 plot_size;
		ImPlotAxisFlags axisBaseFlags = ImPlotAxisFlags_None | ImPlotAxisFlags_AutoFit;
		ImPlotFlags plotBaseFlags = ImPlotFlags_NoLegend;
		ImGuiColorEditFlags basicPlotColorFlags = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB;
	};
	struct TableSettings
	{
		ImGuiTableFlags basicTableFlags = ImGuiTableFlags_None | ImGuiTableFlags_Resizable;
		ImGuiColorEditFlags basicTableColorFlags = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB;
	};
	struct TemporaryVariables
	{
		bool exits = false;
		Characteristic characteristic;
		float downRange;
		float upperRange;
		operator bool() { return exits; };
	};

	struct ContentBrowserData
	{
		ContentBrowserData() = default;
		std::filesystem::path currentPath = "./../Characteristics/";
		std::filesystem::path rootPath = "./../Characteristics/";
		std::vector<Characteristic> characteristics{};
		void readCharacteristic(const std::filesystem::path &path);
	};
	struct GeneratingData
	{

		enum ItemNames
		{
			min = 0,
			max,
			step
		};

		struct Params
		{

			Params() = default;
			std::array<float, 3> items;
			float value;
			float &operator[](const ItemNames &name) { return items[name]; };
		};

		Params &operator[](const ParametersNames &name) { return params[static_cast<int>(name)]; };
		ParametersNames option;
		std::vector<std::string> names{"A   ", "I0  ", "Rs  ", "Rsh ", "T"};
		std::array<Params, 4> params;
		std::vector<double> Voltages;
	};
	class FittingTesting
	{
	public:
		PlotSettings plotSettings;
		TableSettings tableSettings;
		int characteristicIndex{-1};
		ContentBrowserData contentBrowserData;
		GeneratingData generatingData;
		void DrawPlotData();
		void DrawActionsPanel();
		void DrawTable();

		std::vector<Characteristic> m_characteristics;

	private:
		bool m_openedContentBrowserData = false;
		bool m_openGenerateData = false;

	private:
		void drawLegend();
		void setColorsOfCharacteristics();
		void plottingCharacteristics();
		void plotOneCharacteristic(Characteristic &item, bool logy = true, bool logx = false);
		void plotControls();
		void SetRange();
		void FixRs();
		void FixRsh();
		void PreFit();

		void LoadCharacteristics();
		void readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath);

		void GenerateCharacteristic();
		void DrawSingleRangeGenerationOption(const std::string &name, GeneratingData::Params &destination, const int &i, int &ID);
		void generate(Characteristic &characteristic, const FourParameters &parameters);
		void GenerateRange();
		void SingleShot();
		void Fit();
		void DoMonteCarloSimulation();
		void PlotMonteCarloResults();

		void PlotData();
		void AddNoise();
		void DoAutoRange();
	};

}