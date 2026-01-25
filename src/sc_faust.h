#pragma once

#include <SC_PlugIn.hpp>

#include <faust/dsp/llvm-dsp.h>

#include "sc_ui.h"

class ScFaust;

/*! @class ScFaust
 *  @brief Unit generator for evaluating faust dsp
 */
class ScFaust : public SCUnit {
public:
    ScFaust();
    ~ScFaust();

private:
    void next(int numSamples);

    llvm_dsp* mDsp = nullptr;
    SCRTUI* mScRtUi = nullptr;
    int mNumInputs;
    int mNumParams;
};
