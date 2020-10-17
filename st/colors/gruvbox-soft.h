const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#323C44", /* black   */
  [1] = "#BF616A", /* red     */
  [2] = "#A3BE8C", /* green   */
  [3] = "#EBCB8B", /* yellow  */
  [4] = "#81A1C1", /* blue    */
  [5] = "#88C0D0", /* magenta */
  [6] = "#B48EAD", /* cyan    */
  [7] = "#d5c4a1", /* white   */

  /* 8 bright colors */
  [8]  = "#665c54",  /* black   */
  [9]  = "#BF616A",  /* red     */
  [10] = "#A3BE8C", /* green   */
  [11] = "#EBCB8B", /* yellow  */
  [12] = "#81A1C1", /* blue    */
  [13] = "#88C0D0", /* magenta */
  [14] = "#B48EAD", /* cyan    */
  [15] = "#fbf1c7", /* white   */

  /* special colors */
  [256] = "#323C44", /* background */
  [257] = "#d5c4a1", /* foreground */
  [258] = "#FABD2F",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
