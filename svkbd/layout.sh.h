#define KEYS 66
static Key keys_sh[] = {
	{ "`~", XK_quoteleft, 1},
	{ "1!~", XK_1, 1 },
	{ "2\"ˇ", XK_2, 1 },
	{ "3#^", XK_3, 1 },
	{ "4$˘", XK_4, 1 },
	{ "5%°", XK_5, 1 },
	{ "6&˛", XK_6, 1 },
	{ "7/`", XK_7, 1 },
	{ "8(˙", XK_8, 1 },
	{ "9)'", XK_9, 1 },
	{ "0=˝", XK_0, 1 },
	{ "'?¨", XK_apostrophe, 1 },
	{ "+*¸", XK_plus, 1 },
	{ "<-", XK_BackSpace, 2 },
	{ "Del", XK_Delete, 1},
	{ 0 }, /* New row */
	{ "->|", XK_Tab, 1 },
	{ "qQ\\", XK_q, 1 },
	{ "wW|", XK_w, 1 },
	{ "eE", XK_e, 1 },
	{ "rR", XK_r, 1 },
	{ "tT", XK_t, 1 },
	{ "zZ", XK_z, 1 },
	{ "uU", XK_u, 1 },
	{ "iI", XK_i, 1 },
	{ "oO", XK_o, 1 },
	{ "pP", XK_p, 1 },
	{ "šŠ÷", XK_scaron, 1 },
	{ "đĐ×", XK_dstroke, 1 },
	{ "Enter", XK_Return, 3 },
	{ 0 }, /* New row */
	{ 0, XK_Caps_Lock, 2 },
	{ "aA", XK_a, 1 },
	{ "sS", XK_s, 1 },
	{ "dD", XK_d, 1 },
	{ "fF[", XK_f, 1 },
	{ "gG]", XK_g, 1 },
	{ "hH", XK_h, 1 },
	{ "jJ̣̣", XK_j, 1 },
	{ "kKł", XK_k, 1 },
	{ "lLŁ", XK_l, 1 },
	{ "čČ", XK_ccaron, 1 },
	{ "ćĆß", XK_cacute, 1 },
	{ "žŽ¤", XK_zcaron, 1 },
	{ 0 }, /* New row */
	{ 0, XK_Shift_L, 2 },
	{ "<>«»", XK_less, 1 },
	{ "yY", XK_y, 1 },
	{ "xX", XK_x, 1 },
	{ "cC", XK_c, 1 },
	{ "vV@", XK_v, 1 },
	{ "bB{", XK_b, 1 },
	{ "nN}", XK_n, 1 },
	{ "mM§", XK_m, 1 }, /* XXX no symbol */
	{ ",;", XK_comma, 1 },
	{ ".:", XK_period, 1 },
	{ "-_", XK_minus, 1 },
	{ 0, XK_Shift_R, 2 },
	{ 0 }, /* New row */
	{ "Ctrl", XK_Control_L, 2 },
	{ "Win", XK_Super_L, 2 },
	{ "Alt", XK_Alt_L, 2 },
	{ "", XK_space, 5 },
	{ "Alt Gr", XK_ISO_Level3_Shift, 2 },
	{ "Menu", XK_Menu, 2 },
	{ "Ctrl", XK_Control_R, 2 },
};

Buttonmod buttonmods[] = {
	{ XK_Shift_L, Button2 },
	{ XK_Alt_L, Button3 },
};

#define OVERLAYS 1
static Key overlay[OVERLAYS] = {
	{ 0, XK_Cancel },
};

#define LAYERS 1
static char* layer_names[LAYERS] = {
	"sh",
};

static Key* available_layers[LAYERS] = {
	keys_sh,
};
