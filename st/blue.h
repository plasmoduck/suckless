const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#323D43", /* black   */
  [1] = "#E88081", /* red     */
  [2] = "#A7BE8A", /* green   */
  [3] = "#D8BB7F", /* yellow  */
  [4] = "#8CBCBB", /* blue    */
  [5] = "#DB9ABB", /* magenta */
  [6] = "#88BF97", /* cyan    */
  [7] = "#DACAA6", /* white   */

  /* 8 bright colors */
  [8]  = "#323D43",  /* black   */
  [9]  = "#DC8584",  /* red     */
  [10] = "#A7C179", /* green   */
  [11] = "#DFB77D", /* yellow  */
  [12] = "#89BEBC", /* blue    */
  [13] = "#DB9CBB", /* magenta */
  [14] = "#8DBC9C", /* cyan    */
  [15] = "#868C80", /* white   */

  /* special colors */
  [256] = "#323D43", /* background */
  [257] = "#DACAA6", /* foreground */
  [258] = "#D3C071",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
