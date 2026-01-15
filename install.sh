#!/bin/bash
# GitSync v2.0 - Cross-Platform Install Script
# Supports: Linux x86_64, Linux ARM64, macOS Intel, macOS ARM

set -e

echo "🔧 GitSync v2.0 - Cross-Platform Installer"
echo "=========================================="
echo ""

# Detect platform and architecture
detect_platform() {
    OS=$(uname -s | tr '[:upper:]' '[:lower:]')
    ARCH=$(uname -m)
    
    case "$ARCH" in
        x86_64)
            CPU="x86_64"
            ;;
        aarch64|arm64)
            CPU="arm64"
            ;;
        *)
            CPU="$ARCH"
            ;;
    esac
    
    echo "${OS}-${CPU}"
}

PLATFORM=$(detect_platform)
echo "📍 Detected platform: $PLATFORM"

# Try to download pre-built binary
download_binary() {
    local url="https://github.com/VenTheZone/gitsync/releases/download/v2.0.0/gitsync-${PLATFORM}"
    
    echo "📥 Trying to download pre-built binary..."
    if curl -L --fail -o gitsync "$url" 2>/dev/null; then
        chmod +x gitsync
        echo "✅ Downloaded pre-built binary for $PLATFORM"
        return 0
    fi
    return 1
}

# Compile from source
compile_from_source() {
    echo "📦 Compiling from source..."
    
    # Check for required tools
    if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
        echo "❌ Error: No C compiler found (gcc or clang)"
        echo ""
        echo "Install on Ubuntu/Debian:"
        echo "  sudo apt install build-essential"
        echo ""
        echo "Install on macOS:"
        echo "  xcode-select --install"
        return 1
    fi
    
    # Check for git
    if ! command -v git &> /dev/null; then
        echo "❌ Error: Git not found"
        echo "Please install Git first: https://git-scm.com/downloads"
        return 1
    fi
    
    # Clone and compile
    echo "🔨 Cloning source code..."
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    git clone --depth 1 https://github.com/VenTheZone/gitsync.git
    cd gitsync
    
    echo "⚙️  Compiling..."
    make
    
    # Copy binary
    cp gitsync "$OLDPWD/"
    cd "$OLDPWD"
    rm -rf "$TEMP_DIR"
    
    echo "✅ Compiled successfully"
}

# Main installation
install_gitsync() {
    # Try pre-built first
    if download_binary; then
        echo ""
    else
        echo "❌ No pre-built binary for $PLATFORM"
        echo ""
        compile_from_source
    fi
    
    # Move to PATH
    echo ""
    echo "📁 Moving to /usr/local/bin/..."
    sudo mv gitsync /usr/local/bin/ 2>/dev/null || mv gitsync ~/.local/bin/ 2>/dev/null || true
    
    # Verify
    echo ""
    echo "✅ Installation complete!"
    echo ""
    
    if command -v gitsync &> /dev/null; then
        gitsync --version
    else
        echo "⚠️  Add ~/.local/bin to your PATH or run:"
        echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
    fi
    
    echo ""
    echo "📖 Usage:"
    echo "  gitsync /path/to/Obsidian-vault/"
    echo "  gitsync --help"
}

# Run installation
install_gitsync
