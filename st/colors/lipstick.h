const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#0F0F0F", /* black   */
  [1] = "#E182A9", /* red     */
  [2] = "#A9E182", /* green   */
  [3] = "#E1B982", /* yellow  */
  [4] = "#82A9E1", /* blue    */
  [5] = "#B982E1", /* magenta */
  [6] = "#82E1B9", /* cyan    */
  [7] = "#F0F0F0", /* white   */

  /* 8 bright colors */
  [8]  = "#0F0F0F",  /* black   */
  [9]  = "#E182A9",  /* red     */
  [10] = "#A9E182", /* green   */
  [11] = "#E1B982", /* yellow  */
  [12] = "#82A9E1", /* blue    */
  [13] = "#B982E1", /* magenta */
  [14] = "#82E1B9", /* cyan    */
  [15] = "#F0F0F0", /* white   */

  /* special colors */
  [256] = "#0F0F0F", /* background */
  [257] = "#F0F0F0", /* foreground */
  [258] = "#82E1B9",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
