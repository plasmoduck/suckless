/* TempleOS-like (for fun and god) */
/* set true-color foreground / background, Terry would've preferred ANSI */
#define SETFGCOLOR(r,g,b)    ttywritef("\x1b[38;2;%d;%d;%dm", r, g, b)
#define SETBGCOLOR(r,g,b)    ttywritef("\x1b[48;2;%d;%d;%dm", r, g, b)

#define THEME_ITEM_NORMAL()           do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_ITEM_FOCUS()            do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_ITEM_BOLD()             do { attrmode(ATTR_BOLD_ON); SETFGCOLOR(0xaa, 0x00, 0x00); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_ITEM_SELECTED()         do { if (p->focused) attrmode(ATTR_REVERSE_ON); } while(0)
#define THEME_SCROLLBAR_FOCUS()       do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_SCROLLBAR_NORMAL()      do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_SCROLLBAR_TICK_FOCUS()  do { SETBGCOLOR(0x00, 0x00, 0xaa); SETFGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_SCROLLBAR_TICK_NORMAL() do { SETBGCOLOR(0x00, 0x00, 0xaa); SETFGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_LINEBAR()               do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_STATUSBAR()             do { ttywrite("\x1b[6m"); SETBGCOLOR(0x00, 0x00, 0xaa); SETFGCOLOR(0xff, 0xff, 0xff); } while(0) /* blink statusbar */
#define THEME_INPUT_LABEL()           do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)
#define THEME_INPUT_NORMAL()          do { SETFGCOLOR(0x00, 0x00, 0xaa); SETBGCOLOR(0xff, 0xff, 0xff); } while(0)

#undef SCROLLBAR_SYMBOL_BAR
#define SCROLLBAR_SYMBOL_BAR "\xe2\x95\x91" /* symbol: "double vertical" */
#undef LINEBAR_SYMBOL_BAR
#define LINEBAR_SYMBOL_BAR   "\xe2\x95\x90" /* symbol: "double horizontal" */
#undef LINEBAR_SYMBOL_RIGHT
#define LINEBAR_SYMBOL_RIGHT "\xe2\x95\xa3" /* symbol: "double vertical and left" */
