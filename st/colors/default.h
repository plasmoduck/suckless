const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3b4252", /* black   */
  [1] = "#bf616a", /* red     */
  [2] = "#a3be8c", /* green   */
  [3] = "#ebcb8b", /* yellow  */
  [4] = "#81a1c1", /* blue    */
  [5] = "#b48ead", /* magenta */
  [6] = "#88c0d0", /* cyan    */
  [7] = "#eceff4", /* white   */

  /* 8 bright colors */
  [8]  = "#4c566a",  /* black   */
  [9]  = "#bf616a",  /* red     */
  [10] = "#a3be8c", /* green   */
  [11] = "#ebcb8b", /* yellow  */
  [12] = "#81a1c1", /* blue    */
  [13] = "#b48ead", /* magenta */
  [14] = "#88c0d0", /* cyan    */
  [15] = "#eceff4", /* white   */

  /* special colors */
  [256] = "#2e3440", /* background */
  [257] = "#eceff4", /* foreground */
  [258] = "#eceff4",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 256;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
