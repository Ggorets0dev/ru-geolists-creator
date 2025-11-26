#!/usr/bin/env bash
set -e

# ==============================
# CHANGING PWD
# ==============================

# Absolute path to directory with script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CURRENT_DIR="$(pwd)"

if [ "$CURRENT_DIR" = "$SCRIPT_DIR" ]; then
  # Entering project's root
  cd ..
fi
# ==============================

APPNAME="$1"
BINARY_PATH="$2"
APPDIR="${APPNAME}.AppDir"

if [[ -z "$APPNAME" || -z "$BINARY_PATH" ]]; then
    echo "Использование: $0 <AppName> <путь_к_бинарнику>"
    exit 1
fi

# --- Создаем структуру AppDir ---
echo "[1] Создаю структуру ${APPDIR}"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/lib"
mkdir -p "${APPDIR}/usr/share/applications"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/256x256/apps"

# --- Копируем бинарник ---
echo "[2] Копирую бинарник"
cp "$BINARY_PATH" "${APPDIR}/usr/bin/${APPNAME}"

# --- Копируем динамические библиотеки ---
echo "[3] Сканирую зависимости и копирую .so"

# Библиотеки, которые НЕЛЬЗЯ копировать
SYS_LIBS="ld-linux|libc\.so|libm\.so|libpthread|librt|libdl|libgcc_s\.so"

ldd "$BINARY_PATH" | awk '/=>/ {print $3}' | while read -r lib; do
    if [[ -f "$lib" ]]; then
        if [[ "$lib" =~ $SYS_LIBS ]]; then
            echo "   Пропускаю системную библиотеку: $lib"
        else
            echo "   Копирую: $lib"
            cp -v "$lib" "${APPDIR}/usr/lib/" || true
        fi
    fi
done

# --- Обязательно копируем libstdc++ ---
if [[ -f /usr/lib/libstdc++.so.6 ]]; then
    echo "[4] Копирую libstdc++.so.6"
    cp -v /usr/lib/libstdc++.so.6 "${APPDIR}/usr/lib/"
elif [[ -f /usr/lib64/libstdc++.so.6 ]]; then
    cp -v /usr/lib64/libstdc++.so.6 "${APPDIR}/usr/lib/"
else
    echo "⚠️ ВНИМАНИЕ: libstdc++.so.6 не найдена!"
fi

# --- AppRun ---
echo "[5] Создаю AppRun"

cat > "${APPDIR}/AppRun" <<EOF
#!/bin/bash
HERE="\$(dirname "\$(readlink -f "\$0")")"
export LD_LIBRARY_PATH="\$HERE/usr/lib:\$LD_LIBRARY_PATH"
exec "\$HERE/usr/bin/${APPNAME}" "\$@"
EOF

chmod +x "${APPDIR}/AppRun"

# --- Desktop entry ---
echo "[6] Создаю desktop файл"

cat > "${APPDIR}/${APPNAME}.desktop" <<EOF
[Desktop Entry]
Name=${APPNAME}
Exec=${APPNAME}
Icon=${APPNAME}
Type=Application
Categories=Utility;
EOF

cp "${BINARY_PATH%/*}/${APPNAME}.png" "${APPDIR}/usr/share/icons/hicolor/256x256/apps/${APPNAME}.png" 2>/dev/null || true

echo "====================================="
echo " AppDir готов: ${APPDIR}/"
echo " Для сборки AppImage:"
echo "   ./appimagetool-x86_64.AppImage ${APPDIR}"
echo "====================================="
