#ifndef XML_H
#define XML_H

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
	void (*xmlcdata)(struct xmlparser *, const char *, size_t);
	void (*xmldata)(struct xmlparser *, const char *, size_t);
	void (*xmldataentity)(struct xmlparser *, const char *, size_t);
	void (*xmltagend)(struct xmlparser *, const char *, size_t, int);
	void (*xmltagstart)(struct xmlparser *, const char *, size_t);
	void (*xmltagstartparsed)(struct xmlparser *, const char *,
	      size_t, int);

#ifndef GETNEXT
	/* GETNEXT overridden to reduce function call overhead and further
	   context optimizations. */
	#define GETNEXT getchar_unlocked
#endif

	/* current tag */
	char tag[1024];
	size_t taglen;
	/* current tag is in shortform ? <tag /> */
	int isshorttag;
	/* current attribute name */
	char name[1024];
	/* data buffer used for tag data, CDATA and attribute data */
	char data[BUFSIZ];
} XMLParser;

int xml_entitytostr(const char *, char *, size_t);
void xml_parse(XMLParser *);
#endif
