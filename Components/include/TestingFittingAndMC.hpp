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

	namespace utils
	{
		void generateVectorAtGivenRanges(std::vector<double> &destination, double min, double max, double step);
		template <typename ToCast>
		int cast(ToCast name) { return static_cast<int>(name); };
		template <typename ToCast>
		ToCast cast(int index) { return static_cast<ToCast>(index); };
	};
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

		double &operator[](const ParametersNames &name)
		{
			if (name == ParametersNames::Temperature)
				return Temperature;
			return parameters[utils::cast(name)];
		};

		double Temperature = 300;
		// operator NumericStorm::Parameters<4>() { return parameters; };
	};
	//! Remove after including NumericStorm

	class JFMData : public NumericStorm::Fitting::Data
	{
		using vector = std::vector<double>;

	public:
		enum class ReturningType
		{
			Voltage = 0,
			Current,
			DensityCurrent
		};

		JFMData()
			: Data(3) {}
		JFMData(const vector &V, const vector &I, const vector &J)
		{
			m_data[utils::cast(ReturningType::Voltage)] = V;
			m_data[utils::cast(ReturningType::Current)] = I;
			m_data[utils::cast(ReturningType::DensityCurrent)] = J;
		};
		std::vector<double> &get(const ReturningType &name)
		{
			return m_data[utils::cast(name)];
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
		double &getTemperature() { return parameters[ParametersNames::Temperature]; };
		std::string name;
		bool selected = true;
		bool plotRanged = true;
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
			for (const auto &i : std::ranges::iota_view(0, 3))
				originalData.get(utils::cast<ReturningType>(i)) = item;
		};
		double getMin() { return originalData.get(ReturningType::Current)[0]; };
		double getMax()
		{
			int size = originalData.get(ReturningType::Current).size();
			return originalData.get(ReturningType::Current)[size - 1];
		};

	private:
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
	enum ItemNames
	{
		min = 0,
		max,
		step
	};
	template <size_t parameter_size>
	struct Params
	{

		Params() = default;
		std::array<float, parameter_size> items;
		float value;
		float &operator[](const ItemNames &name) { return items[utils::cast(name)]; };
	};
	struct GeneratingData
	{

		Params<3> &operator[](const ParametersNames &name) { return params[utils::cast(name)]; };
		ParametersNames option;
		std::vector<std::string> names{"A   ", "I0  ", "Rs  ", "Rsh ", "T   "};
		std::array<Params<3>, 5> params;
		std::vector<double> Voltages;
	};

	struct SimplexSettings
	{
		std::vector<Params<2>> bounds{4};
		Params<2> &operator[](const ItemNames &name) { return bounds[utils::cast(name)]; };
	};
	struct AutoRangeSettings
	{
		bool open = false;
		std::array<Params<3>, 2> bounds;
		std::vector<std::string> names{"min", "max", "step"};
		operator bool() { return open; };
	};
	struct FittingResults
	{
	using namespace JunctionFitMasterFromNS::IVFitting;
		SimplexOptimizationResults result;
		int lowerIndex, upperIndex;
		bool operator()(const FittingResults &lhs, const FittingResults &rhs) { return lhs.result.getError() < rhs.result.getError(); }
	};
	class FittingTesting
	{
	public:
		PlotSettings plotSettings;
		TableSettings tableSettings;
		int characteristicIndex{-1};
		ContentBrowserData contentBrowserData;
		GeneratingData generatingData;
		SimplexSettings simplexSettings;
		AutoRangeSettings autoRangeSettings;
		std::vector<Characteristic> m_characteristics;

		void DrawPlotData();
		void DrawActionsPanel();
		void DrawTable();

	private:
		bool m_openedContentBrowserData = false;
		bool m_openGenerateData = false;
		bool m_showSimplexSettings = false;

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

		void SetSimplexSettings();
		void SimplexSettingsUI(Params<2> &destination, const std::string &name, int ID);

		void LoadCharacteristics();
		void readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath);

		void GenerateCharacteristic();
		void DrawSingleRangeGenerationOption(const std::string &name, Params<3> &destination, const int &i, int &ID);
		void generate(Characteristic &characteristic, const FourParameters &parameters);
		void GenerateRange();
		void SingleShot();
		void Fit();
		void DoMonteCarloSimulation();
		void PlotMonteCarloResults();

		void PlotData();
		void AddNoise();

		void DoAutoRange();
		void AutoRange();
	};

}