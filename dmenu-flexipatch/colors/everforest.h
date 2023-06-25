/* Everforest colors */
 char *colors[][2] = {
         /*               fg         bg       */
         [SchemeNorm] = { "#d3c6aa", "#343b44" },
         [SchemeSel]  = { "#343b44", "#9da9a0" },
         [SchemeOut]  = { "#fb4934", "#312e2d" },
         #if BORDER_PATCH
         [SchemeBorder] = { "#7a8478", "#7a8478" },
         #endif // BORDER_PATCH
