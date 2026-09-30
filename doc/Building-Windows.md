# Building VISE on Windows with configured dependencies

Use the normal CMake package/cache settings to point to installed dependencies;
the source no longer contains a developer's absolute dependency paths. A
Visual Studio x64 Release build uses Boost, Eigen3, Protobuf (including protoc),
ImageMagick Magick++/MagickCore, VLFeat, SQLite3, PNG and JPEG. Keep their ABI,
architecture and MSVC runtime compatible. This change does not migrate VISE's
legacy protobuf or image-library interfaces to newer major versions.

For example, with dependencies provisioned under your own directories:

```powershell
cmake -S src -B build -G "Visual Studio 17 2022" -A x64 -T v142 `
  -DCMAKE_PREFIX_PATH="C:/deps/boost;C:/deps/eigen;C:/deps/protobuf;C:/deps/sqlite;C:/deps/vlfeat" `
  -DBOOST_ROOT="C:/deps/boost" `
  -DBoost_USE_STATIC_LIBS=ON `
  -DVLFEAT_INCLUDE_DIR="C:/deps/vlfeat" `
  -DVLFEAT_LIB="C:/deps/vlfeat/bin/win64/vl.lib" `
  -DVISE_BUILD_WINDOWS_CLI=ON
cmake --build build --config Release --target VISE vise-cli
```

The paths are examples, not downloads or a complete dependency installation
recipe. CMake's `FindImageMagick`, `FindProtobuf`, `FindPNG`, `FindJPEG` and
`FindSQLite3` cache variables can be supplied when a package does not install a
discoverable prefix. VLFeat uses `VLFEAT_INCLUDE_DIR` and `VLFEAT_LIB` or searches
the supplied prefixes. Eigen uses its normal `Eigen3_DIR` package setting.

With CMake supporting CMP0091, `CMAKE_MSVC_RUNTIME_LIBRARY` selects the runtime;
the default remains the static runtime, as in the original Windows build. To
use dependencies built with `/MD`, add
`-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL` for Release. The root script keeps
its `/MT` fallback when CMP0091 is unavailable; existing subprojects require
CMake 3.18 or newer for the complete build. FindBoost controls library selection; Boost
auto-linking is disabled so MSVC does not add a conflicting library variant.

`VISE_BUILD_WINDOWS_CLI` defaults to `OFF`. Enabling it adds `vise-cli` beside the
existing `VISE` application and installs it in `bin`; it does not replace the
desktop launcher. Runtime DLLs and ImageMagick configuration files must still
be available beside the executables or through the appropriate runtime search
paths.

Windows packaging inputs are optional cache paths: `VISE_DEPS_BASEDIR`,
`VISE_MAGICK_DEPS_BASEDIR`, `VISE_DEMO_PROJECTS_DIR` and `VISE_ASSET_DIR`. Supply
them to package external DLLs, demo projects and vocabularies. Leaving them
empty permits a source build without a packaging tree; it does not generate
those resources.

Validation for this change used MSVC v142 x64 Release, CMake's VS 2022 generator,
static Boost 1.70 and protobuf 2.6.1 with the `/MD` runtime selection, Eigen 3.3.7,
VLFeat 0.9.21, SQLite 3.35.5, and VISE-compatible ImageMagick/JPEG libraries.
Both applications and the existing Oxford test target built. The older-policy
fallback, the static `/MT` runtime, Linux and macOS were not executed in that
validation run.
