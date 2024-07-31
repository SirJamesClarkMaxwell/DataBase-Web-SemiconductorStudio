#include "pch.hpp"
#include "TestingFittingAndMC.hpp"

namespace UI::Data::JunctionFitMasterUI
{
	namespace utils
	{
		void generateVectorAtGivenRanges(std::vector<double> &destination, double min, double max, double step)
		{
			int size = static_cast<int>((max - min) / step) + 1;
			destination.resize(size);

			int count = -1;
			std::ranges::generate(destination.begin(), destination.end(), [&]()
								  { count++; return count * step; });
		};

	};
	void FittingTesting::DrawPlotData()
	{

		plottingCharacteristics();
		ImGui::SameLine();
		drawLegend();
		plotControls();
	}
	void FittingTesting::drawLegend()
	{
		if (ImGui::Button("Select All"))
			for (auto &item : m_characteristics)
				item.selected = true;
		ImGui::SameLine();
		if (ImGui::Button("UnSelect All"))
		{
			for (auto &item : m_characteristics)
				item.selected = false;
			characteristicIndex = -1;
		}
		if (ImGui::Button("SetActive Characteristic"))
		{
			for (auto const &[index, item] : std::views::enumerate(m_characteristics))
				if (item)
					characteristicIndex = index;
		}

		ImGui::Separator();
		if (ImGui::BeginTable("Legend", 2, tableSettings.basicTableFlags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableHeadersRow();
			int i = 0;
			for (auto &item : m_characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(std::to_string(i++).c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
			}
			ImGui::EndTable();
		}
	}
	void FittingTesting::plotControls()
	{
		ImGui::Begin("plotControls");
		ImGui::CheckboxFlags("Fit Plotting Area", (unsigned int *)&plotSettings.plotBaseFlags, ImPlotAxisFlags_AutoFit);
		ImGui::PushItemWidth(50);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
			std::sort(m_characteristics.begin(), m_characteristics.end(), [](Characteristic &lhs, Characteristic &rhs)
					  { return lhs.getTemperature() < rhs.getTemperature(); });
		ImGui::SameLine();
		if (ImGui::Button("Reverse"))
			std::reverse(m_characteristics.begin(), m_characteristics.end());
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Checkbox("x log", &plotSettings.xLog);
		ImGui::SameLine();
		ImGui::Checkbox("y log", &plotSettings.yLog);
		if (ImGui::Button("Set Range"))
			SetRange();
		ImGui::SameLine();
		if (ImGui::Button("Fix Rs"))
			FixRs();
		ImGui::SameLine();
		if (ImGui::Button("Fix Rsh"))
			FixRsh();
		ImGui::SameLine();
		if (ImGui::Button("PreFit"))
			PreFit();

		float min{0.001}, max{5}, step{0.01};

		if (characteristicIndex != -1)
		{
			Characteristic::ReturningType returningType = Characteristic::ReturningType::Voltage;
			Characteristic &tmpCharac = m_characteristics[characteristicIndex];

			int &lowerSlider = (int &)tmpCharac.lowerIndex;
			int &upperSlider = (int &)tmpCharac.upperIndex;

			int min, max, step;
			min = 0;
			max = tmpCharac.get(returningType, false).size() - 1;
			step = tmpCharac.get(returningType, false)[1] - min;

			if (ImGui::SliderInt("Down Range", &lowerSlider, 0, upperSlider, std::to_string(tmpCharac.get(returningType)[lowerSlider]).c_str()))
			{
				tmpCharac.updateRangedCharacteristic();
				auto Voltage = Characteristic::ReturningType::Voltage;
				auto Current = Characteristic::ReturningType::Current;
				auto &originalV = m_characteristics[characteristicIndex].get(Voltage, true);
				auto &originalI = m_characteristics[characteristicIndex].get(Current, true);
				LinearRegression fitter{};
				std::valarray<double> V, I;
				V = std::valarray<double>(originalV.data(), (int)originalV.size());
				I = std::valarray<double>(originalI.data(), (int)originalI.size());
				fitter(I, V);
				m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] = 1 / fitter.getB();
				std::cout << "1/ fitter.getB(): " << 1 / fitter.getB() << std::endl;
				std::cout << "fitter.getB()  " << fitter.getB() << std::endl;
			}
			ImGui::SameLine();
			ImGui::Text(std::to_string(tmpCharac.get(returningType)[lowerSlider]).c_str());
			if (ImGui::SliderInt("Up Range", &upperSlider, lowerSlider, max, std::to_string(tmpCharac.get(returningType)[upperSlider]).c_str()))
			{
				tmpCharac.updateRangedCharacteristic();
				auto Voltage = Characteristic::ReturningType::Voltage;
				auto Current = Characteristic::ReturningType::Current;
				auto &originalV = m_characteristics[characteristicIndex].get(Voltage, true);
				auto &originalI = m_characteristics[characteristicIndex].get(Current, true);
				LinearRegression fitter{};
				std::valarray<double> V, I;
				V = std::valarray<double>(originalV.data(), (int)originalV.size());
				I = std::valarray<double>(originalI.data(), (int)originalI.size());
				fitter(I, V);
				m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] = fitter.getB();
				std::cout << "1/ fitter.getB(): " << 1 / fitter.getB() << std::endl;
				std::cout << "fitter.getB()  " << fitter.getB() << std::endl;
			}
			ImGui::SameLine();
			ImGui::Text(std::to_string(tmpCharac.get(returningType)[upperSlider]).c_str());
			ImGui::SameLine();
		}
		ImGui::End();
	}
	void Characteristic::updateRangedCharacteristic()
	{
		for (const int &item : std::ranges::iota_view(1, 3))
			rangedData.get(static_cast<ReturningType>(item)) = std::vector<double>{originalData.get(static_cast<ReturningType>(item)).begin() + lowerIndex, originalData.get(static_cast<ReturningType>(item)).begin() + upperIndex};
	};

