#!/usr/bin/env bash
# Build the S60 3rd Edition FP1 (Nokia E51) QWidget port of sisGram.
#
# Runs INSIDE keinedx2/symbian-belle-buildtools:final as uid 1001:1003.
# The official Nokia Qt SDK 1.1.2 Windows installer is mounted read-only at
# /work/installer.exe; its SHA256 is verified by the workflow before this runs.
# Nothing is installed on the host and no other network access is required.
set -euo pipefail

SRC=/src
WORK=/work
OUT=/out
SDK="$WORK/sdk/Symbian/SDKs/Symbian1Qt463"
LOGS="$WORK/logs"

export HOME="$WORK/home"
export WINEPREFIX="$WORK/wine"
export WINEARCH=win32
export WINEDEBUG=-all

mkdir -p "$WORK/home" "$WORK/wine" "$LOGS" "$WORK/extract2" "$WORK/sdk" "$OUT"

# --- stage the checkout -------------------------------------------------------
rm -rf "$WORK/project"
mkdir -p "$WORK/project"
tar -C "$SRC" --exclude=.git -cf - . | tar -C "$WORK/project" -xf -

if [ -n "${API_ID:-}" ] && [ -n "${API_HASH:-}" ]; then
  cat > "$WORK/project/libkg/apisecrets.h" <<EOF
#ifndef APISECRETS_H
#define APISECRETS_H

#define KUTEGRAM_API_ID $API_ID
#define KUTEGRAM_API_HASH "$API_HASH"

#endif // APISECRETS_H
EOF
else
  cp "$SRC/libkg/apisecrets.h.empty" "$WORK/project/libkg/apisecrets.h"
fi

# --- extract the official SDK components --------------------------------------
# The offline installer is a binarycreator self-extractor: its payload is a
# series of 7z streams. Extracting them avoids running the Windows installer.
python3 - <<'PY'
data = open("/work/installer.exe", "rb").read()
sig = b"7z\xbc\xaf\x27\x1c"
offs = []
i = 0
while True:
    i = data.find(sig, i)
    if i < 0:
        break
    offs.append(i)
    i += 1
offs = [o for n, o in enumerate(offs) if n == 0 or o - offs[n - 1] > 1000]
for n, o in enumerate(offs):
    end = offs[n + 1] if n + 1 < len(offs) else len(data)
    open("/work/extract2/s%04d.7z" % n, "wb").write(data[o:end])
PY

# Components needed for a Qt 4.6.3 / S60 3rd FP1 build:
#   0025 smart installer, 0032 SIS packages, 0049 epoc32 SDK,
#   0050 SBSv2, 0054 Qt 4.6.3, 0056 GCCE 4.4.1, 0057 Perl, 0058 helpers,
#   0069 additional SIS dependencies.
for n in 0025 0032 0049 0050 0054 0056 0057 0058 0069; do
  7z x -y -bso0 -bsp0 -o"$WORK/sdk" "$WORK/extract2/s${n}.7z" >/dev/null
done

mkdir -p "$SDK"
7z x -y -bso0 -bsp0 -o"$SDK" "$WORK/sdk/Symbian/cache/symbian3.7z" >/dev/null
7z x -y -bso0 -bsp0 -o"$SDK" "$WORK/sdk/Symbian/cache/Qt463_Symbian.7z" >/dev/null
find "$WORK/sdk/Symbian/tools" -type f -iname '*.exe' -exec chmod +x {} +

# --- Wine/SBS workarounds (copies inside the isolated workspace only) ---------
python3 - <<'PY'
p = "/work/sdk/Symbian/tools/sbs/win32/python27/Lib/random.py"
s = open(p, "rb").read().decode("latin-1")
old = ("        if a is None:\r\n"
       "            try:\r\n"
       "                a = long(_hexlify(_urandom(16)), 16)\r\n"
       "            except NotImplementedError:\r\n"
       "                import time\r\n"
       "                a = long(time.time() * 256) # use fractional seconds\r\n")
new = ("        if a is None:\r\n"
       "            import time\r\n"
       "            a = long(time.time() * 256) # wine-safe seed\r\n")
assert old in s, "unexpected Python 2.7 random.py"
open(p, "wb").write(s.replace(old, new).encode("latin-1"))

# Qt 4.6.3's qt.conf has a trailing slash here, which makes SBS see
# "bin//moc.exe"; bld.inf treats // as a comment and runs the directory instead.
p = "/work/sdk/Symbian/SDKs/Symbian1Qt463/bin/qt.conf"
s = open(p, "rb").read().decode("latin-1")
old = "Binaries = $(EPOCROOT)bin/\r\n"
assert old in s, "unexpected Qt 4.6.3 qt.conf"
open(p, "wb").write(s.replace(old, "Binaries = $(EPOCROOT)bin\r\n").encode("latin-1"))
PY

