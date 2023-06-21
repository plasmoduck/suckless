const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#343b44", /* black   */
  [1] = "#e67e80", /* red     */
  [2] = "#a7c080", /* green   */
  [3] = "#dbbc7f", /* yellow  */
  [4] = "#7fbbb3", /* blue    */
  [5] = "#d699b6", /* magenta */
  [6] = "#83c092", /* cyan    */
  [7] = "#d3c6aa", /* white   */

  /* 8 bright colors */
  [8]  = "#7a8478",  /* black   */
  [9]  = "#e67e80",  /* red     */
  [10] = "#a7c080", /* green   */
  [11] = "#dbbc7f", /* yellow  */
  [12] = "#7fbbb3", /* blue    */
  [13] = "#d699b6", /* magenta */
  [14] = "#83c092", /* cyan    */
  [15] = "#9da9a0", /* white   */

  /* special colors */
  [256] = "#343b44", /* background */
  [257] = "#d3c6aa", /* foreground */
  [258] = "#d3c6aa",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
