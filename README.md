zathura-pdf-poppler
===================

zathura is a highly customizable and functional document viewer based on the girara user interface
library and several document libraries. This plugin for zathura provides PDF support using the
`poppler` rendering library.

Requirements
------------

The following dependencies are required:

* `zathura` (>= 2026.01.30)
* `girara`
* `poppler-glib` (>= 21.12)

Installation
------------

To build and install the plugin using meson's ninja backend:

    meson build
    cd build
    ninja
    ninja install

> **Note:** The default backend for meson might vary based on the platform. Please
refer to the meson documentation for platform specific dependencies.

Bugs
----

Please report bugs at https://github.com/pwmt/zathura-pdf-poppler.
