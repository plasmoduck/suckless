const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#272822", /* black   */
  [1] = "#FF6188", /* red     */
  [2] = "#A9DC76", /* green   */
  [3] = "#FC9867", /* yellow  */
  [4] = "#78DCE8", /* blue    */
  [5] = "#AB9DF2", /* magenta */
  [6] = "#a1efe4", /* cyan    */
  [7] = "#f8f8f2", /* white   */

  /* 8 bright colors */
  [8]  = "#75715e",  /* black   */
  [9]  = "#f92672",  /* red     */
  [10] = "#a6e22e", /* green   */
  [11] = "#f4bf75", /* yellow  */
  [12] = "#66d9ef", /* blue    */
  [13] = "#ae81ff", /* magenta */
  [14] = "#a1efe4", /* cyan    */
  [15] = "#f9f8f5", /* white   */

  /* special colors */
  [256] = "#272822", /* background */
  [257] = "#f8f8f2", /* foreground */
  [258] = "#f92672",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
