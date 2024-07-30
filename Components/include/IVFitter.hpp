#pragma once

#include "SimplexCreatorSettings.hpp"
#include "VisitorOperationBase.hpp"
#include "BasicSimplexOptimizerSettings.hpp"
#include "BasicSimplexOptimizer.hpp"
#include "AdditionalParameters.hpp"
#include "Random.hpp"
#include "Model.hpp"
#include "Optimizer.hpp"
#include "ModelBase.hpp"
#include "LogDist.hpp"
#include "data.hpp"
#include "Fitter.hpp"
#include "ErrorModel.hpp"


namespace JunctionFitMaster::IVFitting {
	using namespace NumericStorm::Utils;
	using namespace JunctionFitMaster::Utils;
	using namespace NumericStorm::Fitting;
	using namespace NumericStorm::Concepts;

	template<size_t parameter_size>
	class LogCreatorSettings : public SimplexCreatorSettings<parameter_size> {
	public:



		LogCreatorSettings() = default;

		LogCreatorSettings(const Parameters<parameter_size>& min, const Parameters<parameter_size>& max)
			: SimplexCreatorSettings<parameter_size>(min, max) {};



		auto& getP() {
			return m_p;
		}

		const auto& getP() const {
			return m_p;
		}

		void setP(double p) {
			m_p = p;
		}

	private:
		double m_p{ 0.5 };
	};




	template<size_t parameter_size>
	class LogSimplexCreator : public VisitorOperationBase<LogCreatorSettings<parameter_size>> {
	public:
		using SettingsT = typename VisitorOperationBase<LogCreatorSettings<parameter_size>>::SettingsT;


		LogSimplexCreator() = default;

		LogSimplexCreator(const SettingsT& settings)
			: VisitorOperationBase<SettingsT>{ settings } {}

		virtual ~LogSimplexCreator() = default;

		typename SettingsT::Out operator()(const typename SettingsT::In& input) {
			std::array<typename SettingsT::In, parameter_size + 1> points{};
			points.fill(input);

			std::for_each(points.begin() + 1, points.end(), [&](auto& point) {
				size_t index{ 0 };
				std::for_each(point.begin(), point.end(), [&](auto& value) {
					if (index == 0)
						value += LogDist::value(this->m_settings.getP(), this->m_settings.getMinBounds()[index], this->m_settings.getMaxBounds()[index]);
					else
						value += Random::Float(this->m_settings.getMinBounds()[index], this->m_settings.getMaxBounds()[index]);
					index++;
					});

				point.evaluatePoint();
				});

			typename SettingsT::Out figure{ points };
			return figure;
		}
	};




	template<Model M>
	class IVSimplexOptimizerSettings : public BasicSimplexOptimizerSettings<M> {
	public:


		IVSimplexOptimizerSettings() = default;

		auto& getLogCreatorSettings() {
			return m_logCreatorSettings;
		}


	protected:
		LogCreatorSettings<M::parameter_size> m_logCreatorSettings{};
	protected:
		template<class BuildingType, OptimizerSettings Settings>
		class IVSimplexOptimizerSettingsBuilderBase : public BasicSimplexOptimizerSettings<M>::BasicSimplexOptimizerSettingsBuilderBase<BuildingType, Settings> {
		public:

			BuildingType& addLogCreatorSettings(const LogCreatorSettings<M::parameter_size>& settings) {
				this->m_settingsObject.m_logCreatorSettings = settings;
				return this->returnSelf();
			}
		};

		friend class IVSimplexOptimizerSettingsBuilder;

	public:
		class IVSimplexOptimizerSettingsBuilder : public IVSimplexOptimizerSettingsBuilderBase<IVSimplexOptimizerSettingsBuilder, IVSimplexOptimizerSettings<M>> {};
	};




	template<Model M>
	class IVSimplexOptimizer : public BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void> {
	public:
		using SettingsT = IVSimplexOptimizerSettings<M>;
		using AdapterT = void;

		IVSimplexOptimizer(const SettingsT& settings)
			: BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>{ settings } {}

		virtual ~IVSimplexOptimizer() = default;

		void setUp() {
			BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>::setUp();
			m_logSimplexCreator.updateSettings(this->m_settings.getLogCreatorSettings());
		};

		typename SettingsT::OptimizerStateT setUpOptimization(const typename SettingsT::OptimizerInputT& input, const Data& data, const typename SettingsT::AuxilaryParametersT& additionalParameters) {
			SimplexPoint<M::parameter_size> inputPoint{ input };
			inputPoint.getData() = data;
			inputPoint.onEvaluate([&](SimplexPoint<M::parameter_size>& point) {
				this->m_settings.getModel()(point.getData(), point.getParameters(), additionalParameters);
				point.setError(this->m_settings.getErrorModel()(point.getData(), data));
				});

			auto pointCount = SimplexStrategySettings<M::parameter_size>::indecies::Count;
			typename SettingsT::OptimizerStateT state{ m_logSimplexCreator(inputPoint), pointCount };

			return state;
		};

	protected:
		LogSimplexCreator<M::parameter_size> m_logSimplexCreator{};
	};

	struct IVAdditionalParameters {
		double T{ 0.0 };
	};

	class IVModel : public ModelBase <4, IVAdditionalParameters> {
	public:
		IVModel() : ModelBase<4, IVAdditionalParameters>{ [](Data& data, const Parameters<4>& parameters, const AuxParameters& aParameters) {
			auto [I0, A, Rs, Rch] = parameters.getParameters();
			for (size_t i = 0; i < data[0].size(); i++) {
				UI::Data::current(data[0][i], data[1][i], I0, A, Rs, Rch, aParameters.T);
			}} } {}
	};

	class IVError : public ErrorModel {
	public:
		IVError() : ErrorModel{ [](const Data& data, const Data& model) {
			double error{ 0.0 };
			for (size_t i = 0; i < data[0].size(); i++) {
				error += std::pow(data[1][i] - model[1][i], 2);
			}
			return error;
		} } {}
	};


	struct IVFittingSetup {
		Parameters<4> simplexMin{};
		Parameters<4> simplexMax{};

		//the parameter of the logarithmic distribution for the simplex creator
		double logP{ 0.5 };

		//simplex operations coeficients
		double reflec_coeff{ 1.2 };
		double expand_coeff{ 1.5 };
		double contract_coeff{ 0.6 };
		double shrink_coeff{ 0.5 };

		double minError{ 0.0000001 };
		long int maxIteration{ 3000 };

	};

	Fitter<IVSimplexOptimizer<IVModel>> getFitter(IVFittingSetup& config) {
		using Settings = IVSimplexOptimizerSettings<IVModel>;
		using Builder = typename Settings::IVSimplexOptimizerSettingsBuilder;




		LogCreatorSettings<4> logSettings{ config.simplexMin, config.simplexMax };
		logSettings.setP(config.logP);

		Builder builder{};
		builder.errorModel(IVError{})
			.addLogCreatorSettings(logSettings)
			.addOperationSettings({ { BasicOperationsEnum::Reflect, config.reflec_coeff },
									{ BasicOperationsEnum::Expand, config.expand_coeff },
									{ BasicOperationsEnum::Contract, config.contract_coeff},
									{ BasicOperationsEnum::Shrink, config.shrink_coeff }
				})
			.minError(config.minError)
			.maxIteration(config.maxIteration);


		Settings settings = builder.build();

		IVSimplexOptimizer<IVModel> optimizer{ settings };

		optimizer.setUp();

		return Fitter<IVSimplexOptimizer<IVModel>>{ optimizer };

	}

};