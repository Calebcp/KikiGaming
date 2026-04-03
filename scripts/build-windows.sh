#!/bin/sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
RAYLIB_VERSION="${RAYLIB_VERSION:-5.5}"
RAYLIB_URL="${RAYLIB_URL:-https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz}"
RAYLIB_CACHE_DEFAULT="$HOME/Library/Caches/Homebrew/downloads"
RAYLIB_ARCHIVE="${RAYLIB_ARCHIVE:-}"

TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchains/mingw64-x86_64.cmake"
WORK_DIR="${ROOT_DIR}/.build/windows"
RAYLIB_SRC_DIR="${WORK_DIR}/raylib-src"
RAYLIB_BUILD_DIR="${WORK_DIR}/raylib-build"
RAYLIB_INSTALL_DIR="${WORK_DIR}/raylib-install"
APP_BUILD_DIR="${WORK_DIR}/app-build"
DIST_DIR="${ROOT_DIR}/.dist/windows"

find_raylib_archive() {
  if [ -n "${RAYLIB_ARCHIVE}" ] && [ -f "${RAYLIB_ARCHIVE}" ]; then
    printf '%s\n' "${RAYLIB_ARCHIVE}"
    return
  fi

  if [ -d "${RAYLIB_CACHE_DEFAULT}" ]; then
    found_archive="$(find "${RAYLIB_CACHE_DEFAULT}" -maxdepth 1 -type f -name "*${RAYLIB_VERSION}.tar.gz" | head -n 1 || true)"
    if [ -n "${found_archive}" ]; then
      printf '%s\n' "${found_archive}"
      return
    fi
  fi

  printf '%s\n' "${WORK_DIR}/raylib-${RAYLIB_VERSION}.tar.gz"
}

ensure_raylib_source() {
  archive_path="$(find_raylib_archive)"

  mkdir -p "${WORK_DIR}"

  if [ ! -f "${archive_path}" ]; then
    echo "Downloading raylib ${RAYLIB_VERSION} source..."
    curl -L "${RAYLIB_URL}" -o "${archive_path}"
  fi

  rm -rf "${RAYLIB_SRC_DIR}"
  mkdir -p "${RAYLIB_SRC_DIR}"
  tar -xzf "${archive_path}" -C "${RAYLIB_SRC_DIR}" --strip-components=1
}

build_raylib() {
  cmake -S "${RAYLIB_SRC_DIR}" -B "${RAYLIB_BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_GAMES=OFF \
    -DCMAKE_INSTALL_PREFIX="${RAYLIB_INSTALL_DIR}"

  cmake --build "${RAYLIB_BUILD_DIR}" --config Release
  cmake --install "${RAYLIB_BUILD_DIR}"
}

build_game() {
  cmake -S "${ROOT_DIR}" -B "${APP_BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${RAYLIB_INSTALL_DIR}"

  cmake --build "${APP_BUILD_DIR}" --config Release
}

package_dist() {
  mkdir -p "${DIST_DIR}"
  cp "${APP_BUILD_DIR}/kikigame.exe" "${DIST_DIR}/KiKigame.exe"
  rm -rf "${DIST_DIR}/assets"
  cp -R "${ROOT_DIR}/assets" "${DIST_DIR}/assets"
  if command -v x86_64-w64-mingw32-strip >/dev/null 2>&1; then
    x86_64-w64-mingw32-strip "${DIST_DIR}/KiKigame.exe"
  fi
}

if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
  echo "Missing x86_64-w64-mingw32-gcc. Install mingw-w64 first." >&2
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "Missing cmake." >&2
  exit 1
fi

if ! command -v curl >/dev/null 2>&1; then
  echo "Missing curl." >&2
  exit 1
fi

ensure_raylib_source
build_raylib
build_game
package_dist

echo "Windows build created at ${DIST_DIR}/KiKigame.exe"
