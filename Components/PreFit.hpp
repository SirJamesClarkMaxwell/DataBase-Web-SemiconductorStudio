#pragma once
#include "pch.hpp"
#include "Parameters.hpp"
#include "TestingFittingAndMC.hpp"


namespace JunctionFitMaster::PreFit {
	using namespace NumericStorm::Fitting;
	using namespace UI::Data::JunctionFitMasterUI;

	extern std::pair<std::vector<double>, std::vector<double>> ADerivative;

	Parameters<4> estimate(const JFMData& data, double T);

};