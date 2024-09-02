#pragma once
#include "pch.hpp"
#include "TestingFittingAndMC.hpp"
#include "PreFit.hpp"

namespace UI::Data::JunctionFitMasterUI
{

	std::pair<std::vector<double>, std::vector<double>> AlphaDV{};


	std::array<std::array<double, 5>, 4> param_mat{};

	std::array<double, 4> centr{};
	
	static IVSimplexOptimizer<IVModel> ivOptimizer{};
	static typename IVSimplexOptimizer<IVModel>::SettingsT::OptimizerStateT ivState{};

	using namespace NumericStorm::Fitting;

	struct TestingData {

		/*struct trueData {
			double A = 1.5067;
			double I0 = 6.418e10;
			double Rs = 7.7;
			double Rsh = 6963;
			double T = 330;
			Parameters<4> parameters{ {A, I0, Rs, Rsh} };
		} true_data{};*/

		/*struct trueData {
			double A = 1.5067;
			double I0 = 1.418e-8;
			double Rs = 7.7;
			double Rsh = 7000;
			double T = 330;
			Parameters<4> parameters{ {A, I0, Rs, Rsh} };
		} true_data{};*/

		struct trueData {
			double A = 1;
			//double A = 1.2;
			double I0 = 5e-8;
			double Rs = 5e-6;
			double Rsh = 5e8;
			double T = 330;
			Parameters<4> parameters{ {A, I0, Rs, Rsh} };
		} true_data{};


		struct simplexInit {
			/*Parameters<4> minBounds{ {0.5, 1e-20, 1e-9, 10} };
			Parameters<4> maxBounds{ { 20.0, 1e-3, 100, 1e9} };*/
			Parameters<4> minBounds{ {0.5, 1e-10, 1e-5, 1e4} };
			Parameters<4> maxBounds{ { 2.5, 9e-10, 9e-5, 9e4} };
			Parameters<4> initialGuess{ { 1.5746, 1.6e-8, 6.77, 7000} };
		} simplex_init{};

	} init_data{};


