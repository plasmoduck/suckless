const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2E3233", /* black   */
  [1] = "#D68A8D", /* red     */
  [2] = "#D1B3AF", /* green   */
  [3] = "#9B9D77", /* yellow  */
  [4] = "#7CA3A7", /* blue    */
  [5] = "#E1DAD9", /* magenta */
  [6] = "#92BAB5", /* cyan    */
  [7] = "#EDEDED", /* white   */

  /* 8 bright colors */
  [8]  = "#2E3233",  /* black   */
  [9]  = "#D68A8D",  /* red     */
  [10] = "#D1B3AF", /* green   */
  [11] = "#9B9D77", /* yellow  */
  [12] = "#7CA3A7", /* blue    */
  [13] = "#E1DAD9", /* magenta */
  [14] = "#92BAB5", /* cyan    */
  [15] = "#EDEDED", /* white   */

  /* special colors */
  [256] = "#2E3233", /* background */
  [257] = "#EDEDED", /* foreground */
  [258] = "#FABD2F",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
