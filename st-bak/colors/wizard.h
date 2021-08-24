const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3A2C2C", /* black   */
  [1] = "#A94640", /* red     */
  [2] = "#C34448", /* green   */
  [3] = "#C87750", /* yellow  */
  [4] = "#D89F57", /* blue    */
  [5] = "#F9CC69", /* magenta */
  [6] = "#6A87A1", /* cyan    */
  [7] = "#D6CBBF", /* white   */

  /* 8 bright colors */
  [8]  = "#999999",  /* black   */
  [9]  = "#A94640",  /* red     */
  [10] = "#C34448", /* green   */
  [11] = "#C87750", /* yellow  */
  [12] = "#D89F57", /* blue    */
  [13] = "#F9CC69", /* magenta */
  [14] = "#6A87A1", /* cyan    */
  [15] = "#D6CBBF", /* white   */

  /* special colors */
  [256] = "#3A2C2C", /* background */
  [257] = "#D6CBBF", /* foreground */
  [258] = "#D89F57",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