	void FittingTesting::plottingCharacteristics()
	{
		ImGui::Begin("Plot Area");
		ImVec2 plot_size(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		plotSettings.plot_size = plot_size;
		if (ImPlot::BeginPlot("characteristic", plotSettings.plot_size, plotSettings.plotBaseFlags))
		{
			auto transformForwardLinear = [](double v, void *)
			{ return std::log(std::abs(v)); };
			auto transformForwardNaturalLog = [](double v, void *)
			{ return std::exp(v); };

			ImPlot::SetupAxes("V", "I", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
#if 1
			if (plotSettings.xLog)
				ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
			if (plotSettings.yLog)
				ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);
#endif
			for (auto &item : m_characteristics)
				if (item)
					plotOneCharacteristic(item, false, false);
			if (characteristicIndex != -1)
			{
				Characteristic::ReturningType returningType = Characteristic::ReturningType::Voltage;
				Characteristic &tmpCharac = m_characteristics[characteristicIndex];

				int &lowerSlider = (int &)tmpCharac.lowerIndex;
				int &upperSlider = (int &)tmpCharac.upperIndex;

				ImPlot::PlotInfLines("Lower bound", &tmpCharac.get(returningType)[lowerSlider], 1);
				ImPlot::PlotInfLines("Upper bound", &tmpCharac.get(returningType)[upperSlider - 1], 1);
			}
			ImPlot::EndPlot();
		}
		ImGui::End();
	}
	void FittingTesting::plotOneCharacteristic(Characteristic &item, bool logy, bool logx)
	{
		using ReturningType = JFMData::ReturningType;
		auto V = logx ? item.getLog(ReturningType::Voltage, false) : item.get(ReturningType::Voltage, false);
		auto I = logy ? item.getLog(ReturningType::Current, false) : item.get(ReturningType::Current, false);

		std::string title = "I(V) " + item.name + " K";
		ImPlot::SetNextLineStyle(item.m_color);
		ImPlot::PlotLine(title.c_str(), V.data(), I.data(), V.size());

		auto V1 = logx ? item.getLog(ReturningType::Voltage, true) : item.get(ReturningType::Voltage, true);
		auto I1 = logy ? item.getLog(ReturningType::Current, true) : item.get(ReturningType::Current, true);
		std::string title1 = "I(V) ranged" + item.name + " K";
		ImPlot::SetNextLineStyle((ImVec4)ImColor(24, 249, 223, 255));
		ImPlot::PlotLine(title1.c_str(), V1.data(), I1.data(), V1.size());
	};
	void FittingTesting::DrawActionsPanel()
	{
		ImGui::PushItemWidth(200);
		if (ImGui::Button("Load Data") || m_openedContentBrowserData)
			LoadCharacteristics();
		ImGui::SameLine();
		if (ImGui::Button("Generate Data") || m_openGenerateData)
			GenerateCharacteristic();
		if (ImGui::Button("PreFit"))
			Fit();

		if (ImGui::Button("Monte Carlo Simulation"))
			DoMonteCarloSimulation();
		ImGui::SameLine();
		if (ImGui::Button("Plot MonteCarlo"))
			PlotMonteCarloResults();
		ImGui::PopItemWidth();
	}
	void FittingTesting::SetRange()
	{
	}
	void FittingTesting::FixRs()
	{
		auto Voltage = Characteristic::ReturningType::Voltage;
		auto Current = Characteristic::ReturningType::Current;
		auto &originalV = m_characteristics[characteristicIndex].get(Voltage, true);
		auto &originalI = m_characteristics[characteristicIndex].get(Current, true);
		LinearRegression fitter{};
		std::valarray<double> V, I;
		V = std::valarray<double>(originalV.data(), originalV.size());
		I = std::valarray<double>(originalI.data(), originalI.size());
		fitter(I, V);
		m_characteristics[characteristicIndex].parameters[ParametersNames::Rs] = fitter.getB();
		std::cout << "Rs: (fitter.getB() ) " << m_characteristics[characteristicIndex].parameters[ParametersNames::Rs] << std::endl;
		std::cout << "Rs: V[0]/I[0] " << V[0] / I[0] << std::endl;
	}
	void FittingTesting::FixRsh()
	{
		auto Voltage = Characteristic::ReturningType::Voltage;
		auto Current = Characteristic::ReturningType::Current;
		auto &originalV = m_characteristics[characteristicIndex].get(Voltage, true);
		auto &originalI = m_characteristics[characteristicIndex].get(Current, true);
		LinearRegression fitter{};
		std::valarray<double> V, I;
		V = std::valarray<double>(originalV.data(), (int)originalV.size());
		I = std::valarray<double>(originalI.data(), (int)originalI.size());
		fitter(I, V);
		m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] = 1 / fitter.getB();
		std::cout << "Rp: ( 1/ fitter.getAB) ) " << m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] << std::endl;
	}
	void FittingTesting::PreFit()
	{
	}

	void FittingTesting::LoadCharacteristics()
	{

		ImGui::Begin("Load Data Window");
		m_openedContentBrowserData = true;
		if (ImGui::Button("Read all"))
			readAllDataFromDirectory(contentBrowserData.rootPath, contentBrowserData.currentPath);
		ImGui::SameLine();
		if (ImGui::Button("Transfer Read data to Plot"))
		{
			for (auto &item : contentBrowserData.characteristics)
			{
				if (!checkExistence(m_characteristics, item.name))
					m_characteristics.push_back(item);
			}
		}
		ImGui::SameLine();
		if (contentBrowserData.currentPath != std::filesystem::path(contentBrowserData.rootPath))
		{
			if (ImGui::Button("<-"))
				contentBrowserData.currentPath = contentBrowserData.currentPath.parent_path();
		}
		for (auto &directoryEntry : std::filesystem::directory_iterator(contentBrowserData.currentPath))
		{
			const auto &path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, contentBrowserData.rootPath);
			std::string fileNameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(fileNameString.c_str()))
					contentBrowserData.currentPath /= path.filename();
			}
			else
			{
				if (ImGui::Button(fileNameString.c_str()))
					contentBrowserData.readCharacteristic(path);
			}
		}
		ImGui::Separator();
		if (ImGui::Button("close"))
			m_openedContentBrowserData = false;
		ImGui::End();
	}

	void FittingTesting::readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath)
	{
		for (auto &directoryEntry : std::filesystem::directory_iterator(currentPath))
		{
			const auto &path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, rootPath);
			std::string fileNameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
				continue;
			else
				contentBrowserData.readCharacteristic(path);
		}
	};
	bool JunctionFitMasterUI::checkExistence(const std::vector<Characteristic> &destination, const std::string &item)
	{
		auto found = std::find_if(destination.begin(), destination.end(),
								  [&item](const Characteristic &element)
								  {
									  return element.name == item;
								  });
		bool f = found != destination.end();
		return f;
	}

	void ContentBrowserData::readCharacteristic(const std::filesystem::path &path)
	{
		Characteristic characteristic(path);
		if (!checkExistence(characteristics, characteristic.name))
			characteristics.push_back(characteristic);
	}

	void Characteristic::resize(int value)
	{
		originalData.get(ReturningType::Voltage).resize(value);
		originalData.get(ReturningType::Current).resize(value);
		originalData.get(ReturningType::DensityCurrent).resize(value);
	};
	void Characteristic::readData()
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
	json Characteristic::createJSONObject(std::string &path)
	{
		const char *fileName = path.c_str();
		std::ifstream file(fileName);
		std::string jsonString;
		std::string line;
		while (std::getline(file, line))
			jsonString += line;
		return json::parse(jsonString, nullptr, false);
	};

	void Characteristic::to_json(json &j, Characteristic characteristic)
	{

		j = json{{"voltage", characteristic.get(ReturningType::Voltage)}, {"current", characteristic.originalData.get(ReturningType::Current)}, {"density current", characteristic.get(ReturningType::DensityCurrent)}};
	};

	void Characteristic::from_json(const json &j, Characteristic &iv)
	{

		auto strToDouble = [](const std::string &str)
		{ return std::stod(str); };
		auto transform = [&](const std::vector<std::string> &str, std::vector<double> &vec)
		{
			std::transform(str.begin(), str.end(), vec.begin(), strToDouble);
		};
		iv.resize(j.at("voltage").size());

		transform(j.at("voltage"), iv.get(ReturningType::Voltage));
		transform(j.at("current"), iv.get(ReturningType::Current));
		transform(j.at("density current"), iv.get(ReturningType::DensityCurrent));
	};

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
	};

	void FittingTesting::GenerateCharacteristic()
	{
		m_openGenerateData = true;
		int id = 0;
		ImGui::Begin("Generate Data Window");
		static float voltages[3]{0.0f, 4.0f, 0.1}; // min - max - step
		if (ImGui::DragFloat3("Voltages", (float *)&voltages, 0.1, 0.001, 4, "%.2f"))
			utils::generateVectorAtGivenRanges(generatingData.Voltages, voltages[0], voltages[1], voltages[2]);

		for (const auto &[description, destination, i] : std::views::zip(generatingData.names, generatingData.params, std::ranges::iota_view(0, 4)))
			DrawSingleRangeGenerationOption(description, destination, i, id);
		if (ImGui::Button("Single Shot"))
			SingleShot();
		if (ImGui::Button("Generate Range"))
			GenerateRange();
		ImGui::End();
		// std::cout << "Generate Data was pressed! " << std::endl;
	};
	void FittingTesting::DrawSingleRangeGenerationOption(const std::string &name, GeneratingData::Params &destination, const int &i, int &ID)
	{
		std::string itemBaseName = name + std::to_string(i);
		ImGui::PushID(itemBaseName.c_str());

		ImGui::RadioButton(name.c_str(), (int *)&generatingData.option, i);
		ImGui::SameLine();

		ImGui::PopID();
		ImGui::PushItemWidth(200);
		ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		if (name == "I0")
			flags | ImGuiSliderFlags_Logarithmic;

		ImGui::PushID((itemBaseName + "1").c_str());
		ImGui::DragFloat3("##DragFloat3", (float *)&destination.items, 0.1, 0.0f, 0.0f, "%.3f", flags);
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::PushID((itemBaseName + "2").c_str());
		ImGui::DragFloat("##DragFloat", &destination.value, 0.1, 0.0f, 10.0f, "%.3f");
		ImGui::PopID();
		ImGui::PopItemWidth();
	};
	void FittingTesting::GenerateRange()
	{

		ParametersNames choosen = generatingData.option;
		GeneratingData::Params parameters = generatingData[choosen];

		using names = GeneratingData::ItemNames;
		int counts = static_cast<int>((parameters[names::max] - parameters[names::min]) / parameters[names::step]) + 1;
		std::vector<double> steps(counts);

		utils::generateVectorAtGivenRanges(steps, parameters[names::min], parameters[names::max], parameters[names::step]);
		FourParameters params;
		for (const auto &name : std::ranges::iota_view(0, 4))
			params[static_cast<ParametersNames>(name)] = generatingData.params[static_cast<int>(name)].value;

		std::vector<FourParameters> finalParameters(steps.size(), params);
		for (const auto &[destination, item] : std::views::zip(finalParameters, steps))
			destination[choosen] = item;

		Characteristic referenceToCopy;
		referenceToCopy.setAll(generatingData.Voltages);
		std::vector<Characteristic> characteristics{counts, referenceToCopy};

		for (const auto &[characteristic, parameters] : std::views::zip(characteristics, finalParameters))
			generate(characteristic, parameters);
	};
	void FittingTesting::SingleShot()
	{
		FourParameters parameters;
		for (const auto &name : std::ranges::iota_view(0, 4))
			parameters[static_cast<ParametersNames>(name)] = generatingData.params[static_cast<int>(name)].value;
		Characteristic characteristic;
		characteristic.setAll(generatingData.Voltages);
		generate(characteristic, parameters);
	};
	void FittingTesting::generate(Characteristic &characteristic, const FourParameters &parameters)
	{
		using namespace JunctionFitMasterFromNS::IVFitting;
		IVModel(characteristic.originalData, parameters.parameters, parameters.Temperature);
		m_characteristics.push_back(characteristic);
	};

	void FittingTesting::Fit()
	{
		std::cout << "PreFit was pressed! " << std::endl;
	}
	void FittingTesting::DoMonteCarloSimulation()
	{
		std::cout << "Monte Carlo Simulation was pressed! " << std::endl;
	}
	void FittingTesting::PlotMonteCarloResults()
	{
		std::cout << "Plot MonteCarlo was pressed! " << std::endl;
	}

	void FittingTesting::DrawTable()
	{
		ImGui::SameLine();

		if (ImGui::ColorEdit4("start color", (float *)&plotSettings.startColor, plotSettings.basicPlotColorFlags))
		{
			setColorsOfCharacteristics();
			// setColorsOfGraph();
		}
		ImGui::SameLine();
		if (ImGui::ColorEdit4("end color", (float *)&plotSettings.endColor, plotSettings.basicPlotColorFlags))
		{
			setColorsOfCharacteristics();
			// plotData.setColorsOfGraph();
		}
		ImGui::SameLine();
		if (ImGui::Button("Update Characteristics Color"))
			// {
			setColorsOfCharacteristics();
		// plotData.setColorsOfGraph();
		// }
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::Spacing();

		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);

		ImU32 col_a_ = ImGui::ColorConvertFloat4ToU32(plotSettings.startColor);
		ImU32 col_b_ = ImGui::ColorConvertFloat4ToU32(plotSettings.endColor);
		draw_list->AddRectFilledMultiColor(p0, p1, col_a_, col_b_, col_b_, col_a_);

		ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight()));
		if (ImGui::BeginTable("Characteristic", 4, tableSettings.basicTableFlags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Temperature");
			ImGui::TableSetupColumn("Color");
			ImGui::TableHeadersRow();

			for (auto &item : m_characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.name.c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.getTemperature()).c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.name.c_str(), (float *)&item.m_color, tableSettings.basicTableColorFlags);
			}

			ImGui::EndTable();
		}
	}
	// void FittingTesting::setColorsOfGraph()
	// {
	//     plotProperties.colors.resize(m_characteristics.size());
	//     std::transform(m_characteristics.begin(), m_characteristics.end(), plotProperties.colors.begin(), [&](const Characteristic &item)
	//                    { return item.m_color; });
	// }

	void FittingTesting::setColorsOfCharacteristics()
	{
		// ImColor colorStep = (endColor - startColor) / (characteristics.size() - 1);
		// for (const auto &[index, item] : std::views::enumerate(characteristics))
		//     item.m_color = startColor + index * colorStep;
		// todo extract this code to gconcrete functions
		size_t numChars = m_characteristics.size();
		for (size_t i = 0; i < numChars; ++i)
		{
			float t = static_cast<float>(i) / (numChars - 1); // Calculate the interpolation factor (0 to 1)
			m_characteristics[i].m_color = ImColor(
				plotSettings.startColor.x * (1.0f - t) + plotSettings.endColor.x * t,
				plotSettings.startColor.y * (1.0f - t) + plotSettings.endColor.y * t,
				plotSettings.startColor.z * (1.0f - t) + plotSettings.endColor.z * t,
				plotSettings.startColor.w * (1.0f - t) + plotSettings.endColor.w * t);
		}
	}
}