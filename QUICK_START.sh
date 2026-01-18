#!/bin/bash

################################################################################
# Choir V2.0 - Quick Start Build Script
# Simplified build for common scenarios
################################################################################

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${BLUE}=================================="
echo "Choir V2.0 - Quick Start"
echo "==================================${NC}"
echo ""

# Check if CMakeLists.txt exists
if [ ! -f "${SCRIPT_DIR}/CMakeLists.txt" ]; then
    echo -e "${YELLOW}Warning: CMakeLists.txt not found${NC}"
    echo "This script expects to be run from the choral_v2 directory"
    echo ""
    echo "Usage:"
    echo "  cd juce_backend/instruments/choral_v2"
    echo "  ./quick_start.sh"
    echo ""
    exit 1
fi

# Show menu
echo "Choose build option:"
echo ""
echo "1) Build all 6 formats (VST3, AU, CLAP, LV2, AUv3, Standalone)"
echo "2) Build VST3 only"
echo "3) Build AU only"
echo "4) Build CLAP only"
echo "5) Build Standalone only"
echo "6) Verify existing builds"
echo "7) Exit"
echo ""
read -p "Enter choice [1-7]: " choice

case $choice in
    1)
        echo -e "${GREEN}Building all formats...${NC}"
        "${SCRIPT_DIR}/build_all_formats.sh"
        ;;
    2)
        echo -e "${GREEN}Building VST3 only...${NC}"
        cd "${SCRIPT_DIR}/build"
        cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_BUILD_CORE=ON -DCHOIR_V2_PLUGIN_FORMATS="VST3" -DCMAKE_BUILD_TYPE=Release
        cmake --build . -j8
        echo -e "${GREEN}VST3 plugin built!${NC}"
        echo "Location: build/plugins/vst/Choir V2.0.vst3"
        ;;
    3)
        echo -e "${GREEN}Building AU only...${NC}"
        cd "${SCRIPT_DIR}/build"
        cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_BUILD_CORE=ON -DCHOIR_V2_PLUGIN_FORMATS="AU" -DCMAKE_BUILD_TYPE=Release
        cmake --build . -j8
        echo -e "${GREEN}AU plugin built!${NC}"
        echo "Location: build/plugins/au/Choir V2.0.component"
        ;;
    4)
        echo -e "${GREEN}Building CLAP only...${NC}"
        cd "${SCRIPT_DIR}/build"
        cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_BUILD_CORE=ON -DCHOIR_V2_PLUGIN_FORMATS="CLAP" -DCMAKE_BUILD_TYPE=Release
        cmake --build . -j8
        echo -e "${GREEN}CLAP plugin built!${NC}"
        echo "Location: build/plugins/clap/Choir V2.0.clap"
        ;;
    5)
        echo -e "${GREEN}Building Standalone only...${NC}"
        cd "${SCRIPT_DIR}/build"
        cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_BUILD_CORE=ON -DCHOIR_V2_PLUGIN_FORMATS="Standalone" -DCMAKE_BUILD_TYPE=Release
        cmake --build . -j8
        echo -e "${GREEN}Standalone app built!${NC}"
        echo "Location: build/plugins/standalone/Choir V2.0.app"
        ;;
    6)
        echo -e "${GREEN}Verifying builds...${NC}"
        "${SCRIPT_DIR}/verify_plugins.sh"
        ;;
    7)
        echo "Exiting..."
        exit 0
        ;;
    *)
        echo -e "${YELLOW}Invalid choice. Exiting.${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}Done!${NC}"
