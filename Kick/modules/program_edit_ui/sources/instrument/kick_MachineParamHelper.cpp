/*
 * Helper class for displaying machine parameters with discrete choices support.
 * 
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "kick_MachineParamHelper.h"
#include "../../../engine/audio_program/instrument/mt_instrument_engine.h"

namespace kick_ui
{

juce::String MachineParamHelper::getDisplayTextForParam(
    const mt::InstrumentParam& param,
    float value,
    int numDecimalPlaces)
{
    // If parameter has choices, try to show the choice name
    if (!param.choices.isEmpty() && param.maxValue > param.minValue)
    {
        int choiceIndex = getChoiceIndex(param, value);
        if (choiceIndex >= 0 && choiceIndex < param.choices.size())
            return param.choices[choiceIndex];
    }
    
    // Otherwise, format as numeric value
    if (numDecimalPlaces >= 0)
    {
        return juce::String(value, numDecimalPlaces);
    }
    return juce::String(value);
}

int MachineParamHelper::getChoiceIndex(const mt::InstrumentParam& param, float value)
{
    if (param.choices.isEmpty())
        return -1;
    
    // Map value to choice index
    // Value is in [minValue, maxValue] range
    // Choice index is in [0, choices.size()-1]
    
    if (param.maxValue == param.minValue)
        return 0;  // Avoid division by zero
    
    float normalized = (value - param.minValue) / (param.maxValue - param.minValue);
    int index = static_cast<int>(std::round(normalized * (param.choices.size() - 1)));
    
    return juce::jlimit(0, param.choices.size() - 1, index);
}

juce::String MachineParamHelper::getChoiceName(const mt::InstrumentParam& param, float value)
{
    int index = getChoiceIndex(param, value);
    if (index >= 0 && index < param.choices.size())
        return param.choices[index];
    return juce::String();
}

} // namespace kick_ui
