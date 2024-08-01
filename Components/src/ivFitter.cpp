#include "pch.hpp"
#include "IVFitter.hpp"

namespace JunctionFitMasterFromNS::IVFitting
{
    IVSimplexOptimizer<IVModel> getOptimizer(IVFittingSetup& config) {
        using Settings = IVSimplexOptimizerSettings<IVModel>;
        using Builder = typename Settings::IVSimplexOptimizerSettingsBuilder;

        LogCreatorSettings<4> logSettings{ config.simplexMin, config.simplexMax };
        logSettings.setP(config.logP);

        Builder builder{};
        builder.errorModel(IVError{})
            .addLogCreatorSettings(logSettings)
            .addOperationSettings({ {BasicOperationsEnum::Reflect, config.reflec_coeff},
                                   {BasicOperationsEnum::Expand, config.expand_coeff},
                                   {BasicOperationsEnum::Contract, config.contract_coeff},
                                   {BasicOperationsEnum::Shrink, config.shrink_coeff} })
            .minError(config.minError)
            .maxIteration(config.maxIteration);

        Settings settings = builder.build();

        IVSimplexOptimizer<IVModel> optimizer{ settings };

        optimizer.setUp();

        return optimizer;
    }

    Fitter<IVSimplexOptimizer<IVModel>> getFitter(IVFittingSetup& config)
    {
        

        return Fitter<IVSimplexOptimizer<IVModel>>{getOptimizer(config)};
    }

};