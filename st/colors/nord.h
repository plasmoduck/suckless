const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2E3440", /* black   */
  [1] = "#88C0D0", /* red     */
  [2] = "#BF616A", /* green   */
  [3] = "#5E81AC", /* yellow  */
  [4] = "#EBCB8B", /* blue    */
  [5] = "#A3BE8C", /* magenta */
  [6] = "#D08770", /* cyan    */
  [7] = "#E5E9F0", /* white   */

  /* 8 bright colors */
  [8]  = "#4C566A",  /* black   */
  [9]  = "#88C0D0",  /* red     */
  [10] = "#BF616A", /* green   */
  [11] = "#5E81AC", /* yellow  */
  [12] = "#EBCB8B", /* blue    */
  [13] = "#A3BE8C", /* magenta */
  [14] = "#D08770", /* cyan    */
  [15] = "#8FBCBB", /* white   */

  /* special colors */
  [256] = "#2E3440", /* background */
  [257] = "#E5E9F0", /* foreground */
  [258] = "#E5E9F0",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 256;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