cat > "$SDK/env.bat" <<'EOF'
@echo off
echo Setting up environment for Qt Symbian 1 usage...
pushd "%~dp0"
set SDKPREFIX=%CD%
cd ..\..
set SYMBIANDIR=%CD%
set EPOCROOT=
set EPOCROOT=%SDKPREFIX:~2%\
set QTDIR=%SDKPREFIX%
set DEFALT_MKSPEC_PATH=%SDKPREFIX%\mkspecs\default
set GCCPATH=%SYMBIANDIR%\tools\gcce4
set QMAKE_PATH=%SDKPREFIX%\bin
set SBS_GCCE441BIN=%GCCPATH%\bin
set PATH=%SDKPREFIX%\epoc32\gcc\bin;%SDKPREFIX%\epoc32\tools;%SYMBIANDIR%\tools\;%SYMBIANDIR%\tools\perl\bin\;%SYMBIANDIR%\tools\sbs\bin\;%QMAKE_PATH%;%GCCPATH%;%GCCPATH%\bin;%GCCPATH%\arm-none-symbianelf\bin;%DEFALT_MKSPEC_PATH%;%QTDIR%;%PATH%
popd
EOF

cat > "$SDK/devices.xml" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<devices version="1.0">
  <device name="default" epocroot="Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\">
    <epocroot>Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\</epocroot>
    <platform>armv5</platform>
  </device>
</devices>
EOF

cat > "$WORK/project/regen.bat" <<'EOF'
@echo off
call "Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\env.bat"
set EPOCROOT=Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\
cd /d Z:\work\project
del /q bld.inf Makefile 2>nul
qmake -spec symbian-sbsv2 z-e51.pro
echo QMAKE EXIT: %ERRORLEVEL%
EOF

cat > "$WORK/project/sbsbuild.bat" <<'EOF'
@echo off
call "Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\env.bat"
set EPOCROOT=Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\
cd /d Z:\work\project
if "%SBS_CONFIG%"=="" set SBS_CONFIG=arm.v5.urel.gcce4_4_1
echo SBS_CONFIG=[%SBS_CONFIG%]
echo === SBS release-gcce -j 1 ===
sbs -j 1 -c %SBS_CONFIG%
echo SBS EXIT: %ERRORLEVEL%
EOF

cat > "$WORK/project/sispack.bat" <<'EOF'
@echo off
call "Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\env.bat"
set EPOCROOT=Z:\work\sdk\Symbian\SDKs\Symbian1Qt463\
set SBS_HOME=%SYMBIANDIR%\tools\sbs
cd /d Z:\work\project
echo === MAKE sis ===
"%SBS_HOME%\win32\mingw\bin\mingw32-make.exe" sis
echo SIS EXIT: %ERRORLEVEL%
echo === OUTPUT ===
dir /b *.sis* 2>nul
EOF

for f in "$SDK/env.bat" "$WORK/project/regen.bat" "$WORK/project/sbsbuild.bat" "$WORK/project/sispack.bat"; do
  sed -i 's/\r*$/\r/' "$f"
done

# --- qmake -> SBS/GCCE -> SIS --------------------------------------------------
cd /tmp
timeout 180 xvfb-run -a wine cmd /c "Z:\work\project\regen.bat" > "$LOGS/e51-regen.log" 2>&1
grep -q "QMAKE EXIT: 0" "$LOGS/e51-regen.log"

script -qec "timeout 540 xvfb-run -a env SBS_CONFIG=arm.v5.urel.gcce4_4_1 wine cmd /c \"Z:\\work\\project\\sbsbuild.bat\"" \
  "$LOGS/e51-sbs.typescript" > "$LOGS/e51-sbs.log" 2>&1
if ! grep -aq "sbs:errors:0" "$LOGS/e51-sbs.log"; then
  sed -r 's/\x1B\[[0-9;?]*[a-zA-Z]//g' "$LOGS/e51-sbs.log" | tr -d '\r' | tail -80 >&2
  exit 1
fi

timeout 300 xvfb-run -a wine cmd /c "Z:\work\project\sispack.bat" > "$LOGS/e51-sispack.log" 2>&1
grep -q "SIS EXIT: 0" "$LOGS/e51-sispack.log"

# --- collect only official outputs --------------------------------------------
test -s "$WORK/project/sisgrame51.sis"
cp "$WORK/project/sisgrame51.sis" "$OUT/sisGram-E51.sis"

RUNTIME="$WORK/sdk/Symbian/sis/Symbian^1/Qt/4.6.3/qt_installer.sis"
echo "66dd2d9298ef172741211bafcf3b8179184daf2c4ab48894ba659379d51c77bd  $RUNTIME" | sha256sum -c -
cp "$RUNTIME" "$OUT/Qt-4.6.3-S60-3rd-FP1-SmartInstaller.sis"

echo "E51 build complete:"
ls -lh "$OUT/sisGram-E51.sis" "$OUT/Qt-4.6.3-S60-3rd-FP1-SmartInstaller.sis"
