# Quectel Connect Manager 1.6.8 for Linux&Android

The `master` branch and all version-only branches contain sources provided as is, directly from Quectel without modifications.

The `yocto_<version>` branches are my own, with modifications to use `dhclient` instead of `udhcpc`. They also fix certain compilation issues when the `USE_DHCLIENT` macro is defined (e.g., -Werror=unused-function).
