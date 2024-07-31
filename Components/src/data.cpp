#include "pch.hpp"
#include "../include/storage.hpp"
#include "LambertW.h"
// #include "stdafx.h"
using namespace UI::Data;

void UI::Data::Characteristic::readData()
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

std::vector<double> &Characteristic::getLogCurrent()
{
    auto logf = [](double I)
    { return std::log(std::abs(I)); };
    std::vector<double> logI{I};
    std::transform(I.begin(), I.end(), logI.begin(), logf);
    return logI;
}

bool UI::Data::checkExistence(const std::vector<Characteristic> &destination, const std::string &item)
{
    auto found = std::find_if(destination.begin(), destination.end(),
                              [&item](const Characteristic &element)
                              {
                                  return element.name == item;
                              });
    bool f = found != destination.end();
    return f;
}

void Characteristic::resize(int value)
{
    V.resize(value);
    I.resize(value);
    J.resize(value);
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
}

void Characteristic::to_json(json &j, const Characteristic characteristic)
{

    j = json{{"voltage", characteristic.V}, {"current", characteristic.I}, {"density current", characteristic.J}};
}

void Characteristic::from_json(const json &j, Characteristic &iv)
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
}

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
}

void PlotData::addCharacteristic(Characteristic &item)
{
    if (!checkExistence(characteristics, item.name))

        characteristics.push_back(item);

    item.selected = true;
}

void PlotData::removeCharacteristic(Characteristic &item)
{
    if (checkExistence(characteristics, item.name))
    {
        auto it = std::find(characteristics.begin(), characteristics.end(), item);
        if (it != characteristics.end())
            characteristics.erase(it);
    }
    item.selected = false;
}

Characteristic &UI::Data::PlotData::operator[](const std::string &name)
{

    auto it = std::find_if(characteristics.begin(), characteristics.end(),
                           [&name](const Characteristic &element)
                           {
                               return element.name == name;
                           });
    if (it != characteristics.end())
        return *it;
}

void UI::Data::PlotData::setColorsOfCharacteristics()
{
    // ImColor colorStep = (endColor - startColor) / (characteristics.size() - 1);
    // for (const auto &[index, item] : std::views::enumerate(characteristics))
    //     item.m_color = startColor + index * colorStep;
    // todo extract this code to gconcrete functions
    size_t numChars = characteristics.size();
    for (size_t i = 0; i < numChars; ++i)
    {
        float t = static_cast<float>(i) / (numChars - 1); // Calculate the interpolation factor (0 to 1)
        characteristics[i].m_color = ImColor(
            startColor.Value.x * (1.0f - t) + endColor.Value.x * t,
            startColor.Value.y * (1.0f - t) + endColor.Value.y * t,
            startColor.Value.z * (1.0f - t) + endColor.Value.z * t,
            startColor.Value.w * (1.0f - t) + endColor.Value.w * t);
    }
}

void UI::Data::PlotData::setColorsOfGraph()
{
    plotProperties.colors.resize(characteristics.size());
    std::transform(characteristics.begin(), characteristics.end(), plotProperties.colors.begin(), [&](const Characteristic &item)
                   { return item.m_color; });
}

void ContentBrowserData::readCharacteristic(const std::filesystem::path &path)
{
    Characteristic characteristic(path);
    if (!checkExistence(characteristics, characteristic.name))
        characteristics.push_back(characteristic);
}

void UI::Data::GeneratingCharacteristicsPanelData::CalculateCurrent(Characteristic &characteristic, double A, double I0, double Rs, double Rch, double Temperature)
{

    const double k = 8.6e-5;         // Boltzmann constant in J/K
    const double q = 1.60217662e-19; // Charge of an electron in C
    const double T = Temperature;
    const double V_t = k * T / q; // Thermal voltage
    for (auto i = 0; i < characteristic.getVoltage().size(); i++)
    {
        /*
        double I_calculated = 0; // Initial guess for current
        double tolerance = 1e-6;
        int max_iterations = 100;
        int iterations = 0;
        double f, df;
        const double V = characteristic.getVoltage()[i];

        while (iterations < max_iterations)
        {
            f = diode_equation(I_calculated, V, 0, I0, Rs, Rch, A, V_t);
            df = -I0 * exp((V + I_calculated * Rs) / (A * V_t)) * (Rs / (A * V_t)) - Rs / Rch - 1;

            double I_new = I_calculated - f / df;
            if (std::fabs(I_new - I_calculated) < tolerance)
            {
                characteristic.getCurrent()[i] = I_new;
            }
            I_calculated = I_new;
            iterations++;

            std::cerr << "Warning: Newton-Raphson method did not converge." << std::endl;
        }*/
        current(characteristic.getVoltage()[i], characteristic.getCurrent()[i], I0, A, Rch, Rs, T);
    }
}

void UI::Data::GeneratingCharacteristicsPanelData::CalculateCurrent(Characteristic &characteristic, double A, double I0, double Rs, double Rch, double Rch2, double alpha, double Temperature)
{
}

std::vector<double> UI::Data::generate_range(double start, double end, double step)
{
    std::vector<double> result;
    for (double value = start; (step > 0) ? (value < end) : (value > end); value += step)
    {
        result.push_back(value);
    }
    return result;
};

