zathura-pdf-poppler
===================

zathura is a highly customizable and functional document viewer based on the girara user interface
library and several document libraries. This plugin for zathura provides PDF support using the
`poppler` rendering library.

Requirements
------------

The following dependencies are required:

* `zathura` (>= 0.4.4)
* `girara`
* `poppler-glib` (>= 0.18)

Installation
------------

To build and install the plugin using meson's ninja backend:

    meson build
    cd build
    ninja
    ninja install

Note that the default backend for meson might vary based on the platform. Please
refer to the meson documentation for platform specific dependencies.

Bugs
----

Please report bugs at http://git.pwmt.org/pwmt/zathura-pdf-poppler.
