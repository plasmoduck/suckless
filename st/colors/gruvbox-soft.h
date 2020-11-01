const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#32302F", /* black   */
  [1] = "#DD6253", /* red     */
  [2] = "#B6B945", /* green   */
  [3] = "#DBB253", /* yellow  */
  [4] = "#83a598", /* blue    */
  [5] = "#CE9BA9", /* magenta */
  [6] = "#99BC8C", /* cyan    */
  [7] = "#d5c4a1", /* white   */

  /* 8 bright colors */
  [8]  = "#665c54",  /* black   */
  [9]  = "#DD6253",  /* red     */
  [10] = "#B6B945", /* green   */
  [11] = "#DBB253", /* yellow  */
  [12] = "#83a598", /* blue    */
  [13] = "#CE9BA9", /* magenta */
  [14] = "#99BC8C", /* cyan    */
  [15] = "#fbf1c7", /* white   */

  /* special colors */
  [256] = "#32302F", /* background */
  [257] = "#d5c4a1", /* foreground */
  [258] = "#FABD2F",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
 unsigned int defaultrcs= 258;
