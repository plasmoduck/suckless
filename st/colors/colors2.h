const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3B4252", /* black   */
  [1] = "#BF616A", /* red     */
  [2] = "#A3BE8C", /* green   */
  [3] = "#EBCB8B", /* yellow  */
  [4] = "#81A1C1", /* blue    */
  [5] = "#B48EAD", /* magenta */
  [6] = "#88C0D0", /* cyan    */
  [7] = "#ECEFF4", /* white   */

  /* 8 bright colors */
  [8]  = "#4C566A",  /* black   */
  [9]  = "#BF616A",  /* red     */
  [10] = "#A3BE8C", /* green   */
  [11] = "#EBCB8B", /* yellow  */
  [12] = "#81A1C1", /* blue    */
  [13] = "#B48EAD", /* magenta */
  [14] = "#88C0D0", /* cyan    */
  [15] = "#ECEFF4", /* white   */

  /* special colors */
  [256] = "#2E3440", /* background */
  [257] = "#ECEFF4", /* foreground */
  [258] = "#ECEFF4",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