	void utils::generateVectorAtGivenRanges(std::vector<double>& destination, double min, double max, double step)
	{
		int size = static_cast<int>((max - min) / step) + 1;
		destination.resize(size);

		int count = -1;
		std::ranges::generate(destination.begin(), destination.end(), [&]()
			{ count++; return min + count * step; });
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
			for (auto& item : m_characteristics)
				item.selected = true;
		ImGui::SameLine();
		if (ImGui::Button("UnSelect All"))
		{
			for (auto& item : m_characteristics)
				item.selected = false;
			characteristicIndex = -1;
		}
		ImGui::SameLine();
		if (ImGui::Button("SetActive Characteristic"))
		{
			for (auto const& [index, item] : std::views::enumerate(m_characteristics))
				if (item)
					characteristicIndex = index;
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear characteristics"))
		{
			m_characteristics.clear();
			characteristicIndex = -1;
		}
		if (ImGui::Button("Remove Selected"))
		{
			std::vector<Characteristic> tmp;
			for (auto& item : m_characteristics)
				if (!item)
					tmp.push_back(item);

			characteristicIndex = -1;
			m_characteristics.clear();

			for (auto& item : tmp)
				m_characteristics.push_back(item);
		}

		ImGui::Separator();
		if (ImGui::BeginTable("Legend", 2, tableSettings.basicTableFlags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableHeadersRow();
			int i = 0;
			for (auto& item : m_characteristics)
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

		ImGui::CheckboxFlags("Fit Plotting Area", (unsigned int*)&plotSettings.plotBaseFlags, ImPlotAxisFlags_AutoFit);
		ImGui::PushItemWidth(50);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
			std::sort(m_characteristics.begin(), m_characteristics.end(), [](Characteristic& lhs, Characteristic& rhs)
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
		ImGui::SameLine();
		if (ImGui::Button("Print Results"))
			PrintResults();
		float min{ 0.001 }, max{ 5 }, step{ 0.01 };

		if (characteristicIndex != -1)
		{
			Characteristic::ReturningType returningType = Characteristic::ReturningType::Voltage;
			Characteristic& item = m_characteristics[characteristicIndex];
			ImGui::Checkbox("plot ranged", &item.plotRanged);

			int& lowerSlider = (int&)item.lowerIndex;
			int& upperSlider = (int&)item.upperIndex;

			int min, max, step;
			min = 0;
			max = item.get(returningType, false).size() - 1;
			step = item.get(returningType, false)[1] - min;

			if (ImGui::SliderInt("Down Range", &lowerSlider, 0, upperSlider - 1, std::to_string(item.get(returningType)[lowerSlider]).c_str()))
			{
				item.updateRangedCharacteristic();
				/*auto Voltage = Characteristic::ReturningType::Voltage;
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
				std::cout << "fitter.getB()  " << fitter.getB() << std::endl;*/
			}
			ImGui::SameLine();
			ImGui::Text(std::to_string(item.get(returningType)[lowerSlider]).c_str());
			if (ImGui::SliderInt("Up Range", &upperSlider, lowerSlider + 1, max, std::to_string(item.get(returningType)[upperSlider]).c_str()))
			{
				item.updateRangedCharacteristic();
				/*
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


				originalV = m_characteristics[characteristicIndex].get(Voltage);
				originalI = m_characteristics[characteristicIndex].get(Current);
				int i{ upperSlider };
				V = std::valarray<double>(originalV.data(), (int)i);
				I = std::valarray<double>(originalI.data(), (int)i);

				fitter(I, V);

				std::cout << "B: " << fitter.getB() << std::endl;
				*/
			}
			ImGui::SameLine();
			ImGui::Text(std::to_string(item.get(returningType)[upperSlider]).c_str());
			ImGui::SameLine();
		}
		ImGui::End();
	}
	void Characteristic::updateRangedCharacteristic()
	{
		for (const int& item : std::ranges::iota_view(0, 3))
			rangedData.get(utils::cast<ReturningType>(item)) = std::vector<double>{
				originalData.get(utils::cast<ReturningType>(item)).begin() + lowerIndex,
				originalData.get(utils::cast<ReturningType>(item)).begin() + upperIndex };
	};

	void FittingTesting::plottingCharacteristics()
	{
		ImGui::Begin("Plot Area");
		ImVec2 plot_size(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		plotSettings.plot_size = plot_size;
		if (ImPlot::BeginPlot("characteristic", plotSettings.plot_size, plotSettings.plotBaseFlags))
		{
			auto transformForwardLinear = [](double v, void*)
				{ return std::log(std::abs(v)); };
			auto transformForwardNaturalLog = [](double v, void*)
				{ return std::exp(v); };

			ImPlot::SetupAxes("V", "I", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

			if (plotSettings.xLog)
				ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
			if (plotSettings.yLog)
				ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);
			for (auto& item : m_characteristics)
				if (item)
					plotOneCharacteristic(item, false, false);
			if (characteristicIndex != -1)
			{
				Characteristic::ReturningType returningType = Characteristic::ReturningType::Voltage;
				Characteristic& item = m_characteristics[characteristicIndex];

				int& lowerSlider = (int&)item.lowerIndex;
				int& upperSlider = (int&)item.upperIndex;

				ImPlot::PlotInfLines("Lower bound", &item.get(returningType)[lowerSlider], 1);
				ImPlot::PlotInfLines("Upper bound", &item.get(returningType)[upperSlider], 1);
			}
			ImPlot::EndPlot();
		}
		ImGui::End();
	};
	void FittingTesting::plotOneCharacteristic(Characteristic& item, bool logy, bool logx)
	{
		using ReturningType = JFMData::ReturningType;
		auto V = logx ? item.getLog(ReturningType::Voltage, false) : item.get(ReturningType::Voltage, false);
		auto I = logy ? item.getLog(ReturningType::Current, false) : item.get(ReturningType::Current, false);

		std::string title = "I(V) " + item.name + " K";
		ImPlot::SetNextLineStyle(item.m_color);

		ImPlot::PlotLine(title.c_str(), V.data(), I.data(), V.size());

		auto V1 = logx ? item.getLog(ReturningType::Voltage, true) : item.get(ReturningType::Voltage, true);
		auto I1 = logy ? item.getLog(ReturningType::Current, true) : item.get(ReturningType::Current, true);
		if (item.plotRanged)
		{
			std::string title1 = "I(V) ranged" + item.name + " K";
			ImPlot::SetNextLineStyle((ImVec4)ImColor(24, 249, 223, 255));
			ImPlot::PlotLine(title1.c_str(), V1.data(), I1.data(), V1.size());
		}
	};
	void FittingTesting::DrawActionsPanel()
	{
		ImGui::PushItemWidth(200);
		if (ImGui::Button("Load Data"))
			m_openedContentBrowserData = true;

		ImGui::SameLine();
		if (ImGui::Button("Generate Data"))
			m_openGenerateData = true;

		if (ImGui::Button("Settings Auto Range"))
			autoRangeSettings.open = true;

		ImGui::SameLine();
		if (ImGui::Button("Simplex Settings"))
			m_showSimplexSettings = true;

		// ImGui::SameLine();

		if (ImGui::Button("Fit"))
			Fit();
		
		if (ImGui::Button("Step"))
			Step(ivOptimizer, ivState);

		if (ImGui::Button("Step10"))
			for(int i = 0; i < 10; i++)
				Step(ivOptimizer, ivState);

		if (ImGui::Button("Step100"))
			for (int i = 0; i < 100; i++)
				Step(ivOptimizer, ivState);

		if (ImGui::Button("StepAll"))
			for (int i = 0; i < 2999; i++)
				Step(ivOptimizer, ivState);

		{
			ImGui::Begin("Simplex in A - i0 plane");
			if (ImPlot::BeginPlot("Simplex", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }))
			{
				auto transformForwardLinear = [](double v, void*)
					{ return std::log(std::abs(v)); };
				auto transformForwardNaturalLog = [](double v, void*)
					{ return std::exp(v); };

				ImPlot::SetupAxes("A", "I0", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

				if (plotSettings.xLog)
					ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
				if (plotSettings.yLog)
					ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);

				ImPlot::SetNextFillStyle(ImVec4(0, 0, 1, 0.5f), 1.0); 
				ImPlot::PlotScatter("Best", &(param_mat[0][4]), &(param_mat[1][4]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 0, 1, 0.5f), 1.0);
				ImPlot::PlotScatter("Rest", &(param_mat[0][2]), &(param_mat[1][2]), 2);

				ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Second worst", &(param_mat[0][1]), &(param_mat[1][1]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 0, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Worst", &(param_mat[0][0]), &(param_mat[1][0]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 1, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Centr", &(centr[0]), &(centr[1]), 1);

				ImPlot::EndPlot();
			}
			ImGui::End();
		}

		{
			ImGui::Begin("Simplex in Rs - Rsch plane");
			if (ImPlot::BeginPlot("Simplex", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }))
			{
				auto transformForwardLinear = [](double v, void*)
					{ return std::log(std::abs(v)); };
				auto transformForwardNaturalLog = [](double v, void*)
					{ return std::exp(v); };

				ImPlot::SetupAxes("Rs", "Rsch", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

				if (plotSettings.xLog)
					ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
				if (plotSettings.yLog)
					ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);

				
				ImPlot::SetNextFillStyle(ImVec4(0, 0, 1, 0.5f), 1.0);
				ImPlot::PlotScatter("Best", &(param_mat[2][4]), &(param_mat[3][4]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 0, 1, 0.5f), 1.0);
				ImPlot::PlotScatter("Rest", &(param_mat[2][2]), &(param_mat[3][2]), 2);

				ImPlot::SetNextFillStyle(ImVec4(0, 1, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Second worst", &(param_mat[2][1]), &(param_mat[3][1]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 0, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Worst", &(param_mat[2][0]), &(param_mat[3][0]), 1);

				ImPlot::SetNextFillStyle(ImVec4(1, 1, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Centr", &(centr[2]), &(centr[3]), 1);

				ImPlot::EndPlot();
			}
			ImGui::End();
		}

		{
			ImGui::Begin("Alpha");
			if (ImPlot::BeginPlot("Alpha vs dV", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }))
			{
				auto transformForwardLinear = [](double v, void*)
					{ return std::log(std::abs(v)); };
				auto transformForwardNaturalLog = [](double v, void*)
					{ return std::exp(v); };

				ImPlot::SetupAxes("dV", "Alpha'", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

				if (plotSettings.xLog)
					ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
				if (plotSettings.yLog)
					ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);

				auto& Xdata = AlphaDV.first;
				auto& Ydata = AlphaDV.second;

				ImPlot::PlotScatter("Rs", Xdata.data(), Ydata.data(), Xdata.size() / 2);


				ImPlot::SetNextFillStyle(ImVec4(1, 1, 0, 0.5f), 1.0);
				ImPlot::PlotScatter("Rsch", Xdata.data() + (Xdata.size() / 2), Ydata.data()  + (Xdata.size() / 2), Xdata.size() / 2  - 2);


				ImPlot::EndPlot();
			}
			ImGui::End();
		}

		{
			ImGui::Begin("Char Derivative");
			if (ImPlot::BeginPlot("V vs I'", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }))
			{
				auto transformForwardLinear = [](double v, void*)
					{ return std::log(std::abs(v)); };
				auto transformForwardNaturalLog = [](double v, void*)
					{ return std::exp(v); };

				ImPlot::SetupAxes("V", "I''", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

				if (plotSettings.xLog)
					ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
				if (plotSettings.yLog)
					ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);


				auto& Xdata = JunctionFitMaster::PreFit::ADerivative.first;
				auto& Ydata = JunctionFitMaster::PreFit::ADerivative.second;

				ImPlot::PlotLine("I'", Xdata.data(), Ydata.data(), Xdata.size());


				ImPlot::EndPlot();
			}
			ImGui::End();
		}


		/*{
			ImGui::Begin("Timeline of the fitted characterictic");
			static int time = 0;
			if (ImPlot::BeginPlot("Characteristic", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }))
			{
				auto transformForwardLinear = [](double v, void*)
					{ return std::log(std::abs(v)); };
				auto transformForwardNaturalLog = [](double v, void*)
					{ return std::exp(v); };

				ImPlot::SetupAxes("V", "I", plotSettings.plotBaseFlags, plotSettings.plotBaseFlags);

				if (plotSettings.xLog)
					ImPlot::SetupAxisScale(ImAxis_X1, transformForwardLinear, transformForwardNaturalLog);
				if (plotSettings.yLog)
					ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);


				if (timeline.size() > 0) {
					auto& d = timeline[time];
					ImPlot::PlotLine("Characteristic", d[0].data(), d[1].data(), d[0].size());
				}


				ImPlot::EndPlot();
			}


			ImGui::SliderInt("Iteration", &time, 0, timeline.size() - 1);

			ImGui::End();
		}*/



		if (ImGui::Button("Monte Carlo Simulation"))
			monteCarloEngine.settings.draw = true;

		ImGui::SameLine();
		if (ImGui::Button("Plot MonteCarlo"))
			PlotMonteCarloResults();
		ImGui::PopItemWidth();

		if (m_showSimplexSettings)
			m_showSimplexSettings = true;
		if (m_openedContentBrowserData)
			LoadCharacteristics();

		//
		if (autoRangeSettings)
			AutoRange();
		if (m_openGenerateData)
			GenerateCharacteristic();

		if (monteCarloEngine.settings)
			ShowMonteCarloSettings();
		/*
		ImGui::SameLine();
		if (ImGui::Button("Plot MonteCarlo"))
			PlotMonteCarloResults();
		ImGui::SameLine();
		*/
		if (m_showSimplexSettings)
			SetSimplexSettings();

		ImGui::Separator();
	}
	void FittingTesting::SetRange()
	{
	}
	void FittingTesting::FixRs()
	{
		// auto Voltage = Characteristic::ReturningType::Voltage;
		// auto Current = Characteristic::ReturningType::Current;
		// auto &originalV = m_characteristics[characteristicIndex].get(Voltage, true);
		// auto &originalI = m_characteristics[characteristicIndex].get(Current, true);
		// LinearRegression fitter{};
		// std::valarray<double> V, I;
		// V = std::valarray<double>(originalV.data(), originalV.size());
		// I = std::valarray<double>(originalI.data(), originalI.size());
		// fitter(I, V);
		// m_characteristics[characteristicIndex].parameters[ParametersNames::Rs] = fitter.getB();
		// std::cout << "Rs: (fitter.getB() ) " << m_characteristics[characteristicIndex].parameters[ParametersNames::Rs] << std::endl;
		// std::cout << "Rs: V[0]/I[0] " << V[0] / I[0] << std::endl;

		auto Voltage = Characteristic::ReturningType::Voltage;
		auto Current = Characteristic::ReturningType::Current;
		std::vector<double> originalV = m_characteristics[characteristicIndex].get(Voltage, false);
		std::vector<double> originalI = m_characteristics[characteristicIndex].get(Current, false);
		size_t i = 5;
		std::reverse(originalV.begin(), originalV.end());
		std::reverse(originalI.begin(), originalI.end());
		std::valarray<double> V{ originalV.data(), i }, I{ originalI.data(), i };
		/*for (const auto& [destination, source] : std::views::zip(V, originalV))
			destination = source;
		for (const auto& [destination, source] : std::views::zip(I, originalI))
			destination = source;*/
		auto Rs_s = V / I;
		for (const auto& [Vs, Is, Rs] : std::views::zip(V, I, Rs_s))
			std::cout << "[Vs, Is, Rs]" << Vs << " " << Is << " " << "Vs/Is" << Vs / Is << std::endl;
		double Rs = Rs_s.sum() / Rs_s.size();
		std::cout << " Series resistance: " << Rs << std::endl;
	}
	void FittingTesting::FixRsh()
	{
		auto Voltage = Characteristic::ReturningType::Voltage;
		auto Current = Characteristic::ReturningType::Current;
		auto& originalV = m_characteristics[characteristicIndex].get(Voltage, false);
		auto& originalI = m_characteristics[characteristicIndex].get(Current, false);
		std::valarray<double> V{ 5 }, I{ 5 };
		for (const auto& [destination, source] : std::views::zip(V, originalV))
			destination = source;
		for (const auto& [destination, source] : std::views::zip(I, originalI))
			destination = source;
		auto Rp_s = V / I;
		for (const auto& [i, item] : std::views::enumerate(Rp_s))
			std::cout << i << ": " << item << std::endl;
		double Rp = Rp_s.sum() / Rp_s.size();
		std::cout << " Parallel resistance: " << Rp << std::endl;
		// LinearRegression fitter{};
		// std::valarray<double> V, I;
		// V = std::valarray<double>(originalV.data(), (int)originalV.size());
		// I = std::valarray<double>(originalI.data(), (int)originalI.size());
		// fitter(I, V);
		// m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] = 1 / fitter.getB();
		// std::cout << "Rp: ( 1/ fitter.getAB) ) " << m_characteristics[characteristicIndex].parameters[ParametersNames::Rp] << std::endl;
	}
	void FittingTesting::PreFit() {
		for (const auto& [setUp, init, characteristic] : std::views::zip(prefit.setUps, prefit.initialPoints, prefit.fittingCharacteristics)) {
			prefit.futureResults.push_back(std::async(std::launch::async, PreFit::runOneFit, &prefit.results, &setUp, &init, &characteristic));
		}
	};
	void FittingTesting::PrintResults()
	{
		for (const auto& [checking, item] : std::views::zip(prefit.futureResults, prefit.results))
		{
			if (checking.valid())
			{
				if (checking.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready)
				{
					std::vector<std::string> names{ "A","I0","Rs", "Rp" };
					for (const auto& [name, item] : std::views::zip(names, item.getParameters()))
						std::cout << name << " " << std::scientific << std::setprecision(2) << item << " ";
				}
			}
			std::cout << std::endl;

		}
	};
	void PreFit::Init()
	{
		using namespace NumericStorm::Fitting;
		using namespace JunctionFitMasterFromNS::IVFitting;

		// Generating data
		utils::generateVectorAtGivenRanges(V, 0.01, 3, 0.001);
		utils::generateVectorAtGivenRanges(I, 0.01, 3, 0.001);

		Parameters<4> testParameters({ 1, 5e-8, 5e-6, 5e5 });
		NumericStorm::Fitting::Data data(2);
		data[0] = V;
		data[1] = I;

		Characteristic fittingCharacteristic;
		fittingCharacteristic.get(Characteristic::ReturningType::Voltage) = V;
		fittingCharacteristic.get(Characteristic::ReturningType::Current) = I;
		fittingCharacteristics.assign(18, fittingCharacteristic);

		IVModel()(data, testParameters, 210);

		Parameters<4> minBound({ 0.5, 1e-20, 1e-5, 10 });
		Parameters<4> maxBound({ 20, 1e-3, 100, 1e9 });
		std::vector<double> I0s;
		utils::generateVectorAtGivenRanges(I0s, -20, -2, 1);

		std::vector<Parameters<4>> minBounds(18, minBound);
		std::vector<Parameters<4>> maxBounds(18, maxBound);

		for (const auto& [min, item] : std::views::zip(minBounds, I0s)) {
			IVFittingSetup setUp;
			setUp.maxIteration = numberOfIterations;
			setUp.simplexMin = Parameters<4>({ 0.5, 1 * std::pow(10, item), 1e-5, 10 });
			setUp.simplexMax = Parameters<4>({ 0.5, 9 * std::pow(10, item), 1e-5, 10 });
			setUps.emplace_back(setUp);
			initialPoints.emplace_back(Parameters<4>({ 0.5, 5 * std::pow(10, item), 1e-5, 10 }));
		}
	};


	void PreFit::runOneFit(std::vector<NumericStorm::Fitting::Parameters<4>>* results,
		JunctionFitMasterFromNS::IVFitting::IVFittingSetup* setUp,
		NumericStorm::Fitting::Parameters<4>* initalParams, Characteristic* item)
	{
		auto fitter = JunctionFitMasterFromNS::IVFitting::getFitter(*setUp);
		auto out = fitter.fit(*initalParams, item->originalData, 210);
		std::lock_guard<std::mutex> lock(Mutex);
		results->push_back(out.getParameters());
	};
	void FittingTesting::LoadCharacteristics()
	{

		ImGui::Begin("Load Data Window");
		m_openedContentBrowserData = true;
		if (ImGui::Button("Read all"))
			readAllDataFromDirectory(contentBrowserData.rootPath, contentBrowserData.currentPath);
		ImGui::SameLine();
		if (ImGui::Button("Transfer Read data to Plot"))
		{
			for (auto& item : contentBrowserData.characteristics)
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
		for (auto& directoryEntry : std::filesystem::directory_iterator(contentBrowserData.currentPath))
		{
			const auto& path = directoryEntry.path();
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

	void FittingTesting::readAllDataFromDirectory(const std::filesystem::path& rootPath, const std::filesystem::path& currentPath)
	{
		for (auto& directoryEntry : std::filesystem::directory_iterator(currentPath))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, rootPath);
			std::string fileNameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
				continue;
			else
				contentBrowserData.readCharacteristic(path);
		}
	};
	bool JunctionFitMasterUI::checkExistence(const std::vector<Characteristic>& destination, const std::string& item)
	{
		auto found = std::find_if(destination.begin(), destination.end(),
			[&item](const Characteristic& element)
			{
				return element.name == item;
			});
		bool f = found != destination.end();
		return f;
	}

	void ContentBrowserData::readCharacteristic(const std::filesystem::path& path)
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
		parameters[ParametersNames::Temperature] = read_temperature(pathString);
		name = m_path.filename().string();
		json readCharacteristic;
		try
		{
			readCharacteristic = createJSONObject(pathString);
			from_json(readCharacteristic, *this);
		}
		catch (const std::exception& e)
		{
		}
	}
	json Characteristic::createJSONObject(std::string& path)
	{
		const char* fileName = path.c_str();
		std::ifstream file(fileName);
		std::string jsonString;
		std::string line;
		while (std::getline(file, line))
			jsonString += line;
		return json::parse(jsonString, nullptr, false);
	};

	void Characteristic::to_json(json& j, Characteristic characteristic)
	{
		j = json{ {"voltage", characteristic.get(ReturningType::Voltage)}, {"current", characteristic.originalData.get(ReturningType::Current)}, {"density current", characteristic.get(ReturningType::DensityCurrent)} };
	};
	void Characteristic::from_json(const json& j, Characteristic& iv)
	{
		auto strToDouble = [](const std::string& str)
			{ return std::stod(str); };
		auto transform = [&](const std::vector<std::string>& str, std::vector<double>& vec)
			{
				std::transform(str.begin(), str.end(), vec.begin(), strToDouble);
			};
		iv.resize(j.at("voltage").size());

		transform(j.at("voltage"), iv.get(ReturningType::Voltage));
		transform(j.at("current"), iv.get(ReturningType::Current));
		transform(j.at("density current"), iv.get(ReturningType::DensityCurrent));
	};
	double Characteristic::read_temperature(std::string& path)
	{

		std::stringstream ss(m_path.string());
		std::string token;
		std::vector<std::string> slices;
		char delimiter = '_';
		while (std::getline(ss, token, delimiter))
			slices.push_back(token);

		for (const auto& item : slices)
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
		ImGui::Separator();
		static float voltages[3]{ 0.001f, 3.0f, 0.01 }; // min - max - step
		ImGui::Text("Voltages");
		ImGui::SameLine();
		ImGui::PushID(1);
		static bool init_gen = false;
		if (!init_gen)
		{
			init_gen = true;
			utils::generateVectorAtGivenRanges(generatingData.Voltages, voltages[0], voltages[1], voltages[2]);
		}

		if (ImGui::DragFloat3("##Voltages", (float*)&voltages, 0.1, 0.00001, 4, "%.5f"))
			utils::generateVectorAtGivenRanges(generatingData.Voltages, voltages[0], voltages[1], voltages[2]);
		ImGui::PopID();
		ImGui::PushItemWidth(200);
		for (const char* item : { "name", "min", "max", "step", "value" })
		{
			ImGui::Text(item);
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::PopItemWidth();
		for (const auto& [description, destination, i] : std::views::zip(generatingData.names, generatingData.params, std::ranges::iota_view(0, 5)))
			DrawSingleRangeGenerationOption(description, destination, i, id);

		if (ImGui::Button("Single Shot"))
			SingleShot();

		ImGui::SameLine();
		if (ImGui::Button("Generate Range"))
			GenerateRange();

		ImGui::SameLine();
		if (ImGui::Button("Close"))
			m_openGenerateData = false;
	};
	void FittingTesting::DrawSingleRangeGenerationOption(const std::string& name, Params<3>& destination, const int& i, int& ID)
	{
		std::string itemBaseName = name + std::to_string(i);
		ImGui::PushID(itemBaseName.c_str());

		ImGui::RadioButton(name.c_str(), (int*)&generatingData.option, i);
		ImGui::SameLine();

		ImGui::PopID();
		ImGui::PushItemWidth(200);
		ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		if (name == "I0")
			flags | ImGuiSliderFlags_Logarithmic;
		if (generatingData.option != utils::cast<ParametersNames>(i))
			ImGui::BeginDisabled();
		ImGui::PushID((itemBaseName + "1").c_str());
		ImGui::DragFloat3("##DragFloat3", (float*)&destination.items, 0.1, 0.0f, 0.0f, "%.3f", flags);
		ImGui::PopID();
		if (generatingData.option != utils::cast<ParametersNames>(i))
			ImGui::EndDisabled();

		ImGui::SameLine();
		if (generatingData.option == utils::cast<ParametersNames>(i))
			ImGui::BeginDisabled();
		ImGui::PushID((itemBaseName + "2").c_str());
		ImGui::DragFloat("##DragFloat", &destination.value, 0.1, 0.0f, 10.0f, "%.3f");
		ImGui::PopID();
		if (generatingData.option == utils::cast<ParametersNames>(i))
			ImGui::EndDisabled();
		ImGui::PopItemWidth();
	};
	void FittingTesting::GenerateRange()
	{

		ParametersNames choosen = generatingData.option;
		Params<3> parameters = generatingData[choosen];

		using names = ItemNames;
		unsigned int counts = static_cast<unsigned int>((parameters[names::max] - parameters[names::min]) / parameters[names::step]) + 1;
		std::vector<double> steps(counts);

		utils::generateVectorAtGivenRanges(steps, parameters[names::min], parameters[names::max], parameters[names::step]);
		FourParameters params;
		for (const auto& name : std::ranges::iota_view(0, 4))
			params[utils::cast<ParametersNames>(name)] = generatingData.params[utils::cast(name)].value;

		std::vector<FourParameters> finalParameters(steps.size(), params);
		for (const auto& [destination, item] : std::views::zip(finalParameters, steps))
			destination[choosen] = item;

		Characteristic referenceToCopy;
		referenceToCopy.setAll(generatingData.Voltages);
		std::vector<Characteristic> characteristics{ counts, referenceToCopy };
		for (const auto& [name, item] : std::views::zip(steps, characteristics))
			item.name = std::to_string(name);
		for (const auto& [characteristic, parameters] : std::views::zip(characteristics, finalParameters))
			generate(characteristic, parameters);
	};
	void FittingTesting::SingleShot()
	{
		FourParameters parameters;
		std::vector<double> p{ 1, 5e-8, 5e-5, 5e5 };
		for (const auto& [name, item] : std::views::zip(std::ranges::iota_view(0, 4), p))
		{
			// parameters[utils::cast<ParametersNames>(name)] = item;
			parameters[static_cast<ParametersNames>(name)] = generatingData.params[utils::cast(name)].value;
		}
		parameters.Temperature = init_data.true_data.T;

		parameters.parameters = init_data.true_data.parameters;

		Characteristic characteristic;
		characteristic.parameters = parameters;
		characteristic.setAll(generatingData.Voltages);
		generate(characteristic, parameters);
	};
	void FittingTesting::generate(Characteristic& characteristic, const FourParameters& parameters)
	{
		using namespace JunctionFitMasterFromNS::IVFitting;
		IVModel()(characteristic.originalData, parameters.parameters, parameters.Temperature);
		m_characteristics.push_back(characteristic);
	};

	static double err{ -1.0 };

	void FittingTesting::Fit()
	{



		using namespace JunctionFitMasterFromNS::IVFitting;


		/*Parameters<4> initialPoint{};
		if (m_characteristics.size() > 0) {
			auto& data = m_characteristics[0].originalData;

			auto filt = JunctionFitMaster::PreFit::filter(data);

			Characteristic toAdd;
			toAdd.get(Characteristic::ReturningType::Voltage) = filt[0];
			toAdd.get(Characteristic::ReturningType::Current) = filt[1];
			toAdd.getTemperature() = m_characteristics[0].getTemperature();
			std::string name;
			for (const auto& item : toAdd.parameters.parameters)
				name += std::to_string(item) + "  ";
			toAdd.name = name;
			m_characteristics.push_back(toAdd);

			return;

			auto params = JunctionFitMaster::PreFit::estimate(data, init_data.true_data.T);
			initialPoint = params;
			std::cout << std::scientific << "A: " << params[0] << " I0: " << params[1] << " Rs: " << params[2] << " Rp: " << params[3] << std::fixed << std::endl;
		}*/



		
		IVFittingSetup setUp;
		NumericStorm::Fitting::Data& data = m_characteristics[0].originalData;

		double T = m_characteristics[0].getTemperature();


		std::vector<JFMData> d = JunctionFitMaster::PreFit::correlate(data[0], init_data.true_data.parameters, init_data.true_data.T, AlphaDV.first, AlphaDV.second);

		std::cout << "dV: " << " Alpha: " << std::scientific << std::endl;
		for (const auto& [d, a] : std::views::zip(AlphaDV.first, AlphaDV.second))
			std::cout << d << "   " << a << std::endl;

		for (auto& da : d) {
			
			Characteristic toAdd;
			toAdd.get(Characteristic::ReturningType::Voltage) = da[0];
			toAdd.get(Characteristic::ReturningType::Current) = da[1];
			toAdd.getTemperature() = m_characteristics[0].getTemperature();
			std::string name;
			for (const auto& item : toAdd.parameters.parameters)
				name += std::to_string(item) + "  ";
			toAdd.name = name;
			m_characteristics.push_back(toAdd);
		}


		setUp.simplexMin = init_data.simplex_init.minBounds;
		setUp.simplexMax = init_data.simplex_init.maxBounds;

		/*ivOptimizer = getOptimizer(setUp);
		ivState = ivOptimizer.setUpOptimization(initialPoint, data, T);
		ivState.getSimplexFigure().sort();

		err = ivState.getBestPoint().getError();*/
	};

	

	void FittingTesting::Step(IVSimplexOptimizer<IVModel>& optimizer, typename IVSimplexOptimizer<IVModel>::SettingsT::OptimizerStateT& state) {
		
		if (!optimizer.checkStop(state)) {
			optimizer.oneStep(state);

			double e = std::accumulate(state.getSimplexFigure().begin(), state.getSimplexFigure().end() - 1, 0.0, [](double sum, const auto& item) {return sum + item.getError(); });

			e /= 4;

			if (std::abs(e - state.getBestPoint().getError()) < 1e-1) {
				auto nState = optimizer.setUpOptimization(state.getBestPoint().getParameters(), m_characteristics[0].originalData, m_characteristics[0].getTemperature());
				nState.getIteration() = state.getIteration();
				state = nState;
			}

			state.getSimplexFigure().calculateCentroid();
			centr = state.getSimplexFigure().getCentroid().getParameters().getParameters();

			std::cout << "Iteration" << std::endl;
			for (auto& p : state.getSimplexFigure())
				std::cout << "A: " << p[0] << " I0: " << std::scientific << p[1] << std::fixed << " Rs: " << p[2] << " Rsch: " << p[3] << " Error: " << p.getError() << std::endl;

			for (const auto& [i, param_a] : std::views::enumerate(param_mat)) { // 4
				for (const auto& [src, dest] : std::views::zip(state.getSimplexFigure(), param_a)) { //5
					dest = src[i];
				}
			}
		}
		else if(state.getIteration() < 4000) {
			state.getIteration() = 4100;
			Characteristic toAdd;
			toAdd.setAll(m_characteristics[0].originalData[0]);
			toAdd.parameters.parameters = state.getBestPoint().getParameters();
			toAdd.getTemperature() = m_characteristics[0].getTemperature();
			JunctionFitMasterFromNS::IVFitting::IVModel()(toAdd.originalData, toAdd.parameters.parameters, toAdd.getTemperature());
			std::string name;
			for (const auto& item : toAdd.parameters.parameters)
				name += std::to_string(item) + "  ";
			toAdd.name = name;
			m_characteristics.push_back(toAdd);
		}

	};


	void FittingTesting::AutoRange()
	{
		ImGui::Separator();
		autoRangeSettings.open = true;

		ImGui::PushItemWidth(250);
		for (const auto& [destination, name, ID] : std::views::zip(autoRangeSettings.bounds, autoRangeSettings.names, std::ranges::iota_view(0, 2)))
		{
			std::string itemBaseName = name + std::to_string(ID);
			ImGui::Text(name.c_str());
			ImGui::SameLine();
			ImGui::PushID((itemBaseName + "1").c_str());
			ImGui::DragFloat3("##DragFloat3", (float*)&destination.items, 0.1, 0.0f, 0.0f, "%.3f");
			ImGui::PopID();
		}
		ImGui::PopItemWidth();

		// todo add threads here
		if (ImGui::Button("Auto Range"))
			DoAutoRange();

		if (ImGui::Button("Close"))
			autoRangeSettings.open = false;
	};

	

	void FittingTesting::DoAutoRange()
	{
		std::vector<double> minRanges, maxRanges;

		auto& [Minmin, Minmax, Minstep] = autoRangeSettings.bounds[0].items;
		utils::generateVectorAtGivenRanges(minRanges, Minmin, Minmax, Minstep);

		auto& [Maxmin, Maxmax, Maxstep] = autoRangeSettings.bounds[1].items;
		utils::generateVectorAtGivenRanges(maxRanges, Maxmin, Maxmax, Maxstep);

		//? filtering characteristic to auto range
		std::vector<Characteristic> characteristicToAutoRange;
		for (auto& item : m_characteristics)
			if (item)
				characteristicToAutoRange.push_back(item);
		int i = 1;
		std::for_each(std::execution::seq, characteristicToAutoRange.begin(), characteristicToAutoRange.end(), [&](Characteristic& item)
			{
				using namespace JunctionFitMasterFromNS::IVFitting;
				std::vector<Characteristic> fittingResults;
				for (auto minRange : minRanges)
				{
					for (auto maxRange : maxRanges)
					{

						// todo move this to a single function
						Characteristic item = characteristicToAutoRange[0]; //! replace by characteristic in for loop
						double minI = item.getMin() + minRange;
						double maxI = item.getMax() + maxRange;

						// 2. find nearest point -> get index
						// 3. range characteristic
						std::vector<double> I = item.get(Characteristic::ReturningType::Current);
						item.lowerIndex = std::distance(I.begin(), std::lower_bound(I.begin(), I.end(), minI));
						item.upperIndex = std::distance(I.begin(), std::lower_bound(I.begin(), I.end(), maxI));
						item.updateRangedCharacteristic();

						// 4. fit
						// todo -> this can be extracted to another function which will take settings and characteristic!
						std::array<double, 4> min, max;
						for (const auto& [i, item] : std::views::enumerate(simplexSettings.bounds))
						{
							min[i] = item.items[0];
							max[i] = item.items[1];
						}

						IVFittingSetup setUp{};
						setUp.simplexMin = Parameters<4>(min);
						setUp.simplexMax = Parameters<4>(max);

						Fitter<IVSimplexOptimizer<IVModel>> fitter = getFitter(setUp);
						NumericStorm::Fitting::Data data = m_characteristics[0].rangedData;
						double T = m_characteristics[0].getTemperature();
						auto result = fitter.fit(setUp.simplexMin, data, T);
						item.parameters.parameters = result.getParameters().getParameters();
						std::cout << "i: " << i << " minRange: " << minRange << " maxRange: " << maxRange << " ";
						std::vector<std::string> names{ "A: ","I0: ","Rs: ","Rp: " };
						for (const auto& [i, name] : std::views::zip(names, item.parameters.parameters))
							std::cout << name << item << " ";
						std::cout << std::endl;
					};
					// 5. add to tmp list
					fittingResults.push_back(item);
				}

				// sort list
				std::sort(fittingResults.begin(), fittingResults.end());
				// get best
				std::cout << std::endl;
				std::cout << std::endl;
				std::vector<std::string> names{ "A: ","I0: ","Rs: ","Rp: " };
				for (const auto& [i, name] : std::views::zip(names, fittingResults[0].parameters.parameters))
					std::cout << item << " ";
				std::cout << std::endl;
				std::cout << std::endl;
				for (const auto& [i, name] : std::views::zip(names, fittingResults[fittingResults.size() - 1].parameters.parameters))
					std::cout << item << " ";
				std::cout << std::endl;
				item.lowerIndex = fittingResults[0].lowerIndex;
				item.upperIndex = fittingResults[0].upperIndex;
				item.updateRangedCharacteristic(); });
	};

	void FittingTesting::DoMonteCarloSimulation() {

	};
	void MonteCarloEngine::Simulate(const std::vector<Characteristic>& characteristic) {};
	void MonteCarloEngine::simulate(const Characteristic& item)
	{
		using namespace NumericStorm::Fitting;
		using namespace JunctionFitMasterFromNS::IVFitting;
		MonteCarloResult simulationResults;
		simulationResults.results.resize(settings.numberOfIterations);
		const NumericStorm::Fitting::Data originalData = item.rangedData;
		for (int i = 0; i < settings.numberOfIterations; i++)
		{
		}
	};
	void FittingTesting::ShowMonteCarloSettings()
	{
		auto& settings = monteCarloEngine.settings;
		settings.draw = true;
		ImGui::Separator();
		ImGui::PushItemWidth(150);
		ImGui::InputDouble("noise factor [%]", &settings.noiseFactor, 0.01, 0.1, "%.1f");
		ImGui::SameLine();
		ImGui::InputInt("Number Of Iterations", &settings.numberOfIterations, 1, 100);
		const char* names[]{
			"A",
			"I0",
			"Rs",
			"Rp",
		};
		static int X = 0, Y = 1;
		ImGui::SameLine();
		ImGui::PushItemWidth(50);
		if (ImGui::Combo("X parameter", &X, names, IM_ARRAYSIZE(names)))
			settings.XParameter = utils::cast<ParametersNames>(utils::cast(X));

		ImGui::SameLine();
		if (ImGui::Combo("Y parameter", &Y, names, IM_ARRAYSIZE(names), 4))
			settings.YParameter = utils::cast<ParametersNames>(utils::cast(Y));
		ImGui::PopItemWidth();

		ImGui::ColorEdit4("best color", (float*)&settings.bestErrors, settings.colorsFlags);
		ImGui::SameLine();
		ImGui::ColorEdit4("worse color", (float*)&settings.worserErrors, settings.colorsFlags);
		ImGui::SameLine();
		ImGui::ColorEdit4("worst color", (float*)&settings.worstErrors, settings.colorsFlags);

		if (ImGui::Button("Simulate"))
			DoMonteCarloSimulation();
		static char* path[128];
		// if (ImGui::InputText("Path To Dump", *path, IM_ARRAYSIZE(path),128))
		//	settings.pathToDump = std::filesystem::path(*path);
		ImGui::SameLine();
		if (ImGui::Button("Dump Results"))
			DumpMonteCarloResults();
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			settings.draw = false;
		ImGui::PopItemWidth();
	};
	void FittingTesting::PlotMonteCarloResults()
	{
		std::cout << "Plot MonteCarlo was pressed! " << std::endl;
	};
	void FittingTesting::DumpMonteCarloResults() {};
	void FittingTesting::SetSimplexSettings()
	{
		ImGui::Separator();
		m_showSimplexSettings = true;
		int ID = 0;
		std::vector<std::string> names{ "A   ", "I0  ", "Rs  ", "Rsh " };
		for (const auto& [destination, name, ID] : std::views::zip(simplexSettings.bounds, names, std::ranges::iota_view(0, 4)))
			SimplexSettingsUI(destination, name, ID);
		if (ImGui::Button("Close"))
			m_showSimplexSettings = false;
	};
	void FittingTesting::SimplexSettingsUI(Params<2>& destination, const std::string& name, int ID)
	{
		std::string itemBaseName = name + std::to_string(ID);

		ImGui::PushItemWidth(100);

		ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		std::string format = "%.3f";
		if (name == "I0")
		{
			flags | ImGuiSliderFlags_Logarithmic;
			format = "%.10f";
		}
		// todo add custom config based on the type -> stored in some map
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::PushID((itemBaseName + "1").c_str());
		ImGui::DragFloat2("##DragFloat3", (float*)&destination.items, 0.1, 0.0f, 0.0f, format.c_str(), flags);
		ImGui::PopID();
		ImGui::SameLine();

		ImGui::PushID((itemBaseName + "2").c_str());
		ImGui::DragFloat("##DragFloat", &destination.value, 0.1, 0.0f, 10.0f, "%.3f");
		ImGui::PopID();
		ImGui::PopItemWidth();
	};

	void FittingTesting::DrawTable()
	{
		ImGui::SameLine();

		if (ImGui::ColorEdit4("start color", (float*)&plotSettings.startColor, plotSettings.basicPlotColorFlags))
			setColorsOfCharacteristics();

		ImGui::SameLine();
		if (ImGui::ColorEdit4("end color", (float*)&plotSettings.endColor, plotSettings.basicPlotColorFlags))
			setColorsOfCharacteristics();

		ImGui::SameLine();
		if (ImGui::Button("Update Characteristics Color"))
			setColorsOfCharacteristics();

		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::Spacing();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
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

			for (auto& item : m_characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.name.c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.getTemperature()).c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.name.c_str(), (float*)&item.m_color, tableSettings.basicTableColorFlags);
			}

			ImGui::EndTable();
		}
	}

	void FittingTesting::setColorsOfCharacteristics()
	{

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

// todo MCFdo