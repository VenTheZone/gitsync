#!/bin/bash
# GitSync v2.0 - Quick Install Script
# Copy and paste this to install GitSync

# Download the binary
echo "Downloading GitSync v2.0..."
curl -L https://github.com/VenTheZone/gitsync/releases/download/v2.0.0/gitsync -o gitsync

# Make it executable
chmod +x gitsync

# Move to PATH (optional - requires sudo)
echo "Moving to /usr/local/bin..."
sudo mv gitsync /usr/local/bin/

# Verify installation
echo ""
echo "✅ Installation complete!"
gitsync --version
echo ""
echo "Usage:"
echo "  gitsync /path/to/your/Obsidian-vault/"
echo "  gitsync --help"
