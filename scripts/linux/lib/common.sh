#!/usr/bin/env bash
# common.sh - the hub's core modules for these scripts; antfrastructure.sh fails without the hub, so nothing falls back.

# Never a ../../.. literal: the bootstrap honours the ANTFRASTRUCTURE_DIR override the container needs.
# shellcheck source=/dev/null
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/antfrastructure.sh"

# logging.sh first, the rest call info/warn/err; load-versions-env.sh only defines load_versions_env, it exports nothing.
for _core_module in logging.sh platform.sh verify.sh parallelism.sh vulkan-env.sh tool-checks.sh load-versions-env.sh; do
  antfrastructure_source "linux/scripts/01-core/${_core_module}"
done
unset _core_module

# strict=0: a dev box without an SDK warns and proceeds, unlike the image-side source_vulkan_sdk_env.
source_vulkan_env() {
  vulkan_env_source "" "keep-libs" 0
  return 0
}

# Get script root directory
get_script_root() {
  cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd
}

# Get project root directory
get_project_root() {
  # common.sh lives in scripts/linux/lib; go three levels up to reach repo root
  cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd
}

# Applied on source so every script here gets the hub's Rust toolchain and CARGO_HOME guards; never copy them.
antfrastructure_source linux/scripts/02-toolchain/rust/_rust_toolchain_guard.sh
antfrastructure_source linux/scripts/02-toolchain/rust/_cargo_home_guard.sh
