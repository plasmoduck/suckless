const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#333335", /* black   */
  [1] = "#EEEE9E", /* red     */
  [2] = "#EEEE9E", /* green   */
  [3] = "#AAFFFF", /* yellow  */
  [4] = "#AAFFFF", /* blue    */
  [5] = "#D7AFFF", /* magenta */
  [6] = "#D7AFFF", /* cyan    */
  [7] = "#FFFFD7", /* white   */

  /* 8 bright colors */
  [8]  = "#333335",  /* black   */
  [9]  = "#EEEE9E",  /* red     */
  [10] = "#EEEE9E", /* green   */
  [11] = "#AAFFFF", /* yellow  */
  [12] = "#AAFFFF", /* blue    */
  [13] = "#D7AFFF", /* magenta */
  [14] = "#D7AFFF", /* cyan    */
  [15] = "#FFFFD7", /* white   */

  /* special colors */
  [256] = "#FFFFD7", /* background */
  [257] = "#333335", /* foreground */
  [258] = "#EEEE9E",     /* cursor */
};

/* Default colors (colorname index)
* foreground, background, cursor */
unsigned int defaultbg = 256;
unsigned int defaultfg = 257;
unsigned int defaultcs = 258;
unsigned int defaultrcs= 258;

