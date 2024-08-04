#include "pch.hpp"
#include "components.hpp"
#include "data.hpp"
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <ranges>
namespace UI::Components
{

	void draw_bg_window()
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

		const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y));
		bool open = true;
		ImGui::Begin("##", &open, window_flags);

		ImGui::End();
	}

	void draw_tester(std::shared_ptr<Data::MyData> state)
	{
		/*
		drawContentBrowserData(state->dataPreview);
		drawPlots(state->dataPreview);
		drawGeneratingCurvesPanel(state->generatingCharacteristicsData);
		*/
		TestFittingAndMC(state->fittingTestingData);
		/*
		ImGui::Begin("Advanced Table");
		Demo_LinePlots();
		drawAdvancedTable();
		ImGui::Text("Hello World");
		ImGui::End();
		ShowSelectableTable();
		*/
		ImGui::ShowDemoWindow();
		ImPlot::ShowDemoWindow();
	}
	bool operator==(const Characteristic &lhs, const Characteristic &rhs) { return lhs.getTemperature() == rhs.getTemperature(); };
	void drawPlots(Data::DataPreview &dataPreview)
	{
		using namespace UI::Data;
		static ImPlotFlags plot_flags = ImPlotAxisFlags_None;
		PlotData &plotData = dataPreview.plotData;
		// plot_flags = plot_flags | ImPlotAxisFlags_AutoFit; // TODO move this property into  some class, probably PlotProperties

		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
			std::sort(plotData.characteristics.begin(), plotData.characteristics.end(), [](Characteristic &a, Characteristic &b)
					  { return a.getTemperature() < b.getTemperature(); });

		if (ImGui::Button("Reverse"))
			std::reverse(plotData.characteristics.begin(), plotData.characteristics.end());

		ImVec2 plot_size(-1, ImGui::GetContentRegionAvail().x * 0.7f);

		if (!ImGui::GetIO().KeyCtrl && ImGui::GetScrollMaxY() > 0)
			plot_flags = plot_flags | ImPlotFlags_NoInputs;

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
		std::tuple<std::string, std::string> axisProperties = plotData.plotProperties.axis;

		PlotProperties &plotProperties = plotData.plotProperties;

		if (ImPlot::BeginPlot("testing Plot", plot_size, plot_flags))
		{
			// setupPlot(plot_flags, plotData);
			ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
			if (!plotData.plotProperties.lin_x_scale)
				ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			if (!plotData.plotProperties.lin_y_scale)
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
			for (auto &item : plotData.characteristics)
				if (item)
					plotOneCharacteristic(item, true);

			ImPlot::EndPlot();
		}
	}
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic, bool log)
	{
		auto &V = characteristic.getVoltage();
		auto &I = !log ? characteristic.getCurrent() : characteristic.getLogCurrent();
		std::string title = "I(V) " + characteristic.name + " K";
		ImPlot::SetNextLineStyle(characteristic.m_color);

		// ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		ImPlot::PlotLine(title.c_str(), V.data(), I.data(), V.size(), ImPlotLineFlags_Segments);
	}
	void setupPlot(ImPlotFlags plot_flags, UI::Data::PlotData &plotData)
	{
		ImPlot::SetupAxes("V", "I");
		if (!plotData.plotProperties.lin_x_scale)
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
		if (!plotData.plotProperties.lin_y_scale)
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
	}
	void drawContentBrowserData(Data::DataPreview &contentBrowserData)
	{

		ImGui::Begin("Content Browser");

		if (ImGui::Button("Read all"))
			readAllDataFromDirectory(contentBrowserData.rootPath, contentBrowserData.currentPath, contentBrowserData.contentBrowserData);
		ImGui::SameLine();
		if (ImGui::Button("Transfer Read data to Plot"))
		{
			for (auto &item : contentBrowserData.contentBrowserData.characteristics)
			{
				if (!checkExistence(contentBrowserData.plotData.characteristics, item.name))
					contentBrowserData.plotData.characteristics.push_back(item);
			}
		}
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
					contentBrowserData.contentBrowserData.readCharacteristic(path);
			}
		}

		ImGui::Begin("Plotting Settings ");

		if (ImGui::Button("Plot all"))
		{
			for (Characteristic &item : contentBrowserData.contentBrowserData.characteristics)
				contentBrowserData.plotData.addCharacteristic(item);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove All"))
		{
			for (Characteristic &item : contentBrowserData.contentBrowserData.characteristics)
				item.selected = false;
		}
		auto &plotData = contentBrowserData.plotData;

		ImGui::SameLine();

		if (ImGui::ColorEdit4("start color", (float *)&plotData.startColor, plotData.colorFlags))
		{
			plotData.setColorsOfCharacteristics();
			plotData.setColorsOfGraph();
		}
		ImGui::SameLine();
		if (ImGui::ColorEdit4("end color", (float *)&plotData.endColor, plotData.colorFlags))
		{
			plotData.setColorsOfCharacteristics();
			plotData.setColorsOfGraph();
		}
		ImGui::SameLine();
		if (ImGui::Button("Update Characteristics Color"))
		{
			plotData.setColorsOfCharacteristics();
			plotData.setColorsOfGraph();

			// static std::vector<ImU32> colorMap;

			// if (plotData.characteristics.size() > 1)
			// {
			// 	auto func = [&](const ImColor &item)
			// 	{
			// 		return ImGui::ColorConvertFloat4ToU32(item);
			// 	};

			// 	plotData.plotProperties.colorMap.resize(plotData.plotProperties.colors.size());
			// 	std::transform(plotData.plotProperties.colors.begin(), plotData.plotProperties.colors.end(), plotData.plotProperties.colorMap.begin(), func);
			// 	plotData.plotProperties.colorMapPointer = plotData.plotProperties.colorMap.data();
			// 	plotData.plotProperties.addedColorMap = -1;
			// 	const ImColor diff = plotData.endColor - plotData.startColor;
			// 	plotData.plotProperties.name = ImGui::ColorConvertFloat4ToU32(diff);
			// }
		}
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::Spacing();

		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);

		ImU32 col_a_ = ImGui::ColorConvertFloat4ToU32(plotData.startColor);
		ImU32 col_b_ = ImGui::ColorConvertFloat4ToU32(plotData.endColor);
		draw_list->AddRectFilledMultiColor(p0, p1, col_a_, col_b_, col_b_, col_a_);

		ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight()));
		if (ImGui::BeginTable("Characteristic", 4, contentBrowserData.plotData.flags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Temperature");
			ImGui::TableSetupColumn("Color");
			ImGui::TableHeadersRow();

			for (auto &item : contentBrowserData.plotData.characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.name.c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.getTemperature()).c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.name.c_str(), (float *)&item.m_color, contentBrowserData.plotData.colorFlags);
			}

			ImGui::EndTable();
		}

		ImGui::End();
		ImGui::End();
	}
	void readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath, ContentBrowserData &contentBrowserData)
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

	void Demo_LinePlots()
	{
		static float xs1[1001], ys1[1001];
		for (int i = 0; i < 1001; ++i)
		{
			xs1[i] = i * 0.001f;
			ys1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
		}

		static float xs3[1001], ys3[1001];
		for (int i = 0; i < 1001; ++i)
		{
			xs3[i] = i * 0.001f;
			ys3[i] = 0.5f + 0.5f * sinf(50 * (xs3[i] + (float)ImGui::GetTime() / 5));
		}
		static double xs2[20], ys2[20];
		for (int i = 0; i < 20; ++i)
		{
			xs2[i] = i * 1 / 19.0f;
			ys2[i] = xs2[i] * xs2[i];
		}
		static ImU32 colorDataRGB[3] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};
		static int customRGBMap;
		static int i = 0;
		if (i == 0)
		{
			customRGBMap = ImPlot::AddColormap("RGBColors", colorDataRGB, 3);
			i++;
		}
		/*
				int customRGBMap2 = ImPlot::GetColormapIndex("RGBColors");
				if (customRGBMap2 >= ImPlot::GetColormapCount())
		*/
		ImPlot::PushColormap(customRGBMap);

		if (ImPlot::BeginPlot("Line Plots"))
		{
			ImPlot::SetupAxes("x", "y");
			ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine("g(x)", xs2, ys2, 20, ImPlotLineFlags_Segments);
			ImPlot::PlotLine("h(x)", xs3, ys3, 20, ImPlotLineFlags_Segments);
			ImPlot::EndPlot();
			/*
			int customRGBMap1 = ImPlot::GetColormapIndex("RGBColors");
			if (customRGBMap1 > ImPlot::GetColormapCount())
			{
			*/
			ImPlot::PopColormap();
			//}
		}
	}
	void drawGeneratingCurvesPanel(GeneratingCharacteristicsPanelData &data)
	{
#define string(x) std::to_string(x)
		ImGui::Begin("Enter Parameters");

		static float VStart = 0;
		static float VEnd = 3;
		static float Step = 0.01;
		ImGui::DragFloat("V-start", &VStart, 1, 0, 5);
		ImGui::DragFloat("V-end", &VEnd, 1, 0, 5);
		ImGui::DragFloat("Step", &Step, 0.01, 0.0001, 0.1);

		SixParameters &sixParameters = data.sixParameters;
		ImGui::PushItemWidth(200);
		ImGui::Separator();
		ImGui::Text("Four parameter Model");
		static double I0 = 1e-10;
		static double A = 1;
		static double Rs = 1e-6;
		static double Rch = 1e7;
		static double Temperature = 300;
		ImGui::InputDouble("I0", &I0, 0.01f, 1.0f, "%.10f");
		ImGui::InputDouble("A", &A, 0.1f, 1.0f, "%.8f");
		ImGui::InputDouble("Rs", &Rs, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble("Rch", &Rch, 100.0f, 1.0f, "%.8f");
		ImGui::InputDouble("T", &Temperature, 10.0f, 1.0f, "%.8f");
		ImVec4 color;
		ImGui::ColorEdit4("color", (float *)&color);
		if (ImGui::Button("Add 4 Parameter characteristic"))
		{
			FourParameters &fourParameters = data.fourParameters;
			fourParameters.I0 = std::pow(10, I0);
			Characteristic characteristic;
			std::vector<double> Voltage = generate_range(VStart, VEnd, Step);
			characteristic.getVoltage() = Voltage;
			characteristic.getCurrent() = Voltage;
			characteristic.getDensityCurrent() = Voltage;
			auto toString = [](FourParameters c, double T)
			{ return string(c.I0) + string(c.A) + string(c.Rs) + string(c.Rch) + string(T); };
			characteristic.name = toString(fourParameters, Temperature);

			// double &I0 = fourParameters.I0;
			// double &A = fourParameters.A;
			// double &Rs = fourParameters.Rs;
			// double &Rch = fourParameters.Rch;
			// double &T = fourParameters.Temperature;
			GeneratingCharacteristicsPanelData::CalculateCurrent(characteristic, 1, 1e-8, 1e-6, 1e7, Temperature);
			characteristic.fourParameters = fourParameters;
			// Characteristic characteristicToPush(characteristic);
			data.characteristics.push_back(characteristic);
			// std::cout << "Characteristic with this parameters has been pushed" << "I0: " << I0 << " "
			//		  << "A: " << A << " "
			//		  << "Rs: " << Rs << " "
			//		  << "Rch: " << Rch << " "
			//		  << " name: " << characteristic.name << std::endl;
		}

		ImGui::Separator();
		ImGui::Text("Six parameter Model");
		ImGui::InputDouble(" I02", &sixParameters.I0, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble(" A2", &sixParameters.A, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble(" Rs2", &sixParameters.Rs, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble(" Rch2", &sixParameters.Rch, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble(" Rch22", &sixParameters.Rch2, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble(" alpha2", &sixParameters.alpha, 0.01f, 1.0f, "%.8f");
		ImGui::InputDouble("T2", &sixParameters.Temperature, 10.0f, 1.0f, "%.8f");
		if (ImGui::Button("Add 6 Parameter characteristic"))
		{
			// static Characteristic characteristic1;
			///*
			// auto current{Voltage};
			// const double k = 8.6e-5;*/
			///*auto diode_current = [](double V, double I0, double A)
			//{
			//	return I0 * (std::exp(V / (A * k * 300)));
			//};*/

			// auto diode_current = [](double V, double I0, double A)
			//	{
			//		double x = -std::exp(-V);
			//		std::complex<double> c = { x, 0 };
			//		std::complex<double> c1 = LamberWN::LambertW(c, 0);;
			//		return
			//	};
			// for (int i = 0; i < Voltage.size(); i++)
			//	current[i] = diode_current(Voltage[i], 1e-10, 1);

			// characteristic1.getVoltage() = Voltage;
			// characteristic1.getCurrent() = current;
			// characteristic1.name = "simple current";
			// characteristic1.m_temperature = 300;
			// data.characteristics.push_back(characteristic1);
		}

		ImGui::Separator();
		ImGui::PopItemWidth();

		if (ImGui::BeginTable("characteristics", 5, ImGuiTableFlags_None | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Temperature");
			ImGui::TableSetupColumn("A, I0, Rs, Rch");
			ImGui::TableSetupColumn("Color");
			ImGui::TableHeadersRow();

			for (auto &item : data.characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.name.c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.getTemperature()).c_str());
				ImGui::TableNextColumn();

				double &A = item.fourParameters.A;
				double &I0 = item.fourParameters.I0;
				double &Rs = item.fourParameters.Rs;
				double &Rsh = item.fourParameters.Rch;
				static std::string text =
					string(A) + string(I0) + string(Rs) + string(Rsh);
				ImGui::Text(text.c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.name.c_str(), (float *)&item.m_color);
			}

			ImGui::EndTable();
		}

		ImGui::End();
		ImGui::Begin("Plotting Area");
		using namespace UI::Data;
		static ImPlotFlags plot_flags = ImPlotAxisFlags_None;

		PlotProperties &plotProperties = data.plotProperties;
		ImVec2 plot_size(-1, ImGui::GetContentRegionAvail().x * 0.7f);

		ImGui::SameLine();
		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImGui::SameLine();
		if (ImGui::Button("clear characteristics"))
			data.characteristics.clear();
		static bool log = false;
		ImGui::Checkbox("log", &log);

		if (ImPlot::BeginPlot("testing Plot", plot_size, plot_flags))
		{
			auto transformForwardLinear = [](double v, void *)
			{
				return std::log(v);
			};

			auto transformForwardNaturalLog = [](double v, void *)
			{
				return std::exp(v);
			};
			// setupPlot(plot_flags, plotData);
			ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
			if (!plotProperties.lin_x_scale)
				ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			if (log)
				ImPlot::SetupAxisScale(ImAxis_Y1, transformForwardLinear, transformForwardNaturalLog);

			for (auto &item : data.characteristics)
				if (item)
					plotOneCharacteristic(item, false);
			ImPlot::EndPlot();
		}

		ImGui::End();

		LamberWWindow(plot_size, plot_flags, plotProperties);
	}
	void LamberWWindow(ImVec2 &plot_size, ImPlotFlags plot_flags, UI::Data::PlotProperties &plotProperties)
	{
		ImGui::Begin("Lambert W");
		static float WUpStart = -std::exp(-1);
		static float WUpEnd = 1;
		static float WUpStep = 0.005;
		static bool UpUseSameArguments = false;
		static bool UPuseReal = true;
		static bool UPbranch0 = true;
		static bool UPswap = true;
		ImGui::PushItemWidth(200);
		ImGui::Separator();
		ImGui::DragFloat("Up Start", &WUpStart, 0.05, -std::exp(-1), 5);
		ImGui::SameLine();
		ImGui::DragFloat("Up End", &WUpEnd, 0.05, WUpStart, 5);
		ImGui::SameLine();
		// ImGui::DragFloat("Up Step", &WUpStep, 0.0001f, 0.01, 0.2);
		// ImGui::SameLine();
		ImGui::Checkbox("Up useSameArguments", &UpUseSameArguments);
		ImGui::SameLine();
		ImGui::Checkbox("Up useReal", &UPuseReal);
		ImGui::SameLine();
		ImGui::Checkbox("Up branch 0", &UPbranch0);
		ImGui::SameLine();
		ImGui::Checkbox("Up swap", &UPswap);
		ImGui::PopItemWidth();

		std::vector<double> Up_x = generate_range(WUpStart, WUpEnd, WUpStep);
		std::vector<double> Up_y;
		Up_y.resize(Up_x.size());
		for (int i = 0; i < Up_x.size(); i++)
		{
			std::complex<double> z;
			if (UpUseSameArguments)
				z = {Up_x[i], Up_x[i]};
			else
				z = {Up_x[i], 0};
			std::complex<double> output;
			if (UPbranch0)
				output = LamberWN::LambertW(z, 0);
			else
				output = LamberWN::LambertW(z, 1);

			if (UPuseReal)
				Up_y[i] = output.real();
			else
				Up_y[i] = output.imag();
		}
		///////////////////////////////

		//////////////////////////////
		static float WDownStart = -std::exp(-1);
		static float WDownEnd = -3;
		static float WDownStep = 0.005;
		static bool DownUseSameArguments = false;
		static bool DownUseReal = true;
		static bool DownBranch0 = true;
		static bool DownSwap = false;
		ImGui::PushItemWidth(200);
		ImGui::Separator();
		ImGui::DragFloat("Down Start", &WDownStart, 0.05, -10, -std::exp(-1));
		ImGui::SameLine();
		ImGui::DragFloat("Down End", &WDownEnd, 0.05, -std::exp(-1), WDownStart);
		ImGui::SameLine();
		// ImGui::DragFloat("Down Step", &WDownStep, 0.0001f, 0.01, 0.2);
		// ImGui::SameLine();
		ImGui::Checkbox("Down useSameArguments", &DownUseSameArguments);
		ImGui::SameLine();
		ImGui::Checkbox("Down useReal", &DownUseReal);
		ImGui::SameLine();
		ImGui::Checkbox("Down branch 0", &DownBranch0);
		ImGui::SameLine();
		ImGui::Checkbox("Down swap", &DownSwap);

		ImGui::PopItemWidth();
		std::vector<double> Down_x = generate_range(WDownStart, WDownEnd, WDownStep);
		std::vector<double> Down_y;
		Down_y.resize(Down_x.size());
		for (int i = 0; i < Down_x.size(); i++)
		{
			std::complex<double> z;
			if (DownUseSameArguments)
				z = {Down_x[i], Down_x[i]};
			else
				z = {Down_x[i], 0};
			std::complex<double> output;
			if (DownBranch0)
				output = LamberWN::LambertW(z, 0);
			else
				output = LamberWN::LambertW(z, 1);

			if (DownUseReal)
				Down_y[i] = output.real();
			else
				Down_y[i] = output.imag();
		}

		if (ImPlot::BeginPlot("Controlled Lambert W", plot_size, plot_flags))
		{
			ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
			if (!plotProperties.lin_x_scale)
				ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			if (!plotProperties.lin_y_scale)
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);

			ImPlot::SetNextLineStyle(ImVec4(1, 0, 0, 1));
			if (!UPswap)
				ImPlot::PlotLine("Up", Up_x.data(), Up_y.data(), Up_x.size(), ImPlotLineFlags_None);
			else
				ImPlot::PlotLine("Up", Up_y.data(), Up_x.data(), Up_x.size(), ImPlotLineFlags_None);

			ImPlot::SetNextLineStyle(ImVec4(0, 0, 1, 1));
			if (!DownSwap)
				ImPlot::PlotLine("Down", Down_x.data(), Down_y.data(), Down_x.size(), ImPlotLineFlags_None);
			else
				ImPlot::PlotLine("Down", Down_y.data(), Down_x.data(), Down_x.size(), ImPlotLineFlags_None);

			ImPlot::EndPlot();
		}

		ImGui::End();
	}
	void TestFittingAndMC(JunctionFitMasterUI::FittingTesting &data)
	{

		ImGui::Begin("Plotting");
		data.DrawPlotData();
		ImGui::End();

		ImGui::Begin("Actions");
		data.DrawActionsPanel();
		ImGui::End();
		ImGui::Begin("Table");
		data.DrawTable();
		ImGui::End();

		//! NumericStormWrapper
		// todo: 4ParameterModel

		// todo: load characteristic
		// todo: display in table
		// todo: plot it

		// todo: generating I(V) based on the parameters
		// todo: adding noise
		// todo: set starting parameters
		// todo: set color
		// todo: (auto)range
		// todo: MC

		//* later
		// todo: plot MC
	};

};