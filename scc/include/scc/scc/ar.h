#define ARMAG      "!<arch>\n"     /* ar "magic number" */
#define SARMAG     8               /* strlen(ARMAG); */
#define ARFMAG     "`\n"
#define SARNAM     16


struct ar_hdr {
        char ar_name[SARNAM];           /* name */
        char ar_date[12];               /* modification time */
        char ar_uid[6];                 /* user id */
        char ar_gid[6];                 /* group id */
        char ar_mode[8];                /* octal file permissions */
        char ar_size[10];               /* size in bytes */
        char ar_fmag[2];                /* consistency check */
};
