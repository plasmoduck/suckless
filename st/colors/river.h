const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#0A141E", /* black   */
  [1] = "#5A92B4", /* red     */
  [2] = "#6EA2C3", /* green   */
  [3] = "#FCB192", /* yellow  */
  [4] = "#FED3B0", /* blue    */
  [5] = "#FEDEB3", /* magenta */
  [6] = "#8CB4CD", /* cyan    */
  [7] = "#f6eedd", /* white   */

  /* 8 bright colors */
  [8]  = "#aca69a",  /* black   */
  [9]  = "#5A92B4",  /* red     */
  [10] = "#6EA2C3", /* green   */
  [11] = "#FCB192", /* yellow  */
  [12] = "#FED3B0", /* blue    */
  [13] = "#FEDEB3", /* magenta */
  [14] = "#8CB4CD", /* cyan    */
  [15] = "#f6eedd", /* white   */

  /* special colors */
  [256] = "#0A141E", /* background */
  [257] = "#f6eedd", /* foreground */
  [258] = "#f6eedd",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
