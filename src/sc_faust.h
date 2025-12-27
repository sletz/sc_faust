#pragma once

#include <SC_PlugIn.hpp>

#include <faust/dsp/llvm-dsp.h>

#include "sc_ui.h"

class FaustGen;

/*! @class FaustGen
 *  @brief Unit generator for evaluating faust dsp
 */
class FaustGen : public SCUnit {
public:
    FaustGen();
    ~FaustGen();

private:
    void next(int numSamples);

    llvm_dsp* mDsp = nullptr;
    SCRTUI* mScRtUi = nullptr;
    int mNumInputs;
    int mNumParams;
};
