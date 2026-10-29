/*
 * Helper class for displaying machine parameters with discrete choices support.
 * 
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

namespace mt
{
    struct InstrumentParam;
}

namespace kick_ui
{

/**
 * Helper functions for working with machine parameters that have discrete choices.
 */
class MachineParamHelper
{
public:
    /**
     * Get the display text for a parameter value.
     * If the parameter has choices, returns the choice name.
     * Otherwise, returns the numeric value formatted appropriately.
     */
    static juce::String getDisplayTextForParam(
        const mt::InstrumentParam& param,
        float value,
        int numDecimalPlaces = 2);

    /**
     * Get the choice index for a parameter value.
     * Returns -1 if no choices or value out of range.
     */
    static int getChoiceIndex(const mt::InstrumentParam& param, float value);

    /**
     * Get the choice name for a parameter value.
     * Returns empty string if no choices or value out of range.
     */
    static juce::String getChoiceName(const mt::InstrumentParam& param, float value);
};

} // namespace kick_ui
