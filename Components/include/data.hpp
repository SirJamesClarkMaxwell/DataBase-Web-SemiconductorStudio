#pragma once
#include "stl.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "pch.hpp"
#include <cmath>
#include <filesystem>
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
		void readData(std::filesystem::path &path)
		{
			std::string pathString = path.string();
			json readCharacteristic = createJSONObject(pathString);
			from_json(readCharacteristic, *this);
		};
		std::vector<double> getVoltage() { return V; };
		std::vector<double> getCurrent() { return I; };
		std::vector<double> getLogCurrent()
		{
			auto logf = [](double I)
			{ return std::log(std::abs(I)); };
			std::vector<double> logI{I};
			std::transform(I.begin(), I.end(), logI.begin(), logf);
			return logI;
		}
		std::vector<double> getDensityCurrent() { return J; };

	private:
		void resize(int value)
		{
			V.resize(value);
			I.resize(value);
			J.resize(value);
		};
		json createJSONObject(std::string &path)
		{
			const char *fileName = path.c_str();
			std::ifstream file(fileName);
			std::string jsonString;
			std::string line;
			while (std::getline(file, line))
				jsonString += line;
			return json::parse(jsonString, nullptr, false);
		};
		void to_json(json &j, const Characteristic characteristic)
		{
			j = json{{"voltage", characteristic.V}, {"current", characteristic.I}, {"density current", characteristic.J}};
		};
		void from_json(const json &j, Characteristic &iv)
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
		};

	private:
		std::vector<double> V{};
		std::vector<double> I{};
		std::vector<double> J{};
	};

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
		void addCharacteristic(std::filesystem::path path);
		Characteristic operator[](int index) { return m_characteristics[index]; }
		int numberOfCharacteristics = 0;

	private:
		std::vector<Characteristic> m_characteristics{};
	};

	struct ContentBrowserData
	{
		ContentBrowserData() = default;
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