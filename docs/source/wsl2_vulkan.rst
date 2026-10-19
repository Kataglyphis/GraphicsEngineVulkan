WSL2 — Vulkan (NVIDIA)
======================

Running Vulkan applications on an NVIDIA GPU in WSL2.

Summary
-------

- Symptom: the application reports only ``llvmpipe`` (Mesa's software renderer).
- Cause: the Vulkan loader loads only Mesa ICDs (e.g. ``lvp``/``llvmpipe``),
  because the NVIDIA Vulkan ICD is not installed in the WSL distribution or not found.
- Fix: install the matching headless/ICD package in WSL, restart WSL and check
  with ``vulkaninfo``.

Prerequisites
-------------

- Windows: a current NVIDIA driver with WSL/WSLg support (Windows 11
  recommended). ``nvidia-smi`` in WSL should list the card.
- WSL distribution: ``apt`` (or its package manager), network access and root.

Steps (Debian/Ubuntu-based distributions)
-----------------------------------------

1. If needed, install the Vulkan tools (for ``vulkaninfo``):

   .. code-block:: bash

      sudo apt update
      sudo apt install vulkan-tools
      sudo apt install nvidia-driver-595

2. Optionally, find the installed NVIDIA driver version:

   .. code-block:: bash

      nvidia-smi

      # Note the Driver Version (e.g. 595.97); package names often carry its major version.

3. Install the matching headless/ICD package, here for driver version 595:

   .. code-block:: bash

      # List the available headless packages
      apt search nvidia-headless | sed -n '1,200p'

      # Example (replace it if your distribution shows other package names):
      sudo apt install --reinstall nvidia-headless-595 nvidia-utils-595

      # Optional, if available:
      sudo apt install libnvidia-gl-595 libnvidia-vulkan-595

4. Shut WSL down completely (from Windows) and start it again:

   .. code-block:: powershell

      wsl --shutdown

      # Then open WSL again (e.g. restart the terminal).

5. Check in WSL that the NVIDIA ICD is there and ``vulkaninfo`` shows the GPU:

   .. code-block:: bash

      ls -l /usr/share/vulkan/icd.d
      vulkaninfo | grep -i 'Device Name' -A2

      # or the full output
      vulkaninfo | head -n 60

Temporary workaround
--------------------

A vendor JSON that exists but is not loaded by default can be forced for a test
with ``VK_ICD_FILENAMES``:

.. code-block:: bash

   export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia.json
   vulkaninfo | head -n 20

Troubleshooting
---------------

- ``nvidia-smi`` works in WSL but only ``llvmpipe`` appears: most likely the
  ``libnvidia-vulkan-<version>`` package or the ICD JSON in
  ``/usr/share/vulkan/icd.d`` is missing.
- Container and VM setups also need GPU passthrough or the
  ``nvidia-container-toolkit``.
- After changing packages, always run ``wsl --shutdown`` from Windows; an
  ordinary WSL restart is not enough.

Links
-----

- NVIDIA's WSL documentation: https://developer.nvidia.com/wsl
- Vulkan loader and ICDs: https://github.com/KhronosGroup/Vulkan-Loader
