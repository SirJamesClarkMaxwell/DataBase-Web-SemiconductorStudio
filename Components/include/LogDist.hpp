#pragma once

#include "pch.hpp"


namespace JunctionFitMaster::Utils {

	class LogDist {
	public:
		static float value(double p, float min, float max) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);

            double u = dis(gen);
            double q = 1.0 - p;
            double term = p / (q * std::log(1.0 - p));
            int k = 1;

            double sum = term;
            while (u > sum) {
                k++;
                term *= p * (k - 1.0) / k;
                sum += term;
            }

            double normalized_value = static_cast<double>(k) / std::numeric_limits<int>::max();
            double scaled_value = min + normalized_value * (max - min);

            return scaled_value;
		}
	};
}