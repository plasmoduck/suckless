SVKBD: Simple Virtual Keyboard
=================================

This is a simple virtual keyboard, intended to be used in environments,
where no keyboard is available.

Installation
------------

	$ make
	$ make install

This will create by default `svkbd-intl`, which is svkbd using an international
layout with multiple layers and overlays, and optimised for mobile devices.

You can create svkbd for additional layouts by doing:

	$ make LAYOUT=$layout

This will take the file `layout.$layout.h` and create `svkbd-$layout`.
`make install` will then pick up the new file and install it accordingly.

Layouts
---------

The following layouts are available:

* **Mobile Layouts:**
    * ``mobile-intl`` - A small international layout optimised for mobile devices. This layout consists of multiple layers which
        can be switched on the fly, and overlays that appear on long-press of certain keys, adding input ability for
        diacritics and other variants, as well as some emoji. The layers are:
        * a basic qwerty layer
        * a layer for numeric input, arrows, and punctuation
        * a layer for function keys, media keys, and arrows
        * a cyrillic layer (ЙЦУКЕН based); the э key is moved to an overlay on е
        * a dialer/numeric layer
    * ``mobile-plain`` - This is a plain layout with only a qwerty layer and numeric/punctuation layer. It was
        originally made for [sxmo](https://sr.ht/~mil/Sxmo/).
* **Traditional layouts**:
    * ``en`` - An english layout without layers (QWERTY)
    * ``de`` - A german layout (QWERTZ)
    * ``ru`` - A russian layout (ЙЦУКЕН)
    * ``sh`` - A serbo-croatian layout using latin script (QWERTZ)

Usage
-----

	$ svkbd-mobile-intl

This will open svkbd at the bottom of the screen, showing the default
international layout.

	$ svkbd-mobile-intl -d

This tells svkbd to announce itself being a dock window, which then
is managed differently between different window managers. If using dwm
and the dock patch, then this will make svkbd being managed by dwm and
some space of the screen being reserved for it.

	$ svkbd-mobile-intl -g 400x200+1+1

This will start svkbd-intl with a size of 400x200 and at the upper left
window corner.

For layouts that consist of multiple layers, you can enable layers on program start through either the ``-l`` flag or
through the ``SVKBD_LAYERS`` environment variable.  They both take a comma separated list of layer names (as defined in
your ``layout.*.h``). Use the ``↺`` button in the bottom-left to cycle through all the layers.

Some layouts come with overlays that will show when certain keys are hold pressed for a longer time. For
example, a long press on the ``a`` key will enable an overview showing all kinds of diacritic combinations for ``a``.

Overlay functionality interferes with the ability to hold a key and have it outputted repeatedly.  You can disable
overlay functionality with the ``-O`` flag or by setting the environment variable ``SVKBD_ENABLEOVERLAYS=0``. There is
also a key on the function layer of the keyboard itself to enable/disable this behaviour on the fly. Its label shows
``≅`` when the overlay functionality is enabled and ``≇`` when not.

Notes
---------

This virtual keyboard does not actually modify the X keyboard layout, the ``mobile-intl``, ``mobile-plain`` and ``en`` layouts simply rely on a standard US QWERTY layout (setxkbmap us) being activated, the other layouts (``de``, ``ru``, ``sh``) require their respective XKB keymaps to be active.

If you use another XKB layout you will get unpredictable output that does not match the labels on the virtual keycaps!

Repository
----------

	git clone https://git.suckless.org/svkbd
