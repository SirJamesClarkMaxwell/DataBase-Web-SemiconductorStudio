#include "pch.hpp"
#include "IVFitter.hpp"
#include "PreFit.hpp"


namespace JunctionFitMaster::PreFit {

	using M = JunctionFitMasterFromNS::IVFitting::IVModel;

	std::pair<std::vector<double>, std::vector<double>> ADerivative{};

	double dV{ 0.0 };

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
		for (size_t i = 0; i < ADerivative.second.size(); i++)
			if (ADerivative.second[i] > maxDer) {
				maxDer = ADerivative.second[i];
				maxDerIndex = i;
			}

		size_t dVB{ maxDerIndex };
		size_t dVE{ maxDerIndex };
		double tol{ 0.95 };
		while (ADerivative.second[dVB] > maxDer * tol || ADerivative.second[dVE] > maxDer * tol) {
			if (ADerivative.second[dVB] > maxDer * tol) dVB--;
			if (ADerivative.second[dVE] > maxDer * tol) dVE++;
			//assert(dVB < ADerivative.second.size());
			if (dVB > ADerivative.second.size() - 1 || dVE > ADerivative.second.size() - 1)
				break;
		}

		dV = V[AStart + dVE] - V[AStart + dVB];

		const double k = 8.6e-5;

		double A = 1 / (k * T * maxDer);

		//A *= 0.85;

		//log(I) = V / (A  * k * T) + log(I0)
		//I0 = I / exp(V / (A * k * T))
		//double I0 = I[maxDerIndex + AStart] / std::exp(V[maxDerIndex + AStart] / (A * k * T));
		double l = logI[maxDerIndex] - V[maxDerIndex + AStart] / (A * k * T);

		double I0 = std::exp(l);

		result[0] = A;
		result[1] = I0;

		return result;
	}


	std::vector<JFMData> correlate(const std::vector<double>& voltages, Parameters<4>& true_params, double T, std::vector<double>& dVs, std::vector<double>& alphas) {
		std::vector<double> Rs{};
		std::vector<double> Rsch{};

		dVs.clear();
		alphas.clear();

		M m{};
		double ko = std::pow(10, (double)1 / 40);
		for (double R = 100; R <= 1e9;) {
			Rsch.push_back(R);
			R = 1e9;
			Rsch.push_back(R);
			break;
			/*if (R < 1e2) R *= ko;
			else R += (1e9 - 100.0) / 10000.0;*/
		}

		for (double R = 1e-7; R <= 100;) {
			Rs.push_back(R);
			R = 100;
			Rs.push_back(R);
			break;
			/*if (R < 1e-1) R *= ko;
			else R += (100.0 - 1.0) / 10000.0;*/
		}

		std::vector<JFMData> dataSets{};

		for (double R : Rs) {
			Parameters<4> p = true_params;
			p[2] = R;
			JFMData data{};
			data[0] = voltages;
			data[1] = voltages;

			m(data, p, T);
			dataSets.push_back(data);
		}

		for (double R : Rsch) {
			Parameters<4> p = true_params;
			p[3] = R;
			JFMData data{};
			data[0] = voltages;
			data[1] = voltages;

			m(data, p, T);
			dataSets.push_back(data);
		}


		for (auto& st : dataSets) {
			alphas.push_back(true_params[0] / estimate(st, T)[0]);
			dVs.push_back(dV);
		}

		
		std::unordered_map<double, std::vector<double>> dVtoA{};
		for (const auto& [i, v] : std::views::zip(alphas, dVs)) {
			dVtoA[v].push_back(i);
		}

		size_t num{ 0 };
		for (const auto& [k, v] : dVtoA) {
			dVs[num] = k;
			alphas[num] = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
			num++;
		}
		dVs.resize(num);
		alphas.resize(num);
		for (const auto& [v, a] : std::views::zip(dVs, alphas)) {
			std::cout << std::scientific << v << "\t" << a << std::endl;
		}
		return dataSets;
	}

	JFMData filter(const JFMData& input) {
		double sum{ 0.0 };

		size_t start{ 0 };

		for (const auto& [i, v] : std::views::zip(input[0], input[1]))
			if (i < 0.0 || v < 0.0)
				start++;

		std::vector<double> logI{};

		std::transform(input[1].begin() + start, input[1].end(), std::back_inserter(logI), [](double i) {return std::log(i); });

		JFMData result{};

		result[0].resize(input[0].size() - start);
		std::copy(input[0].begin() + start, input[0].end(), result[0].begin());
		result[1].resize(input[1].size() - start);
		const unsigned int N{ 4 };

		for (size_t i = 0; i < result[1].size(); i++) {
			sum += logI[i];
			if (i >= N) {
				sum -= logI[i - N];
			}
			result[1][i] = sum / std::min(i + 1, (size_t)N);
		}


		std::transform(result[1].begin(), result[1].end(), result[1].begin(), [](double i) {return std::exp(i); });


		auto derivate = [&](const std::vector<double>& v, const std::vector<double>& c, std::pair<std::vector<double>, std::vector<double>>& result) {
			double der{ 0.0 };

			result.first.clear();
			result.second.clear();

			for (size_t i = 0; i < c.size() - 2; i++) {
				der = (c[i + 2] - c[i]) / (v[i + 2] - v[i]);
				result.first.push_back(v[i + 1]);
				result.second.push_back(der);
			}
			};

		derivate(result[0], logI, ADerivative);

		double avg{ 0.0 };
		size_t ind{ 0 };

		do {
			avg *= ind;
			avg += ADerivative.second[ADerivative.second.size() - 1 - ind];
			avg /= ++ind;
		} while (ADerivative.second[ADerivative.second.size() - 1 - ind] / avg < 5.0);

		ind = ADerivative.second.size() - 1 - ind;
		//ind stores the end point of the data set

		return result;
	}

	void filter(std::span<double>& V, std::span<double>& I) {

	}

	double Acoeff(double dV) {
		return 1.0;
	}

	//Parameters<4> preFit(JFMData& data, double T) {
	//	//precut
	//	//-- remove negatives
	//	//filter
	//	//--running average
	//	//cut
	//	//--derivative
	//	//Rs and Rsch
	//	//A
	//	//I0
	//	
	//	//precut
	//	size_t preCut{ 0 };
	//	auto& V = data[0];
	//	auto& I = data[1];

	//	for (const auto& [v, i] : std::views::zip(V, I))
	//		if (i < 0.0 || v < 0.0)
	//			preCut++;


	//	std::span<double> Vspan{ V.begin() + preCut, V.size() - preCut };
	//	std::span<double> Ispan{ I.begin() + preCut, I.size() - preCut };


	//	//filter



	//}



};