const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#32302F", /* black   */
  [1] = "#CC5E50", /* red     */
  [2] = "#A9AC43", /* green   */
  [3] = "#CBA650", /* yellow  */
  [4] = "#7C9A8D", /* blue    */
  [5] = "#BF919D", /* magenta */
  [6] = "#8FAF83", /* cyan    */
  [7] = "#d5c4a1", /* white   */

  /* 8 bright colors */
  [8]  = "#665c54",  /* black   */
  [9]  = "#CC5E50", /* red	*/
  [10] = "#A9AC43", /* green   */
  [11] = "#CBA650", /* yellow  */
  [12] = "#7C9A8D", /* blue    */
  [13] = "#BF919D", /* magenta */
  [14] = "#8FAF83", /* cyan    */
  [15] = "#fbf1c7", /* white   */

  /* special colors */
  [256] = "#32302F", /* background */
  [257] = "#C5B696", /* foreground */
  [258] = "#FABD2F",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
