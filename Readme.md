# suckless utilities collection
These are my patched builds of all of the suckless tools I use.

Also, the build flags are set for FreeBSD (cause that's what I use). So please modify config.mk to suit your system.

Most of the software is set to use Gruvbox as it's color scheme. 
You can change this by modifying the corresponding colors.h or config.def.h file.

## INSTALL
To install simple set the build flags for your system by editing:
    ```
    config.mk
    ```
Then configure the software by editing:
    ```
    config.def.h
    ```
Lastly, build & install:
    ```
    doas/sudo make install
    ```

## Updates
### 13-09-2020 - Added modular dmenu build. Edit ``patches.def.h`` to select modules, then customise ``config.def.h`` & build ``doas/sudo make install``. 

### 08-09-2020 - Fix st not showing w3m image previews. 
  This patch achieves the same goal but instead of canceling the double
  buffer it first copies the front buffer to the back buffer.
  This has the same issues as the FAQ patch in that the cursor line is
  deleted at the image (because st renders always full lines), but
  otherwise it's simpler and does keeps double buffering.
  Refer to [w3m patch](https://st.suckless.org/patches/w3m/)
  
## ST IMAGES
To display w3m images in st patched with [alpha](https://st.suckless.org/patches/alpha/) transparency (ranger/neofetch) etc. You must disable transparancy by editing
    ```
    config.def.h
    ```
and change the alpha value to
    ```
    unsigned int alpha = 0xff;
    ```
This will disable transparency of st whilst still allowing you to run a compositor for other programs.
