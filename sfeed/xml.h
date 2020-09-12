#ifndef _XML_H
#define _XML_H

#include <stdio.h>

typedef struct xmlparser {
	/* handlers */
	void (*xmlattr)(struct xmlparser *, const char *, size_t,
	      const char *, size_t, const char *, size_t);
	void (*xmlattrend)(struct xmlparser *, const char *, size_t,
	      const char *, size_t);
	void (*xmlattrstart)(struct xmlparser *, const char *, size_t,
	      const char *, size_t);
	void (*xmlattrentity)(struct xmlparser *, const char *, size_t,
	      const char *, size_t, const char *, size_t);
	void (*xmlcdatastart)(struct xmlparser *);
	void (*xmlcdata)(struct xmlparser *, const char *, size_t);
	void (*xmlcdataend)(struct xmlparser *);
	void (*xmlcommentstart)(struct xmlparser *);
	void (*xmlcomment)(struct xmlparser *, const char *, size_t);
	void (*xmlcommentend)(struct xmlparser *);
	void (*xmldata)(struct xmlparser *, const char *, size_t);
	void (*xmldataend)(struct xmlparser *);
	void (*xmldataentity)(struct xmlparser *, const char *, size_t);
	void (*xmldatastart)(struct xmlparser *);
	void (*xmltagend)(struct xmlparser *, const char *, size_t, int);
	void (*xmltagstart)(struct xmlparser *, const char *, size_t);
	void (*xmltagstartparsed)(struct xmlparser *, const char *,
	      size_t, int);

#ifndef GETNEXT
	/* GETNEXT overridden for sfeed to reduce function call overhead and
	   further context optimizations. */
	#define GETNEXT getchar
#endif

	/* current tag */
	char tag[1024];
	size_t taglen;
	/* current tag is in short form ? <tag /> */
	int isshorttag;
	/* current attribute name */
	char name[1024];
	/* data buffer used for tag data, cdata and attribute data */
	char data[BUFSIZ];
} XMLParser;

int xml_entitytostr(const char *, char *, size_t);
void xml_parse(XMLParser *);
#endif
