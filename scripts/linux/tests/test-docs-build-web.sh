#!/usr/bin/env bash
# test-docs-build-web.sh: docs-build-web.sh's wasm-bindgen pin from LF, CRLF, pin-less and missing Cargo.lock files.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/linux/lib/antfrastructure.sh
source "${here}/../lib/antfrastructure.sh"
_harness="$(antfrastructure_path linux/scripts/lib/test-harness.sh)" || exit 1
# shellcheck source=third_party/ANTfrastructure/linux/scripts/lib/test-harness.sh
source "${_harness}"
logging="$(antfrastructure_path linux/scripts/01-core/logging.sh)" || exit 1
work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

# The backend first: a match on the name's prefix would take its version instead.
lock='[[package]]\nname = "wasm-bindgen-backend"\nversion = "0.2.99"\n\n[[package]]\nname = "wasm-bindgen"\nversion = "0.2.100"\n'

# make_repo <name> <lock|missing>: the real script, a common.sh with the hub's logging and stubbed hub calls, cargo and wasm-bindgen stubs.
make_repo() {
  local repo="${work}/$1" web="${work}/$1/third_party/OxidANT/crates/webgpu_renderer/web"
  mkdir -p "${repo}/scripts/linux/lib" "${repo}/bin" "${web}"
  cp "${here}/../docs-build-web.sh" "${repo}/scripts/linux/"
  printf 'source %q\nantfrastructure_source() { :; }\nensure_wasm32_target() { :; }\ndocs_build_main() { echo "docs_build_main reached"; }\n' \
    "${logging}" > "${repo}/scripts/linux/lib/common.sh"
  printf '#!/usr/bin/env bash\necho "cargo $*" >> %q\n' "${repo}/calls" > "${repo}/bin/cargo"
  cat > "${repo}/bin/wasm-bindgen" <<'EOF'
#!/usr/bin/env bash
[ "$1" != --version ] || { echo "wasm-bindgen 0.0.0"; exit 0; }
mkdir -p "$3" && touch "$3/kataglyphis_webgpu_renderer.js" "$3/kataglyphis_webgpu_renderer_bg.wasm"
EOF
  chmod +x "${repo}/bin/cargo" "${repo}/bin/wasm-bindgen"
  printf '<!doctype html>\n' > "${web}/index.html"
  [ "$2" = missing ] || printf '%b' "$2" > "${repo}/third_party/OxidANT/Cargo.lock"
}

# run_script <repo>: the script's exit code; its output goes to <repo>/out.
run_script() { (cd "$1" && env -u GITHUB_ACTIONS PATH="$1/bin:${PATH}" bash scripts/linux/docs-build-web.sh > "$1/out" 2>&1); }

make_repo lf "${lock}"
make_repo crlf "${lock//'\n'/'\r\n'}"
make_repo pinless '[[package]]\nname = "serde"\nversion = "1.0.0"\n'
make_repo missing missing

for shape in lf crlf; do
  repo="${work}/${shape}"
  t_case "a ${shape} Cargo.lock pins wasm-bindgen-cli to wasm-bindgen's version and builds the demo"
  t_assert_rc 0 run_script "${repo}"
  t_assert_grep "${repo}/calls" '^cargo install --locked wasm-bindgen-cli --version 0\.2\.100$'
  t_assert_grep "${repo}/out" 'WebGPU WASM demo built from current source'
  t_assert_ok test -f "${repo}/docs/source/_webgpu_demo/webgpu-demo/pkg/kataglyphis_webgpu_renderer_bg.wasm"
done

for shape in pinless missing; do
  repo="${work}/${shape}"
  t_case "a ${shape} Cargo.lock stops the script with a message, before the docs build"
  t_assert_rc 1 run_script "${repo}"
  t_assert_grep "${repo}/out" 'No wasm-bindgen version in third_party/OxidANT/Cargo.lock'
  t_assert_no_grep "${repo}/out" 'docs_build_main reached'
done
t_summary
