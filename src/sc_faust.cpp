#include "sc_faust.h"

#include "library.h"

#include <fstream>
#include <iostream>

#ifndef FAUST_LIBRARY_PATH
error "FAUST_LIBRARY_PATH must be defined by CMake"
#endif

    InterfaceTable* ft;

extern Library::CodeLibrary* gLibrary;

FaustGen::FaustGen() {
    auto hash = static_cast<int>(in0(0));
    mNumInputs = static_cast<int>(in0(1));
    mNumParams = static_cast<int>(in0(2));

    set_calc_function<FaustGen, &FaustGen::next>();

    auto node = Library::findEntry(hash);
    if (node == nullptr) {
        Print("ERROR: Could not find faust factory with hash %d\n", hash);
        return;
    }

    mScRtUi = new SCRTUI(mWorld, node->numParams);
    mDsp = node->factory->createDSPInstance();
    mDsp->init(static_cast<int>(mWorld->mSampleRate));
    mDsp->buildUserInterface(mScRtUi);
}

FaustGen::~FaustGen() {
    delete mScRtUi;
    delete mDsp;
}


void FaustGen::next(int numSamples) {
    if (mDsp != nullptr) {
        for (int i = 0; i < mNumParams; i++) {
            auto paramOffset = **(mInBuf + 3 + mNumInputs + (i * 2));
            auto param = mScRtUi->getParam(paramOffset);
            *param = **(mInBuf + 3 + mNumInputs + (i * 2) + 1);
        };
        mDsp->compute(numSamples, mInBuf + 3, mOutBuf);
    }
}

PluginLoad("FaustGen") {
    ft = inTable;

    registerUnit<FaustGen>(inTable, "FaustGen", false);

    ft->fDefinePlugInCmd("faustgenscript", Library::faustGenCompileScript, nullptr);
}
