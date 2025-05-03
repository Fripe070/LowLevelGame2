meson setup build/release --buildtype release --optimization=3
meson compile -C build/release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
