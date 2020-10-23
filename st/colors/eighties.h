const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2d2d2d", /* black   */
  [1] = "#f2777a", /* red     */
  [2] = "#99cc99", /* green   */
  [3] = "#ffcc66", /* yellow  */
  [4] = "#6699cc", /* blue    */
  [5] = "#cc99cc", /* magenta */
  [6] = "#66cccc", /* cyan    */
  [7] = "#d3d0c8", /* white   */

  /* 8 bright colors */
  [8]  = "#747369",  /* black   */
  [9]  = "#f2777a",  /* red     */
  [10] = "#99cc99", /* green   */
  [11] = "#ffcc66", /* yellow  */
  [12] = "#6699cc", /* blue    */
  [13] = "#cc99cc", /* magenta */
  [14] = "#66cccc", /* cyan    */
  [15] = "#f2f0ec", /* white   */

  /* special colors */
  [256] = "#2d2d2d", /* background */
  [257] = "#d3d0c8", /* foreground */
  [258] = "#d3d0c8",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
