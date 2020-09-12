# suckless software master collection
These are my patched builds of all of the suckless tools I use. Dmenu is modular, so you can edit patches.def.h to enable/disale what you need.

Also, the build flags are set for FreeBSD (cause that's what I use). So please modify config.mk to suit your system.

Most of the software is set to use Gruvbox as it's color scheme. 
You can change this by modifying the corresponding colors.h or config.def.h file.

# Updates
Fix st not showing w3m images.
To display images in st, like ranger or neofetch for example, you must disable transparancy by changing the alpha value to
```
 unsigned int alpha = 0xff;
```