double UI::Data::diode_equation(double I, double V, double I_L, double I_0, double R_s, double R_sh, double n, double V_t)
{
    return I_L - I_0 * (std::exp((V + I * R_s) / (n * V_t)) - 1) - (V + I * R_s) / R_sh - I;
}

void UI::Data::current(double &V, double &I, double &I0, double &A, double &Rsch, double &Rs, double T)
{
    // Constants
    const double k = 8.6e-5;         // Boltzmann constant in J/K
    const double q = 1.60217662e-19; // Charge of an electron in C
    const double V_t = k * T / q;
    double x = ((q * I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
    // std::complex<double> c = {x, 0};
    // std::complex<double> c1 = LamberWN::LambertW(c, 0);

    double I_lw = x > -std::exp(-1) ? utl::LambertW<0>(x) : utl::LambertW<-1>(x);
    //  long double c1 = std::log(x);
    // double I_lw = ((A * k * T) / (Rs)) * c.real();
    // std::cout << "(q * I0 * Rs): " << (q * I0 * Rs) << std::endl;
    // std::cout << " (A * k * T):" << (A * k * T) << std::endl;
    // std::cout << "(q * I0 * Rs) / (A * k * T): " << ((q * I0 * Rs) / (A * k * T)) << std::endl;
    // std::cout << "V: " << V << std::endl;
    // std::cout << "V / (A * k * T): " << V / (A * k * T) << std::endl;
    // std::cout << "std::exp(V / (A * k * T)) " << std::exp(V / (A * k * T)) << std::endl;
    // std::cout << "x " << x << std::endl;
    I = I_lw + (V - I_lw * Rs) / Rsch;
    // I = c1.real();
}

namespace LamberWN
{

    using namespace std;

    // z * exp(z)
    complex<double> zexpz(complex<double> z)
    {
        return z * exp(z);
    }

    // The derivative of z * exp(z) = exp(z) + z * exp(z)
    complex<double> zexpz_d(complex<double> z)
    {
        return exp(z) + z * exp(z);
    }

    // The second derivative of z * exp(z) = 2. * exp(z) + z * exp(z)
    complex<double> zexpz_dd(complex<double> z)
    {
        return 2. * exp(z) + z * exp(z);
    }

    // Determine the initial point for the root finding
    complex<double> InitPoint(complex<double> z, int k)
    {
        const double pi{3.14159265358979323846};
        const double e{2.71828182845904523536};
        complex<double> I{0, 1};
        complex<double> two_pi_k_I{0., 2. * pi * k};
        complex<double> ip{log(z) + two_pi_k_I - log(log(z) + two_pi_k_I)}; // initial point coming from the general asymptotic approximation
        complex<double> p{sqrt(2. * (e * z + 1.))};                         // used when we are close to the branch cut around zero and when k=0,-1

        if (abs(z - (-exp(-1.))) <= 1.) // we are close to the branch cut, the initial point must be chosen carefully
        {
            if (k == 0)
                ip = -1. + p - 1. / 3. * pow(p, 2) + 11. / 72. * pow(p, 3);
            if (k == 1 && z.imag() < 0.)
                ip = -1. - p - 1. / 3. * pow(p, 2) - 11. / 72. * pow(p, 3);
            if (k == -1 && z.imag() > 0.)
                ip = -1. - p - 1. / 3. * pow(p, 2) - 11. / 72. * pow(p, 3);
        }

        if (k == 0 && abs(z - .5) <= .5)
            ip = (0.35173371 * (0.1237166 + 7.061302897 * z)) / (2. + 0.827184 * (1. + 2. * z)); // (1,1) Pade approximant for W(0,a)

        if (k == -1 && abs(z - .5) <= .5)
            ip = -(((2.2591588985 +
                     4.22096 * I) *
                    ((-14.073271 - 33.767687754 * I) * z - (12.7127 -
                                                            19.071643 * I) *
                                                               (1. + 2. * z))) /
                   (2. - (17.23103 - 10.629721 * I) * (1. + 2. * z))); // (1,1) Pade approximant for W(-1,a)

        return ip;
    }

    complex<double> LambertW(complex<double> z, int k)
    {
        // For some particular z and k W(z,k) has simple value:
        if (z == 0.)
            return (k == 0) ? 0. : -INFINITY;
        if (z == -exp(-1.) && (k == 0 || k == -1))
            return -1.;
        if (z == exp(1.) && k == 0)
            return 1.;

        // Halley method begins
        complex<double> w{InitPoint(z, k)}, wprev{InitPoint(z, k)}; // intermediate values in the Halley method
        const unsigned int maxiter = 30;                            // max number of iterations. This eliminates improbable infinite loops
        unsigned int iter = 0;                                      // iteration counter
        double prec = 1.E-30;                                       // difference threshold between the last two iteration results (or the iter number of iterations is taken)

        do
        {
            wprev = w;
            w -= 2. * ((zexpz(w) - z) * zexpz_d(w)) /
                 (2. * pow(zexpz_d(w), 2) - (zexpz(w) - z) * zexpz_dd(w));
            iter++;
        } while ((abs(w - wprev) > prec) && iter < maxiter);
        return w;
    }
}
