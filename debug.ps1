meson setup build/debug
meson compile -C build/debug
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
