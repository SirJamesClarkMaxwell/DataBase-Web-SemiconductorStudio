#include "pch.hpp"
#include "PreFit.hpp"


namespace JunctionFitMaster::PreFit {

	std::pair<std::vector<double>, std::vector<double>> ADerivative{};

	Parameters<4> estimate(const JFMData& data, double T) {

		auto& I = data[1];
		auto& V = data[0];
		Parameters<4> result{};

		size_t RpStart{ 0 }, RsStart{ 0 };
		size_t AStart{ 0 }, AEnd{ 0 };

		for (const auto& [i, v] : std::views::zip(V, I))
			if (i < 0.0 || v < 0.0)
				RpStart++;

		RsStart = std::min(V.size() - 1, I.size() - 1);

		double S{ 0.0 }, S0{ 0.0 };

		//getting Rp
		size_t n{ RpStart + 2 };
		do {
			if (n >= V.size() || n >= I.size() || I.size() == 0 || V.size() == 0)
				break;

			S = (V[n - 1] - V[n - 2]) / (I[n - 1] - I[n - 2]);
			S0 = (V[n - 1] - V[RpStart]) / (I[n - 1] - I[RpStart]);

			n++;
		} while (S / S0 >= 0.8);

		AStart = n;

		result[3] = S0;


		//getting Rs
		n = RsStart - 2;
		do {
			if (n == 0 || n == AStart)
				break;


			S = (V[n + 2] - V[n + 1]) / (I[n + 2] - I[n + 1]);
			S0 = (V[RsStart] - V[n + 1]) / (I[RsStart] - I[n + 1]);

			n--;
		} while (S / S0 <= 0.8);

		AEnd = n;

		result[2] = S0;

		//getting A

		std::vector<double> logI{};

		std::transform(I.begin() + AStart, I.begin() + AEnd, std::back_inserter(logI), [](double i) {return std::log(i); });

		auto derivate = [&](const std::vector<double>& v, const std::vector<double>& c, std::pair<std::vector<double>, std::vector<double>>& result) {
			double der{ 0.0 };

			result.first.clear();
			result.second.clear();

			for (size_t i = 0; i < c.size() - 2; i++) {
				der = (c[i + 2] - c[i]) / (v[i + 2 + AStart] - v[i + AStart]);
				result.first.push_back(v[i + 1 + AStart]);
				result.second.push_back(der);
			}
			};

		derivate(V, logI, ADerivative);

		double maxDer{ 0.0 };
		size_t maxDerIndex{ 0 };
		for(size_t i = 0; i < ADerivative.second.size(); i++)
			if (ADerivative.second[i] > maxDer) {
				maxDer = ADerivative.second[i];
				maxDerIndex = i;
			}

		const double k = 8.6e-5;

		double A = 1 / (k * T * maxDer);

		A *= 0.85;

		//log(I) = V / (A  * k * T) + log(I0)
		//I0 = I / exp(V / (A * k * T))
		//double I0 = I[maxDerIndex + AStart] / std::exp(V[maxDerIndex + AStart] / (A * k * T));
		double l = logI[maxDerIndex] - V[maxDerIndex + AStart] / (A * k * T);

		double I0 = std::exp(l);

		result[0] = A;
		result[1] = I0;

		return result;
	}
};