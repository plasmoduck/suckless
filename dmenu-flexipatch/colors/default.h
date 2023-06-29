/* Everforest colors */
 char *colors[][2] = {
         /*               fg         bg       */
         [SchemeNorm] = { "#eceff4", "#2e3440" },
         [SchemeSel]  = { "#eceff4", "#4c566a" },
         [SchemeOut]  = { "#fb4934", "#312e2d" },
         #if BORDER_PATCH
         [SchemeBorder] = { "#7a8478", "#7a8478" },
         #endif // BORDER_PATCH
