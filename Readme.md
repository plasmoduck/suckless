# suckless software master collection
These are my builds of all of the suckless software I use. 
Most of the software is set to use Gruvbox as it's color scheme. 
You can change this by modifying the corresponding colors.h file.

# Updates
Fix st not showing w3m images.
To display images in st, like ranger or neofetch for example, you must disable transparancy by changing the alpha value to
```
 unsigned int alpha = 0xff;
```
