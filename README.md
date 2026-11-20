# sisGram

An unofficial Telegram client for Symbian^3 / Belle, built in Qt Quick and C++.

**sisGram is a fork of [Kutegram](https://github.com/kutegram/quick).** It is not
affiliated with Telegram or Nokia. The name is a nod to the `.sis` installation
package format used by Symbian.

Every successful build is published as a **GitHub Release**
[here](https://github.com/resymbian/Kutegram/releases/latest). Build outputs are
never uploaded as Actions artifacts; the Releases page is the only place to get
an installable package.

## Devices

| Platform | Examples | Supported? | Qt runtime to install first |
| --- | --- | --- | --- |
| Symbian^3 / Anna / Belle | N8, E7, C7, C6-01, X7, 700, 701, 808 | Yes | `Qt-4.7.4-Symbian3-Belle.sis` |
| S60 5th Edition | N97, 5800, X6, 5230, C5-03 | Best effort | `Qt-4.7.3-S60-5th.sis` |
| S60 3rd Edition FP1/FP2 (e.g. **Nokia E51**) | E51, E71, N95, 6120 | **No** | Qt 4.7 / Qt Quick was never shipped for these devices |

The application uses Qt Quick 1 (`QDeclarativeView`), which requires Qt 4.7.
S60 3rd Edition devices top out at Qt 4.6.3, so sisGram cannot run on an E51
even with Qt installed. S60 5th Edition is included on a best-effort basis with
Qt 4.7.3; Symbian^3/Belle with Qt 4.7.4 is the tested target.

## Installation

1. Go to the [latest release](https://github.com/resymbian/Kutegram/releases/latest).
2. Download **both** the Qt runtime for your device (see the table above) and
   `sisGram.sis`. The Qt packages are the official installers taken from the
   Qt SDK, so there is no need to fetch anything from third-party sites.
3. Copy the files to the phone (USB, Bluetooth or memory card) and open them in
   the file manager.
4. **Install the Qt runtime first**, then `sisGram.sis`.

`sisGram.sis` is self-signed with the toolchain development certificate: the
phone will ask for confirmation during installation. Requested capabilities are
`ReadUserData`, `WriteUserData`, `UserEnvironment`, `NetworkServices` and
`LocalServices` — the self-signable set. `SwEvent` is deliberately not requested
because it requires a paid developer certificate.

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

The Symbian package needs Qt 4.7.4 for Symbian, GCCE and the SBSv2 toolchain.
CI builds it inside `keinedx2/symbian-belle-buildtools:final` (a Qt SDK 1.2.1
image running under Wine):

```sh
docker run --rm \
  -v "$PWD:/work:ro" -v "$OUT:/out" \
  keinedx2/symbian-belle-buildtools:final /work /out
```

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
