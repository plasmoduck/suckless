const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3B4252", /* black   */
  [1] = "#BF616A", /* red     */
  [2] = "#A3BE8C", /* green   */
  [3] = "#EBCB8B", /* yellow  */
  [4] = "#81A1C1", /* blue    */
  [5] = "#B48EAD", /* magenta */
  [6] = "#8FBCBB", /* cyan    */
  [7] = "#D8DEE9", /* white   */

  /* 8 bright colors */
  [8]  = "#323D43",  /* black   */
  [9]  = "#BF616A",  /* red     */
  [10] = "#A3BE8C", /* green   */
  [11] = "#EBCB8B", /* yellow  */
  [12] = "#81A1C1", /* blue    */
  [13] = "#B48EAD", /* magenta */
  [14] = "#8FBCBB", /* cyan    */
  [15] = "#D8DEE9", /* white   */

  /* special colors */
  [256] = "#3B4252", /* background */
  [257] = "#D8DEE9", /* foreground */
  [258] = "#D3C071",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
