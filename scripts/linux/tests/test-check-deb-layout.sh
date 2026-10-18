#!/usr/bin/env bash
# test-check-deb-layout.sh: check-deb-layout.sh against synthetic .debs, one per way the layout broke or could break.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
gate="${here}/../check-deb-layout.sh"
work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT
fails=0
check() {
  if "${@:2}"; then printf 'ok    %s\n' "$1"; else printf 'FAIL  %s\n' "$1"; fails=$((fails + 1)); fi
}

# Builds <name>.deb from the good layout plus extra paths (+path) minus removed ones (-path).
make_deb() {
  local name="$1" root="${work}/$1" entry
  shift
  mkdir -p "${root}/DEBIAN"
  printf 'Package: graphicsengine\nVersion: 1.0\nArchitecture: all\nMaintainer: t <t@t>\nDescription: t\n' > "${root}/DEBIAN/control"
  for entry in usr/bin/GraphicsEngine usr/share/applications/GraphicsEngine.desktop \
    usr/share/metainfo/de.jonasheinle.GraphicsEngine.appdata.xml usr/share/icons/hicolor/256x256/apps/Engine_logo.png \
    usr/lib/graphicsengine/GraphicsEngine "$@"; do
    case "${entry}" in
      -*) rm -f "${root}/${entry#-}" ;;
      *) mkdir -p "$(dirname "${root}/${entry#+}")" && printf 'x' > "${root}/${entry#+}" ;;
    esac
  done
  dpkg-deb --root-owner-group -b "${root}" "${work}/${name}.deb" > /dev/null
}

make_deb good
make_deb doubled +usr/usr/share/applications/GraphicsEngine.desktop -usr/share/applications/GraphicsEngine.desktop
make_deb glfw-headers +usr/include/GLFW/glfw3.h
make_deb glfw-static +usr/lib/libglfw3.a
make_deb glfw-config +usr/lib/cmake/glfw3/glfw3Config.cmake
make_deb no-icon -usr/share/icons/hicolor/256x256/apps/Engine_logo.png

check "the good layout passes" bash "${gate}" "${work}/good.deb"
check "a desktop entry under /usr/usr fails" bash -c "! bash '${gate}' '${work}/doubled.deb' >/dev/null"
check "GLFW's headers fail" bash -c "! bash '${gate}' '${work}/glfw-headers.deb' >/dev/null"
check "GLFW's static library fails" bash -c "! bash '${gate}' '${work}/glfw-static.deb' >/dev/null"
check "GLFW's CMake package fails" bash -c "! bash '${gate}' '${work}/glfw-config.deb' >/dev/null"
check "a missing icon fails" bash -c "! bash '${gate}' '${work}/no-icon.deb' >/dev/null"

if [ "${fails}" -gt 0 ]; then
  printf '%d of the deb layout checks failed\n' "${fails}"
  exit 1
fi
printf 'check-deb-layout: all checks passed\n'
