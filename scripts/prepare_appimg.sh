#!/usr/bin/env bash
set -euo pipefail

# ==============================
# НАЗВАНИЕ И ПУТИ
# ==============================

APPNAME="$1"
BINARY_PATH="$2"

if [[ -z "$APPNAME" || -z "$BINARY" || ! -f "$BINARY" ]]; then
    echo "Использование: $0 <AppName> <путь_к_бинарнику>"
    echo "Пример: $0 MyApp ./build/myapp"
    exit 1
fi

APPDIR="${APPNAME}.AppDir"

echo "Создаём AppImage для $APPNAME из $BINARY → $APPDIR"

# Удаляем старый AppDir, если есть
rm -rf "$APPDIR"
mkdir -p "$APPDIR"/{usr/bin,usr/lib,usr/share/applications,usr/share/icons/hicolor/256x256/apps}

# ==============================
# 1. Копируем сам бинарник
# ==============================
echo "[1] Копируем исполняемый файл"
cp -v "$BINARY" "$APPDIR/usr/bin/$APPNAME"
chmod +x "$APPDIR/usr/bin/$APPNAME"

# ==============================
# 2. Функция копирования библиотеки + всех её симлинков
# ==============================
copy_library() {
    local lib="$1"
    [[ -z "$lib" || ! -f "$lib" ]] && return

    local dest="$APPDIR/usr/lib/"

    # Копируем сам файл (обычно это .so.x.y.z)
    cp -v --parents "$lib" "$APPDIR/usr/" 2>/dev/null || cp -v "$lib" "$dest/"

    # Копируем все симлинки, которые указывают на него или являются его версиями
    local dir=$(dirname "$lib")
    local base=$(basename "$lib")
    (
        cd "$dir"
        find . -type l -lname "*$base*" -o -name "$base.*" | while read -r link; do
            cp -Pv --parents "$link" "$APPDIR/usr/" 2>/dev/null || true
        done
    )
}

# ==============================
# 3. Рекурсивно собираем ВСЕ зависимости (кроме базовых системных)
# ==============================
echo "[2] Сканируем и копируем зависимости"

# Эти библиотеки почти всегда лучше брать из системы пользователя, а не bundling
BLACKLIST='libc\.so|libm\.so|libpthread\.so|librt\.so|libdl\.so|ld-linux|libcrypt\.so|libresolv\.so|libnsl\.so|libutil\.so'

mapfile -t libs < <(ldd "$BINARY" | awk '/=>/ {print $3}' | grep '^/' || true)

for lib in "${libs[@]}"; do
    if [[ -f "$lib" ]]; then
        basename_lib=$(basename "$lib")
        if [[ "$basename_lib" =~ $BLACKLIST ]]; then
            echo "   Пропуск системной: $basename_lib"
            continue
        fi

        echo "   Копирую: $lib"
        copy_library "$lib"
    fi
done

# ==============================
# 4. Принудительно добавляем важные runtime-библиотеки
# ==============================
echo "[3] Добавляем критически важные библиотеки (libstdc++, libgcc_s, etc.)"

# libstdc++.so.6 — берём точно ту версию, с которой слинкован бинарник
if libstdcpp=$(ldd "$BINARY" | grep -o '/.*libstdc++\.so\.[0-9]*' | head -1); then
    copy_library "$libstdcpp"
else
    echo "⚠️  libstdc++.so.6 не найдена в ldd!"
fi

# libgcc_s.so.1 — часто нужна на старых системах
if libgcc=$(ldd "$BINARY" | grep -o '/.*libgcc_s\.so\.[0-9]*' | head -1); then
    copy_library "$libgcc"
fi

# ==============================
# 5. Чисти RPATH и ставим $ORIGIN, если нужно
# ==============================
echo "[4] Устанавливаем RPATH → \$ORIGIN/../lib"
patchelf --set-rpath '$ORIGIN/../lib' "$APPDIR/usr/bin/$APPNAME" 2>/dev/null || true

# ==============================
# 6. AppRun
# ==============================
echo "[5] Создаём AppRun"
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="${HERE}/usr/share:${XDG_DATA_DIRS:-/usr/local/share/:/usr/share/}"
exec "${HERE}/usr/bin/@APPNAME@" "$@"
EOF
sed -i "s|@APPNAME@|$APPNAME|g" "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"

# ==============================
# 7. .desktop и иконка
# ==============================
echo "[6] Создаём .desktop и копируем иконку"
cat > "$APPDIR/${APPNAME}.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APPNAME
Icon=$APPNAME
Exec=$APPNAME
Categories=Utility;
Terminal=false
EOF

# Ищем иконку в нескольких популярных местах
for icon_path in \
    "$(dirname "$BINARY")/${APPNAME}.png" \
    "$(dirname "$BINARY")/../${APPNAME}.png" \
    "$(dirname "$BINARY")/resources/${APPNAME}.png" \
    "$(dirname "$BINARY")/../resources/${APPNAME}.png"; do
    if [[ -f "$icon_path" ]]; then
        cp -v "$icon_path" "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APPNAME}.png"
        break
    fi
done

# Если иконки нет — создадим заглушку (чтобы appimagetool не ругался)
if [[ ! -f "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APPNAME}.png" ]]; then
    echo "⚠️  Иконка не найдена, создаём пустую"
    convert -size 256x256 xc:none "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APPNAME}.png" 2>/dev/null || \
    touch "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APPNAME}.png"
fi

cp "$APPDIR/${APPNAME}.desktop" "$APPDIR/.Desktop" 2>/dev/null || cp "$APPDIR/${APPNAME}.desktop" "$APPDIR/"

echo "========================================"
echo "AppDir успешно собран: $APPDIR"
echo ""
echo "Теперь можно собрать AppImage:"
echo "   appimagetool \"$APPDIR\" \"${APPNAME}-x86_64.AppImage\""
echo "   (или linuxdeployqt / appimagetool из https://github.com/AppImage/appimagetool)"
echo "========================================"

exit 0