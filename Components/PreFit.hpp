#pragma once
#include "pch.hpp"
#include "Parameters.hpp"
#include "TestingFittingAndMC.hpp"


namespace JunctionFitMaster::PreFit {
	using namespace NumericStorm::Fitting;
	using namespace UI::Data::JunctionFitMasterUI;

	extern std::pair<std::vector<double>, std::vector<double>> ADerivative;

	Parameters<4> estimate(const JFMData& data, double T);

	std::vector<JFMData> correlate(const std::vector<double>& voltages, Parameters<4>& true_params, double T, std::vector<double>& dVs, std::vector<double>& alphas);

	JFMData filter(const JFMData& data);

	Parameters<4> preFit(JFMData& data, double T);

};