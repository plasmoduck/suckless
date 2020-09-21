/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nogroup";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "black",     /* after initialization */
	[INPUT] =  "#005577",   /* during input */
	[FAILED] = "#CC3333",   /* wrong password */
    [CAPS] = "red",         /* CapsLock on */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 0;

/* default message */
static const char * message = "🔒Password 🔒";

/* text color */
static const char * text_color = "#FB4934";

/* text size (must be a valid size) */
/*static const char * font_name = "-b&h-luxi mono-bold-o-normal--0-0-0-0-m-0-iso8859-15";*/
static const char * font_name = "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-1";

/*Enable blur*/
#define BLUR
/*Set blur radius*/
static const int blurRadius=5;

/*Enable Pixelation*/
/* #define PIXELATION */
/*Set pixelation radius*/
/*static const int pixelSize=5; */

/* time in seconds before the monitor shuts down */
static const int monitortime = 5;

/* allow control key to trigger fail on clear */
static const int controlkeyclear = 0;
