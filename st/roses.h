const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2B3544", /* black   */
  [1] = "#E3857A", /* red     */
  [2] = "#71B892", /* green   */
  [3] = "#CAC86B", /* yellow  */
  [4] = "#849BED", /* blue    */
  [5] = "#D28594", /* magenta */
  [6] = "#79B5C6", /* cyan    */
  [7] = "#D2D2D4", /* white   */

  /* 8 bright colors */
  [8]  = "#3C4554",  /* black   */
  [9]  = "#ED8F84",  /* red     */
  [10] = "#7BC29C", /* green   */
  [11] = "#D4D276", /* yellow  */
  [12] = "#8EA6F6", /* blue    */
  [13] = "#DC8F9E", /* magenta */
  [14] = "#83BED0", /* cyan    */
  [15] = "#E0E0F2", /* white   */

  /* special colors */
  [256] = "#2B3544", /* background */
  [257] = "#D2D2D4", /* foreground */
  [258] = "#DC8F9E",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
