# sisGram

An unofficial Telegram client for Symbian^3 / Belle and S60 3rd Edition FP1
(e.g. Nokia E51). Symbian^3 / Belle uses Qt Quick; the E51 build is a separate
Qt 4.6.3 / QWidget port that talks MTProto directly.

**sisGram is a fork of [Kutegram](https://github.com/kutegram/quick).** It is not
affiliated with Telegram or Nokia. The name is a nod to the `.sis` installation
package format used by Symbian.

Every successful build is published as a **GitHub Release**
[here](https://github.com/resymbian/Kutegram/releases/latest). Build outputs are
never uploaded as Actions artifacts; the Releases page is the only place to get
an installable package.

## Devices

| Platform | Examples | Supported? | Files to install, in order |
| --- | --- | --- | --- |
| Symbian^3 / Anna / Belle | N8, E7, C7, C6-01, X7, 700, 701, 808 | Yes | `Qt-4.7.4-Symbian3-Belle.sis` → `sisGram.sis` |
| S60 5th Edition | N97, 5800, X6, 5230, C5-03 | Best effort | `Qt-4.7.3-S60-5th.sis` → `sisGram.sis` |
| S60 3rd Edition FP1 (**Nokia E51**) | E51, 6120 | Yes (QWidget port) | `Qt-4.6.3-S60-3rd-FP1-SmartInstaller.sis` → `sisGram-E51.sis` |
| S60 3rd Edition FP2 | E71, N95, 6220 | Unverified | Same E51 packages may work, but only FP1 has been built for |

Symbian^3/Belle uses the original Qt Quick 1 UI (Qt 4.7.4). S60 3rd Edition FP1
cannot run Qt Quick: it tops out at official Qt 4.6.3, so `sisGram-E51.sis` uses
a separate QWidget interface and the same `libkg` protocol library. Transport
is direct MTProto over `QTcpSocket` — no JVM, proxy or relay is needed.

[**J2MEgram**](https://github.com/smbdsbrain/J2MEgram) remains a usable Java ME
alternative for these phones; the E51 is among its tested devices. Its tested
setup used Wi-Fi with an external MTProxy on a high port, so the native Qt build
above is the simpler option when it installs successfully.

## Installation

1. Go to the [latest release](https://github.com/resymbian/Kutegram/releases/latest).
2. Download the Qt runtime and the app package for your device (see the table
   above). All Qt packages are the official ones shipped inside Nokia Qt SDKs;
   no third-party downloads are needed.
3. Copy the files to the phone (USB, Bluetooth or memory card) and open them in
   the file manager.
4. **Install the Qt runtime first**, then the app.

The E51 runtime is Nokia's Qt Smart Installer. It contacts Nokia's distribution
servers to fetch Qt 4.6.3, so the phone needs a data connection at install time.
If that service is unavailable, use J2MEgram instead.

Both `sisGram.sis` and `sisGram-E51.sis` are self-signed with the toolchain
development certificate: the phone will ask for confirmation during installation.
Requested capabilities are `ReadUserData`, `WriteUserData`, `UserEnvironment`,
`NetworkServices` and `LocalServices` — the self-signable set. `SwEvent` is
deliberately not requested because it requires a paid developer certificate.

## Telegram API credentials

Telegram requires every client to identify itself with an `api_id` and an
`api_hash`. The release build has the project's values pre-filled from the
`KUTEGRAM_API_ID` / `KUTEGRAM_API_HASH` GitHub Secrets, so it can log in out of
the box.

You can override them from the first screen of the app:

1. Visit [my.telegram.org/apps](https://my.telegram.org/apps), sign in with your
   phone number and create an application.
2. Note the **API ID** (a number) and **API hash** (32 hex characters).
3. Enter them in the `Telegram API ID / hash` fields on the intro screen and
   press **Start**. They are saved in the device settings and reused on the
   next launch.

These are not your phone number, SMS login code or password. If you use your
own credentials, Telegram sees your app rather than the project's one. Note
that credentials baked into a public binary can be extracted from it; use your
own if that matters to you.

## Building from source

The Symbian^3/Belle package needs Qt 4.7.4 for Symbian, GCCE and the SBSv2
toolchain. CI builds it inside `keinedx2/symbian-belle-buildtools:final` (a Qt
SDK 1.2.1 image running under Wine):

```sh
docker run --rm \
  -v "$PWD:/work:ro" -v "$OUT:/out" \
  keinedx2/symbian-belle-buildtools:final /work /out
```

The E51 build uses Qt 4.6.3 from the original Nokia Qt SDK 1.1.2. CI verifies
the installer hash `ce16cbd7c99e607f51789d857fe8573852a999053a79af1fa20d645457044e30`,
then extracts and builds it only inside the same container as uid `1001:1003`:

```sh
curl -fLO https://archive.org/download/nokia_sdks_n_dev_tools/Qt_SDK_Win_offline_v1_1_2_en.exe
echo "ce16cbd7c99e607f51789d857fe8573852a999053a79af1fa20d645457044e30  Qt_SDK_Win_offline_v1_1_2_en.exe" | sha256sum -c -
mkdir -p e51-out e51-work
docker run --rm --platform linux/amd64 --entrypoint bash \
  --user 1001:1003 --network=none \
  -v "$PWD:/src:ro" -v "$PWD/e51-work:/work" -v "$PWD/e51-out:/out" \
  -v "$PWD/Qt_SDK_Win_offline_v1_1_2_en.exe:/work/installer.exe:ro" \
  keinedx2/symbian-belle-buildtools:final /src/tools/build-e51.sh
```

`tools/build-e51.sh` never executes the Windows installer: it extracts the
official 7z payload components, builds with GCCE 4.4.1 and writes
`sisGram-E51.sis` plus the official Qt Smart Installer to `e51-out/`.

For a local Qt 5 compile-only check:

```sh
mkdir build && cd build
cp ../libkg/apisecrets.h.empty ../libkg/apisecrets.h  # needed to build
qmake ../kutegramquick.pro CONFIG+=release
make -j"$(nproc)"
```

`libkg` is a submodule pointing at
[resymbian/libkg](https://github.com/resymbian/libkg), a fork that reads the
credentials at runtime. Clone with `--recursive` or run
`git submodule update --init --recursive`.

## Credits

- [Kutegram](https://github.com/kutegram/quick) — the original client this is forked from.
- [libkg](https://github.com/kutegram/libkg) — Telegram protocol library.
- Qt and the Symbian SDK by Nokia/Qt Project.
