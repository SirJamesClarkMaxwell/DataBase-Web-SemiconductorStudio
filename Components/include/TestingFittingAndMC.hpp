#pragma once
#include "pch.hpp"
#include "LambertW.h"
#include "imgui.h"
#include "implot.h"
#include ".\linerRegression.hpp"
// #include <NumericStorm.hpp>

namespace UI::Data::JunctionFitMaster
{
	class Characteristic;
	using json = nlohmann::json;
	void from_json(const json &jsonObject, Characteristic &characteristic);
	void to_json(json &jsonObject, Characteristic &characteristic);

	bool checkExistence(const std::vector<Characteristic> &destination, const std::string &item);

	struct FourParameters
	{
		FourParameters() = default;
		double A = 1;
		double I0 = std::pow(10, -19);
		double Rs = 10;
		double Rp = 1000;
		double Temperature = 300;
	};
	//! Remove after including NumericStorm
	namespace NumericStorm
	{
		class Data
		{
			using vector = std::vector<double>;

		public:
			enum class ReturningType
			{
				None = 0,
				Voltage,
				Current,
				DensityCurrent
			};

			Data() = default;
			Data(const vector &V, const vector &I, const vector &J)
				: V{V}, I{I}, J{J} {};
			std::vector<double> &get(const ReturningType &item)
			{

				switch (item)
				{
				case ReturningType::Voltage:
					return V;
				case ReturningType::Current:
					return I;
				case ReturningType::DensityCurrent:
					return J;
				}
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

		private:
			std::vector<double> V;
			std::vector<double> I;
			std::vector<double> J;
		};
		class Model
		{
		};
		class Fitter
		{
		};
		class MonteCarloSimulation
		{
		};
	}
	////////////////
	class FourParameterModel : public NumericStorm::Model
	{
	};

	class Characteristic
	{
	private:
		using Data = NumericStorm::Data;

	public:
		using ReturningType = NumericStorm::Data::ReturningType;
		Characteristic() = default;
		Characteristic(const std::filesystem::path &path)
			: m_path(path) { readData(); };
		NumericStorm::Data originalData;
		NumericStorm::Data rangedData;
		double getTemperature() { return m_temperature; };
		std::string name;
		bool selected = true;
		ImVec4 m_color{1, 0, 0, 1};
		operator bool() { return selected; };
		size_t lowerIndex{ 0 };
		size_t upperIndex{ 10 };
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
	class FittingTesting
	{
	public:
		PlotSettings plotSettings;
		TableSettings tableSettings;
		int characteristicIndex{-1};
		ContentBrowserData contentBrowserData;
		void DrawPlotData();
		void DrawActionsPanel();
		void DrawTable();

		std::vector<Characteristic> m_characteristics;

	private:
		bool m_openedContentBrowserData = false;

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
		void Fit();
		void DoMonteCarloSimulation();
		void PlotMonteCarloResults();

		void PlotData();
		void AddNoise();
		void DoAutoRange();
	};

}