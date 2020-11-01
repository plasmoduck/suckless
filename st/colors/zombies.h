const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#21232B", /* black   */
  [1] = "#BF5C56", /* red     */
  [2] = "#BF5C56", /* green   */
  [3] = "#97B19C", /* yellow  */
  [4] = "#97B19C", /* blue    */
  [5] = "#D7BD8A", /* magenta */
  [6] = "#D7BD8A", /* cyan    */
  [7] = "#545F72", /* white   */

  /* 8 bright colors */
  [8]  = "#21232B",  /* black   */
  [9]  = "#BF5C56",  /* red     */
  [10] = "#BF5C56", /* green   */
  [11] = "#97B19C", /* yellow  */
  [12] = "#97B19C", /* blue    */
  [13] = "#D7BD8A", /* magenta */
  [14] = "#D7BD8A", /* cyan    */
  [15] = "#F4F4F2", /* white   */

  /* special colors */
  [256] = "#21232B", /* background */
  [257] = "#F4F4F2", /* foreground */
  [258] = "#F4F4F2",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
