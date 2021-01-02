#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <webkitdom/webkitdom.h>

#include "adblock.h"

typedef struct Page {
	guint64 id;
	WebKitWebPage *webpage;
	struct Page *next;
} Page;

static Page *pages;

static Page *
newpage(WebKitWebPage *page)
{
	Page *p;

	if (!(p = calloc(1, sizeof(Page)))) {
		fprintf(stderr, "surf-adblock: calloc: %s\n", strerror(errno));
		return NULL;
	}
	p->id = webkit_web_page_get_id(page);
	p->webpage = page;
	p->next = pages;

	pages = p;

	return p;
}

static void
documentloaded(WebKitWebPage *wp, Page *p)
{
	WebKitDOMDocument *doc = webkit_web_page_get_dom_document(wp);
	WebKitDOMHTMLElement *head = webkit_dom_document_get_body(doc);
	WebKitDOMElement *el;
	const char *uri = webkit_web_page_get_uri(p->webpage);
	char *css, *globalcss;

	if ((globalcss = getglobalcss())) {
		el = webkit_dom_document_create_element(doc, "style", NULL);
		webkit_dom_element_set_attribute(el, "type", "text/css", NULL);
		webkit_dom_element_set_inner_html(el, globalcss, NULL);
		webkit_dom_node_append_child(WEBKIT_DOM_NODE(head),
					     WEBKIT_DOM_NODE(el), NULL);
	}

	if ((css = getdocumentcss(uri))) {
		el = webkit_dom_document_create_element(doc, "style", NULL);
		webkit_dom_element_set_attribute(el, "type", "text/css", NULL);
		webkit_dom_element_set_inner_html(el, css, NULL);
		webkit_dom_node_append_child(WEBKIT_DOM_NODE(head),
					     WEBKIT_DOM_NODE(el), NULL);
	}

	free(css);
	/* NOTE: globalcss should not be free'd */
}

static gboolean
sendrequest(WebKitWebPage *wp, WebKitURIRequest *req,
                   WebKitURIResponse *res, Page *p)
{
	const char *fromuri, *requri;

	if (!webkit_uri_request_get_http_method(req))
		return TRUE; /* TRUE = don't handle any more events */
	fromuri = webkit_web_page_get_uri(p->webpage);
	requri = webkit_uri_request_get_uri(req);

	return allowrequest(fromuri, requri) ? FALSE : TRUE;
}

static void
webpagecreated(WebKitWebExtension *e, WebKitWebPage *p, gpointer unused)
{
	Page *np;

	if (!(np = newpage(p))) {
		fprintf(stderr, "surf-adblock: cannot associate webext with new page\n");
		return;
	}

	g_signal_connect(p, "document-loaded", G_CALLBACK(documentloaded), np);
	g_signal_connect(p, "send-request", G_CALLBACK(sendrequest), np);
}

G_MODULE_EXPORT void
webkit_web_extension_initialize(WebKitWebExtension *ext)
{
	init();
	g_signal_connect(ext, "page-created", G_CALLBACK(webpagecreated), NULL);
}
