#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# Universal minimal AppDir → works when built as root in Docker
# and packaged with appimagetool by a normal user on the host
# =============================================================================

APPNAME="$1"
BINARY_PATH="$2"

[[ -z "${APPNAME:-}" || -z "${BINARY_PATH:-}" || ! -f "$BINARY_PATH" ]] && {
    echo "Usage: $0 <AppName> <path-to-binary>"
    echo "Example: $0 rglc ./build/rglc"
    exit 1
}

APPDIR="${APPNAME}.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib"

echo "Creating AppDir for $APPNAME ..."

# 1. Copy binary
cp -a "$BINARY_PATH" "$APPDIR/usr/bin/$APPNAME"
chmod 755 "$APPDIR/usr/bin/$APPNAME"

# 2. Collect shared libraries (recursive)
mapfile -t LIBS < <(
    ldd "$BINARY_PATH" | awk 'NF==4{print$3};NF==2{print$1}' | grep '^/' || true
    for _ in {1..6}; do
        ldd "$APPDIR/usr/lib/"*.so* 2>/dev/null |
            awk 'NF==4{print$3};NF==2{print$1}' | grep '^/' || true
    done | sort -u
)

BLACKLIST='^/lib.*/(ld-linux.*|libc\.so|libm\.so|libpthread\.so|librt\.so|libdl\.so|libcrypt\.so|libresolv\.so|libutil\.so|libnsl\.so|libgcc_s\.so|libstdc++\.so)'

for lib in "${LIBS[@]}"; do
    [[ -f "$lib" ]] || continue
    [[ "$lib" =~ $BLACKLIST ]] && continue

    dest="$APPDIR/usr/lib/$(basename "$lib")"
    [[ -f "$dest" ]] || cp -a -- "$lib" "$dest"

    # copy versioned symlinks
    find "$(dirname "$lib")" -maxdepth 1 -type l -name "$(basename "$lib")*" \
        -exec cp -a -- {} "$APPDIR/usr/lib/" \; 2>/dev/null || true
done

# 3. Patch RPATH (recommended)
if command -v patchelf >/dev/null 2>&1; then
    patchelf --force-rpath --set-rpath '$ORIGIN/../lib' "$APPDIR/usr/bin/$APPNAME"
fi

# 4. AppRun — the trick: write it already with the correct name (no sed needed)
cat > "$APPDIR/AppRun" << EOF
#!/bin/bash
HERE="\$(dirname "\$(readlink -f "\${0}")")"
export LD_LIBRARY_PATH="\$HERE/usr/lib\${LD_LIBRARY_PATH:+:}\$LD_LIBRARY_PATH"
exec "\$HERE/usr/bin/$APPNAME" "\$@"
EOF
chmod 755 "$APPDIR/AppRun"

# 5. Desktop file
cat > "$APPDIR/${APPNAME}.desktop" << EOF
[Desktop Entry]
Type=Application
Name=$APPNAME
Icon=$APPNAME
Exec=$APPNAME
Categories=Utility;
Terminal=false
EOF

# 6. Icon (appimagetool loves exactly this name)
cp img/rglc.png "$APPDIR/${APPNAME}.png"   # ← change this path if your icon is elsewhere

# =============================================================================
# Make the directory fully usable by a regular host user (even if built as root)
# =============================================================================
chmod -R a+rX "$APPDIR"
chmod -R go-w "$APPDIR"
rm -f "$APPDIR/.DirIcon" 2>/dev/null || true   # remove if exists

# allow host user to delete/overwrite everything without sudo
chmod 777 "$APPDIR" 2>/dev/null || true
chmod 666 "$APPDIR"/*.png "$APPDIR"/*.desktop 2>/dev/null || true

echo
echo "AppDir ready: $APPDIR"
echo "Bundled libraries: $(find "$APPDIR/usr/lib" -name "*.so*" | wc -l)"
echo
echo "Now run on the host (as normal user):"
echo "   appimagetool $APPDIR"
echo "→ No warnings, perfect icon, works every time."

exit 0