const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#151515", /* black   */
  [1] = "#AC4142", /* red     */
  [2] = "#90A959", /* green   */
  [3] = "#F4BF75", /* yellow  */
  [4] = "#6A9FB5", /* blue    */
  [5] = "#AA759F", /* magenta */
  [6] = "#75B5AA", /* cyan    */
  [7] = "#D0D0D0", /* white   */

  /* 8 bright colors */
  [8]  = "#505050",  /* black   */
  [9]  = "#AC4142",  /* red     */
  [10] = "#90A959", /* green   */
  [11] = "#F4BF75", /* yellow  */
  [12] = "#6A9FB5", /* blue    */
  [13] = "#AA759F", /* magenta */
  [14] = "#75B5AA", /* cyan    */
  [15] = "#F5F5F5", /* white   */

  /* special colors */
  [256] = "#151515", /* background */
  [257] = "#D0D0D0", /* foreground */
  [258] = "#D0D0D0",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
