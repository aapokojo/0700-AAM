#!/bin/bash
# Configure and build the KICK app (Kick target), the simulator (KT_KickEM), and VST3 plugins on macOS.
# Needs CMake and Ninja on PATH (or set CMAKE / NINJA), and network access the first time
# to fetch JUCE 7.0.12 and CPM.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CMAKE="${CMAKE:-cmake}"
NINJA="${NINJA:-ninja}"
BUILD="$ROOT/Kick/build-mac"
JUCE_DIR="$ROOT/.3rdPartySourceCache/JUCE-7.0.12"
[ -d "$JUCE_DIR" ] || git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git "$JUCE_DIR"

# JUCE 7.0.12's juceaide calls an API removed in macOS 15; target 12.0 as Kick itself does.
export MACOSX_DEPLOYMENT_TARGET=12.0

# Build everything in one go - apps and VST plugins
"$CMAKE" -S "$ROOT/Kick" -B "$BUILD" -G Ninja -DCMAKE_MAKE_PROGRAM="$(command -v "$NINJA" || echo "$NINJA")" \
    -DCMAKE_BUILD_TYPE=Release -DTIMESTAMP=OFF \
    -DKICK_SIMULATOR=ON -DKICK_VST_PLUGIN=OFF -DKICK_VST_FSM=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCPM_JUCE_SOURCE="$JUCE_DIR" -DCPM_AudIO_SOURCE="$ROOT/AudIO" -DCPM_COM_PROTOCOL_SOURCE="$ROOT/ComProtocol"
"$NINJA" -C "$BUILD" Kick KT_KickEM kickvst_fsm kickvst_fsm_VST3

# Create symlinks for VST3 plugins to expected locations
if [ -d "$BUILD/apps/kickvst_fsm/kickvst_fsm_artefacts/Release/VST3/kickvst_fsm.vst3" ]; then
    ln -sf "$BUILD/apps/kickvst_fsm/kickvst_fsm_artefacts/Release/VST3/kickvst_fsm.vst3" "$BUILD/kickvst_fsm.vst3"
fi

# On macOS the app finds ./assets, ./demos and ./templates relative to the working directory,
# and the build copies them into the build folder, so start the apps from there.
echo ""
echo "Built applications:"
echo "  $BUILD/Kick.app"
echo "  $BUILD/KT_KickEM.app"
echo ""
echo "Built VST3 plugins:"
echo "  $BUILD/kickvst.vst3"
echo "  $BUILD/kickvst_fsm.vst3"
echo ""
echo "Run the simulator with:"
echo "  cd \"$BUILD\" && KT_KickEM.app/Contents/MacOS/KT_KickEM"
