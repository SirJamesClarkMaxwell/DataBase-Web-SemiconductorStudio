#pragma once
#include "pch.hpp"

class LinearRegression
{

public:
    LinearRegression() = default;
    void operator()(const std::valarray<double> &dataX, const std::valarray<double> &dataY)
    {
        calculate(dataX, dataY);
    };
    double getA() { return A; };
    double getB() { return B; };

private:
    double r = 0, rsquared = 0, A = 0, B = 0, x = 0, y = 0;
    double yHat = 0, yBar = 0, xBar = 0;
    double SSR = 0, SSE = 0, SST = 0;
    double residualSE = 0, residualMax = 0, residualMin = 0, residualMean = 0, t = 0;
    double SEBeta = 0, sample = 0;
    std::valarray<double> residuals;
    double WN1 = 0, WN2 = 0, WN3 = 0, WN4 = 0, Sy = 0, Sx = 0;

    float mean(const std::valarray<double> &data)
    {
        return std::accumulate(std::begin(data), std::end(data), 0.0) / data.size();
    }
    void calculate(const std::valarray<double> &dataX, const std::valarray<double> &dataY)
    {
        if (dataX.size() != dataY.size())
            throw std::invalid_argument("data has to have the same number of elements");

        size_t sampleSize = dataX.size();
        xBar = mean(dataX);
        yBar = mean(dataY);

        // Calculate r correlation using modern C++ ranges
        for (const auto &[x, y] : std::views::zip(dataX, dataY))
        {
            WN1 += (x - xBar) * (y - yBar);
            WN2 += std::pow((x - xBar), 2);
            WN3 += std::pow((y - yBar), 2);
        }

        WN4 = WN2 * WN3;
        r = WN1 / (std::sqrt(WN4));

        // Calculate A and B
        Sy = std::sqrt(WN3 / (sampleSize - 1));
        Sx = std::sqrt(WN2 / (sampleSize - 1));
        B = r * (Sy / Sx);
        A = yBar - B * xBar;

        // Calculate SSR, SSE, R-Squared, residuals
        residuals.resize(sampleSize);
        std::valarray<double> yHats = A + B * dataX;

        std::valarray<double> difSSE = yHats - yBar;
        SSE = std::accumulate(std::begin(difSSE), std::end(difSSE), 0.0, [](double sum, double val)
                              { return sum + std::pow(val, 2); });

        std::valarray<double> difSSR = dataY - yHats;
        SSR = std::accumulate(std::begin(difSSR), std::end(difSSR), 0.0, [](double sum, double val)
                              { return sum + std::pow(val, 2); });

        residuals = dataY - yHats;
        residualMax = residuals.max();
        residualMin = residuals.min();
        residualMean = std::accumulate(std::begin(residuals), std::end(residuals), 0.0, [](double sum, double val)
                                       { return sum + std::fabs(val); }) /
                       sampleSize;

        SST = SSR + SSE;
        rsquared = SSE / SST; // Can also be obtained by r^2 for simple regression (i.e. 1 independent variable)

        // Calculate T-test for Beta
        residualSE = std::sqrt(SSR / (sampleSize - 2));
        SEBeta = residualSE / (Sx * std::sqrt(sampleSize - 1));
        t = B / SEBeta;
    }
};