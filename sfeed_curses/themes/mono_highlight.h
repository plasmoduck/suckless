/* mono theme with highlighting of the active panel.
   The faint attribute may not work on all terminals though.
   The combination bold with faint generally does not work either. */
#define THEME_ITEM_NORMAL()           do {                            } while(0)
#define THEME_ITEM_FOCUS()            do {                            } while(0)
#define THEME_ITEM_BOLD()             do { if (p->focused || !selected) attrmode(ATTR_BOLD_ON); } while(0)
#define THEME_ITEM_SELECTED()         do { attrmode(ATTR_REVERSE_ON); if (!p->focused) attrmode(ATTR_FAINT_ON); } while(0)
#define THEME_SCROLLBAR_FOCUS()       do {                            } while(0)
#define THEME_SCROLLBAR_NORMAL()      do { attrmode(ATTR_FAINT_ON);   } while(0)
#define THEME_SCROLLBAR_TICK_FOCUS()  do { attrmode(ATTR_REVERSE_ON); } while(0)
#define THEME_SCROLLBAR_TICK_NORMAL() do { attrmode(ATTR_REVERSE_ON); } while(0)
#define THEME_LINEBAR()               do { attrmode(ATTR_FAINT_ON);   } while(0)
#define THEME_STATUSBAR()             do { attrmode(ATTR_REVERSE_ON); } while(0)
#define THEME_INPUT_LABEL()           do { attrmode(ATTR_REVERSE_ON); } while(0)
#define THEME_INPUT_NORMAL()          do {                            } while(0)
