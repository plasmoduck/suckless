const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#1C2731", /* black   */
  [1] = "#B09686", /* red     */
  [2] = "#ACA391", /* green   */
  [3] = "#A1AD8A", /* yellow  */
  [4] = "#88A3AE", /* blue    */
  [5] = "#AD879B", /* magenta */
  [6] = "#96889D", /* cyan    */
  [7] = "#E1E6F9", /* white   */

  /* 8 bright colors */
  [8]  = "#1C2731",  /* black   */
  [9]  = "#B09686",  /* red     */
  [10] = "#ACA391", /* green   */
  [11] = "#A1AD8A", /* yellow  */
  [12] = "#88A3AE", /* blue    */
  [13] = "#AD879B", /* magenta */
  [14] = "#96889D", /* cyan    */
  [15] = "#E1E6F9", /* white   */

  /* special colors */
  [256] = "#1C2731", /* background */
  [257] = "#E1E6F9", /* foreground */
  [258] = "#E1E6F9",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
