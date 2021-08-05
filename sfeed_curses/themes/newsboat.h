/* newsboat-like (blue, yellow) */
#define THEME_ITEM_NORMAL()           do {                          } while(0)
#define THEME_ITEM_FOCUS()            do {                          } while(0)
#define THEME_ITEM_BOLD()             do { attrmode(ATTR_BOLD_ON);  } while(0)
#define THEME_ITEM_SELECTED()         do { if (p->focused) ttywrite("\x1b[93;44m"); } while(0) /* bright yellow fg, blue bg */
#define THEME_SCROLLBAR_FOCUS()       do { ttywrite("\x1b[34m");    } while(0) /* blue fg */
#define THEME_SCROLLBAR_NORMAL()      do { ttywrite("\x1b[34m");    } while(0)
#define THEME_SCROLLBAR_TICK_FOCUS()  do { ttywrite("\x1b[44m");    } while(0) /* blue bg */
#define THEME_SCROLLBAR_TICK_NORMAL() do { ttywrite("\x1b[44m");    } while(0)
#define THEME_LINEBAR()               do { ttywrite("\x1b[34m");    } while(0)
#define THEME_STATUSBAR()             do { attrmode(ATTR_BOLD_ON); ttywrite("\x1b[93;44m"); } while(0)
#define THEME_INPUT_LABEL()           do {                          } while(0)
#define THEME_INPUT_NORMAL()          do {                          } while(0)
