#pragma once

#define FAUSTFLOAT float

#include <faust/gui/UI.h>
#include <SC_PlugIn.hpp>

using ParamPair = std::pair<std::string, float*>;
extern InterfaceTable* ft;

/*! @class SCUI
 *  @brief A wrapper between Faust UI and SuperCollider inputs
 */
class SCUI : public UI {
protected:
    std::vector<ParamPair> params;


public:
    size_t getNumParams() { return params.size(); }
    float* getParam(const int num) { return params[num].second; }
    std::vector<ParamPair> getParams() { return params; }

    // UI callbacks
    void openTabBox(const char* label) override {}
    void openHorizontalBox(const char* label) override {};
    void openVerticalBox(const char* label) override {};
    void closeBox() override {};
    void addButton(const char* label, float* zone) override { params.emplace_back(label, zone); };
    void addCheckButton(const char* label, float* zone) override {};
    void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) override {
        params.emplace_back(label, zone);
    };
    void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) override {
        params.emplace_back(label, zone);
    };
    void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) override {};
    void addHorizontalBargraph(const char* label, float* zone, float min, float max) override {};
    void addVerticalBargraph(const char* label, float* zone, float min, float max) override {};
    void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) override {};
};

/*! @class SCRTUI
 *  @brief Same as SCUI but does not allocate by redacting the name of the parameter
 *  and instead relying on the deterministic order obtained in the NRT version
 */
class SCRTUI : public UI {
protected:
    float** params;
    int counter = 0;

    void add(float* zone) {
        params[counter] = zone;
        counter++;
    }

public:
    SCRTUI(World* world, int numParams): params(nullptr), counter(0) {
        params = static_cast<float**>(RTAlloc(world, sizeof(float*) * numParams));
    };

    float* getParam(const int num) { return params[num]; }
    // UI callbacks
    void openTabBox(const char* label) override {}
    void openHorizontalBox(const char* label) override {};
    void openVerticalBox(const char* label) override {};
    void closeBox() override {};
    void addButton(const char* label, float* zone) override { add(zone); };
    void addCheckButton(const char* label, float* zone) override {};
    void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step) override {
        add(zone);
    };
    void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step) override {
        add(zone);
    };
    void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) override {};
    void addHorizontalBargraph(const char* label, float* zone, float min, float max) override {};
    void addVerticalBargraph(const char* label, float* zone, float min, float max) override {};
    void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) override {};
};