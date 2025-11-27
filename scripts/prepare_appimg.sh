#!/usr/bin/env bash
set -euo pipefail

APPNAME="$1"
BINARY_PATH="$2"

if [[ -z "$APPNAME" || -z "$BINARY_PATH" || ! -f "$BINARY_PATH" ]]; then
    echo "Usage: $0 <AppName> <path-to-binary>"
    echo "Example: $0 MyApp ./myapp"
    exit 1
fi

APPDIR="${APPNAME}.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib"

echo "Creating minimal AppImage for $APPNAME"

# 1. Copy the main binary
cp -a "$BINARY_PATH" "$APPDIR/usr/bin/$APPNAME"
chmod +x "$APPDIR/usr/bin/$APPNAME"

# 2. Recursively collect all required shared libraries
echo "Collecting required shared libraries..."
mapfile -t all_libs < <(
    # Direct dependencies of the binary
    ldd "$BINARY_PATH" | awk 'NF==4 {print $3}; NF==2 {print $1}' | grep '^/' || true

    # Recursively add dependencies of already copied libraries (up to 5 levels deep)
    for i in {1..5}; do
        ldd "$APPDIR/usr/lib/"*.so* 2>/dev/null | \
            awk 'NF==4 {print $3}; NF==2 {print $1}' | grep '^/' || true
    done | sort -u
)

# Blacklist: core system libraries that should never be bundled
BLACKLIST='^/lib.*/ld-linux.*|^/lib.*/libc\.so|^/lib.*/libm\.so|^/lib.*/libpthread\.so|^/lib.*/librt\.so|^/lib.*/libdl\.so|^/lib.*/libcrypt\.so|^/lib.*/libresolv\.so|^/lib.*/libutil\.so|^/lib.*/libnsl\.so'

for lib in "${all_libs[@]}"; do
    [[ -f "$lib" ]] || continue
    if [[ "$lib" =~ $BLACKLIST ]]; then
        echo "   Skipping system library: $(basename "$lib")"
        continue
    fi

    dest="$APPDIR/usr/lib/$(basename "$lib")"
    if [[ ! -f "$dest" ]]; then
        cp -a -- "$lib" "$dest"
        echo "   + $(basename "$lib")"
    fi

    # Copy symlinks that point to this library (from the same directory)
    srcdir=$(dirname "$lib")
    find "$srcdir" -maxdepth 1 -type l -name "$(basename "$lib")*" -exec cp -a -- {} "$APPDIR/usr/lib/" \; 2>/dev/null || true
done

# 3. Patch RPATH using patchelf (if available)
if command -v patchelf >/dev/null 2>&1; then
    patchelf --force-rpath --set-rpath '$ORIGIN/../lib' "$APPDIR/usr/bin/$APPNAME"
    echo "RPATH set to \$ORIGIN/../lib"
fi

# 4. Create AppRun
cat > "$APPDIR/AppRun" <<EOF
#!/bin/bash
export LD_LIBRARY_PATH="\$(dirname "\$(readlink -f "\$0")")/usr/lib\${LD_LIBRARY_PATH:+:}\$LD_LIBRARY_PATH"
exec "\$(dirname "\$(readlink -f "\$0")")/usr/bin/$APPNAME" "\$@"
EOF
chmod +x "$APPDIR/AppRun"

# 5. Create minimal .desktop file
cat > "$APPDIR/${APPNAME}.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APPNAME
Icon=$APPNAME
Exec=$APPNAME
Categories=Utility;
Terminal=false
EOF

echo
echo "Done! Minimal AppDir created: $APPDIR"
echo "Bundled shared libraries: $(find "$APPDIR/usr/lib" -name "*.so*" | wc -l)"
echo
echo "To build the final AppImage, run:"
echo "   appimagetool \"$APPDIR\""
echo "   (download appimagetool from https://github.com/AppImage/appimagetool/releases if needed)"