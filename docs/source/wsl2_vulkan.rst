WSL2 — Vulkan (NVIDIA)
======================

How to run Vulkan applications on an NVIDIA GPU in WSL2. If your application
reports only ``llvmpipe`` (Mesa's software renderer), the NVIDIA Vulkan ICD is
usually missing from the WSL distribution.

Summary
-------

- Cause: the Vulkan loader loads only Mesa ICDs (e.g. ``lvp``/``llvmpipe``),
  because the NVIDIA ICD is not installed or not found.
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

2. Find the installed NVIDIA driver version (optional):

   .. code-block:: bash

      nvidia-smi

      # Note the Driver Version (e.g. 595.97); package names often carry its major version.

3. Install the matching headless/ICD package in WSL, here for driver version 595:

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

If a vendor JSON exists but is not loaded by default, set ``VK_ICD_FILENAMES``
to it for a test:

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
