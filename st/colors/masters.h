const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#030102", /* black   */
  [1] = "#5C5D5C", /* red     */
  [2] = "#A3151A", /* green   */
  [3] = "#B84531", /* yellow  */
  [4] = "#CB9E48", /* blue    */
  [5] = "#4B788C", /* magenta */
  [6] = "#459AAF", /* cyan    */
  [7] = "#e5c7bb", /* white   */

  /* 8 bright colors */
  [8]  = "#a08b82",  /* black   */
  [9]  = "#5C5D5C",  /* red     */
  [10] = "#A3151A", /* green   */
  [11] = "#B84531", /* yellow  */
  [12] = "#CB9E48", /* blue    */
  [13] = "#4B788C", /* magenta */
  [14] = "#459AAF", /* cyan    */
  [15] = "#e5c7bb", /* white   */

  /* special colors */
  [256] = "#030102", /* background */
  [257] = "#e5c7bb", /* foreground */
  [258] = "#e5c7bb",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
