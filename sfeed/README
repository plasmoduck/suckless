sfeed
-----

RSS and Atom parser (and some format programs).

It converts RSS or Atom feeds from XML to a TAB-separated file. There are
formatting programs included to convert this TAB-separated format to various
other formats. There are also some programs and scripts included to import and
export OPML and to fetch, filter, merge and order feed items.


Build and install
-----------------

$ make
# make install


Usage
-----

Initial setup:

	mkdir -p "$HOME/.sfeed/feeds"
	cp sfeedrc.example "$HOME/.sfeed/sfeedrc"

Edit the sfeedrc(5) configuration file and change any RSS/Atom feeds. This file
is included and evaluated as a shellscript for sfeed_update, so it's functions
and behaviour can be overridden:

	$EDITOR "$HOME/.sfeed/sfeedrc"

or you can import existing OPML subscriptions using sfeed_opml_import(1):

	sfeed_opml_import < file.opml > "$HOME/.sfeed/sfeedrc"

an example to export from an other RSS/Atom reader called newsboat and import
for sfeed_update:

	newsboat -e | sfeed_opml_import > "$HOME/.sfeed/sfeedrc"

an example to export from an other RSS/Atom reader called rss2email (3.x+) and
import for sfeed_update:

	r2e opmlexport | sfeed_opml_import > "$HOME/.sfeed/sfeedrc"

Update feeds, this script merges the new items, see sfeed_update(1) for more
information what it can do:

	sfeed_update

Format feeds:

Plain-text list:

	sfeed_plain $HOME/.sfeed/feeds/* > "$HOME/.sfeed/feeds.txt"

HTML view (no frames), copy style.css for a default style:

	cp style.css "$HOME/.sfeed/style.css"
	sfeed_html $HOME/.sfeed/feeds/* > "$HOME/.sfeed/feeds.html"

HTML view with the menu as frames, copy style.css for a default style:

	mkdir -p "$HOME/.sfeed/frames"
	cd "$HOME/.sfeed/frames" && sfeed_frames $HOME/.sfeed/feeds/*

To automatically update your feeds periodically and format them in a way you
like you can make a wrapper script and add it as a cronjob.

Most protocols are supported because curl(1) is used by default and also proxy
settings from the environment (such as the $http_proxy environment variable)
are used.

The sfeed(1) program itself is just a parser that parses XML data from stdin
and is therefore network protocol-agnostic. It can be used with HTTP, HTTPS,
Gopher, SSH, etc.

See the section "Usage and examples" below and the man-pages for more
information how to use sfeed(1) and the additional tools.

A separate curses UI front-end called sfeed_curses is available at:
https://codemadness.org/sfeed_curses.html


Dependencies
------------

- C compiler (C99).
- libc (recommended: C99 and POSIX >= 200809).


Optional dependencies
---------------------

- POSIX make(1) for Makefile.
- POSIX sh(1),
  used by sfeed_update(1) and sfeed_opml_export(1).
- POSIX utilities such as awk(1) and sort(1),
  used by sfeed_update(1).
- curl(1) binary: https://curl.haxx.se/ ,
  used by sfeed_update(1), but can be replaced with any tool like wget(1),
  OpenBSD ftp(1) or hurl(1): https://git.codemadness.org/hurl/
- iconv(1) command-line utilities,
  used by sfeed_update(1). If the text in your RSS/Atom feeds are already UTF-8
  encoded then you don't need this. For a minimal iconv implementation:
  https://git.etalabs.net/cgit/noxcuse/tree/src/iconv.c
- mandoc for documentation: https://mdocml.bsd.lv/


OS tested
---------

- Linux (compilers: clang, cproc, gcc, pcc, tcc, libc: glibc, musl).
- OpenBSD (clang, gcc).
- NetBSD
- FreeBSD
- DragonFlyBSD
- Windows (cygwin gcc, mingw).
- HaikuOS (using libbsd).
- FreeDOS (djgpp).
- FUZIX (sdcc -mz80).


Architectures tested
--------------------

amd64, ARM, aarch64, HPPA, i386, SPARC64, Z80.


Files
-----

sfeed             - Read XML RSS or Atom feed data from stdin. Write feed data
                    in TAB-separated format to stdout.
sfeed_atom        - Format feed data (TSV) to an Atom feed.
sfeed_frames      - Format feed data (TSV) to HTML file(s) with frames.
sfeed_gopher      - Format feed data (TSV) to Gopher files.
sfeed_html        - Format feed data (TSV) to HTML.
sfeed_opml_export - Generate an OPML XML file from a sfeedrc config file.
sfeed_opml_import - Generate a sfeedrc config file from an OPML XML file.
sfeed_mbox        - Format feed data (TSV) to mbox.
sfeed_plain       - Format feed data (TSV) to a plain-text list.
sfeed_twtxt       - Format feed data (TSV) to a twtxt feed.
sfeed_update      - Update feeds and merge items.
sfeed_web         - Find urls to RSS/Atom feed from a webpage.
sfeed_xmlenc      - Detect character-set encoding from a XML stream.
sfeedrc.example   - Example config file. Can be copied to $HOME/.sfeed/sfeedrc.
style.css         - Example stylesheet to use with sfeed_html(1) and
                    sfeed_frames(1).


Files read at runtime by sfeed_update(1)
----------------------------------------

sfeedrc - Config file. This file is evaluated as a shellscript in
          sfeed_update(1).

Atleast the following functions can be overridden per feed:

- fetch: to use wget(1), OpenBSD ftp(1) or an other download program.
- filter: to filter on fields.
- merge: to change the merge logic.
- order: to change the sort order.

See also the sfeedrc(5) man page documentation for more details.

The feeds() function is called to process the feeds. The default feed()
function is executed concurrently as a background job in your sfeedrc(5) config
file to make updating faster. The variable maxjobs can be changed to limit or
increase the amount of concurrent jobs (8 by default).


Files written at runtime by sfeed_update(1)
-------------------------------------------

feedname     - TAB-separated format containing all items per feed. The
               sfeed_update(1) script merges new items with this file.
               The format is documented in sfeed(5).


File format
-----------

man 5 sfeed
man 5 sfeedrc
man 1 sfeed


Usage and examples
------------------

Find RSS/Atom feed urls from a webpage:

	url="https://codemadness.org"; curl -L -s "$url" | sfeed_web "$url"

output example:

	https://codemadness.org/blog/rss.xml	application/rss+xml
	https://codemadness.org/blog/atom.xml	application/atom+xml

- - -

Make sure your sfeedrc config file exists, see sfeedrc.example. To update your
feeds (configfile argument is optional):

	sfeed_update "configfile"

Format the feeds files:

	# Plain-text list.
	sfeed_plain $HOME/.sfeed/feeds/* > $HOME/.sfeed/feeds.txt
	# HTML view (no frames), copy style.css for a default style.
	sfeed_html $HOME/.sfeed/feeds/* > $HOME/.sfeed/feeds.html
	# HTML view with the menu as frames, copy style.css for a default style.
	mkdir -p somedir && cd somedir && sfeed_frames $HOME/.sfeed/feeds/*

View formatted output in your browser:

	$BROWSER "$HOME/.sfeed/feeds.html"

View formatted output in your editor:

	$EDITOR "$HOME/.sfeed/feeds.txt"

- - -

Example script to view feed items in a vertical list/menu in dmenu(1). It opens
the selected url in the browser set in $BROWSER:

	#!/bin/sh
	url=$(sfeed_plain "$HOME/.sfeed/feeds/"* | dmenu -l 35 -i | \
		sed -n 's@^.* \([a-zA-Z]*://\)\(.*\)$@\1\2@p')
	test -n "${url}" && $BROWSER "${url}"

dmenu can be found at: https://git.suckless.org/dmenu/

- - -

Generate a sfeedrc config file from your exported list of feeds in OPML
format:

	sfeed_opml_import < opmlfile.xml > $HOME/.sfeed/sfeedrc

- - -

Export an OPML file of your feeds from a sfeedrc config file (configfile
argument is optional):

	sfeed_opml_export configfile > myfeeds.opml

- - -

The filter function can be overridden in your sfeedrc file. This allows
filtering items per feed. It can be used to shorten urls, filter away
advertisements, strip tracking parameters and more.

	# filter fields.
	# filter(name)
	filter() {
		case "$1" in
		"tweakers")
			awk -F '\t' 'BEGIN { OFS = "\t"; }
			# skip ads.
			$2 ~ /^ADV:/ {
				next;
			}
			# shorten link.
			{
				if (match($3, /^https:\/\/tweakers\.net\/[a-z]+\/[0-9]+\//)) {
					$3 = substr($3, RSTART, RLENGTH);
				}
				print $0;
			}';;
		"yt BSDNow")
			# filter only BSD Now from channel.
			awk -F '\t' '$2 ~ / \| BSD Now/';;
		*)
			cat;;
		esac | \
			# replace youtube links with embed links.
			sed 's@www.youtube.com/watch?v=@www.youtube.com/embed/@g' | \

			awk -F '\t' 'BEGIN { OFS = "\t"; }
			function filterlink(s) {
				# protocol must start with http, https or gopher.
				if (match(s, /^(http|https|gopher):\/\//) == 0) {
					return "";
				}

				# shorten feedburner links.
				if (match(s, /^(http|https):\/\/[^\/]+\/~r\/.*\/~3\/[^\/]+\//)) {
					s = substr($3, RSTART, RLENGTH);
				}

				# strip tracking parameters
				# urchin, facebook, piwik, webtrekk and generic.
				gsub(/\?(ad|campaign|fbclid|pk|tm|utm|wt)_([^&]+)/, "?", s);
				gsub(/&(ad|campaign|fbclid|pk|tm|utm|wt)_([^&]+)/, "", s);

				gsub(/\?&/, "?", s);
				gsub(/[\?&]+$/, "", s);

				return s
			}
			{
				$3 = filterlink($3); # link
				$8 = filterlink($8); # enclosure

				# try to remove tracking pixels: <img/> tags with 1px width or height.
				gsub("<img[^>]*(width|height)[\s]*=[\s]*[\"'"'"' ]?1[\"'"'"' ]?[^0-9>]+[^>]*>", "", $4);

				print $0;
			}'
	}

- - -

The fetch function can be overridden in your sfeedrc file. This allows to
replace the default curl(1) for sfeed_update with any other client to fetch the
RSS/Atom data:

	# fetch a feed via HTTP/HTTPS etc.
	# fetch(name, url, feedfile)
	fetch() {
		hurl -m 1048576 -t 15 "$2" 2>/dev/null
	}

- - -

Aggregate feeds. This filters new entries (maximum one day old) and sorts them
by newest first. Prefix the feed name in the title. Convert the TSV output data
to an Atom XML feed (again):

	#!/bin/sh
	cd ~/.sfeed/feeds/ || exit 1

	awk -F '\t' -v "old=$(($(date +'%s') - 86400))" '
	BEGIN {	OFS = "\t"; }
	int($1) >= old {
		$2 = "[" FILENAME "] " $2;
		print $0;
	}' * | \
	sort -k1,1rn | \
	sfeed_atom

- - -

To have a "tail(1) -f"-like FIFO stream filtering for new unique feed items and
showing them as plain-text per line similar to sfeed_plain(1):

Create a FIFO:

	fifo="/tmp/sfeed_fifo"
	mkfifo "$fifo"

On the reading side:

	# This keeps track of unique lines so might consume much memory.
	# It tries to reopen the $fifo after 1 second if it fails.
	while :; do cat "$fifo" || sleep 1; done | awk '!x[$0]++'

On the writing side:

	feedsdir="$HOME/.sfeed/feeds/"
	cd "$feedsdir" || exit 1
	test -p "$fifo" || exit 1

	# 1 day is old news, don't write older items.
	awk -F '\t' -v "old=$(($(date +'%s') - 86400))" '
	BEGIN { OFS = "\t"; }
	int($1) >= old {
		$2 = "[" FILENAME "] " $2;
		print $0;
	}' * | sort -k1,1n | sfeed_plain | cut -b 3- > "$fifo"

cut -b is used to trim the "N " prefix of sfeed_plain(1).

- - -

For some podcast feed the following code can be used to filter the latest
enclosure url (probably some audio file):

	awk -F '\t' 'BEGIN { latest = 0; }
	length($8) {
		ts = int($1);
		if (ts > latest) {
			url = $8;
			latest = ts;
		}
	}
	END { if (length(url)) { print url; } }'

- - -

Over time your feeds file might become quite big. You can archive items of a
feed from (roughly) the last week by doing for example:

	awk -F '\t' -v "old=$(($(date +'%s') - 604800))" 'int($1) > old' < feed > feed.new
	mv feed feed.bak
	mv feed.new feed

This could also be run weekly in a crontab to archive the feeds. Like throwing
away old newspapers. It keeps the feeds list tidy and the formatted output
small.

- - -

Convert mbox to separate maildirs per feed and filter duplicate messages using the
fdm program.
fdm is available at: https://github.com/nicm/fdm

fdm config file (~/.sfeed/fdm.conf):

	set unmatched-mail keep

	account "sfeed" mbox "%[home]/.sfeed/mbox"
		$cachepath = "%[home]/.sfeed/fdm.cache"
		cache "${cachepath}"
		$maildir = "%[home]/feeds/"

		# Check if message is in in cache by Message-ID.
		match case "^Message-ID: (.*)" in headers
			action {
				tag "msgid" value "%1"
			}
			continue

		# If it is in the cache, stop.
		match matched and in-cache "${cachepath}" key "%[msgid]"
			action {
				keep
			}

		# Not in the cache, process it and add to cache.
		match case "^X-Feedname: (.*)" in headers
			action {
				# Store to local maildir.
				maildir "${maildir}%1"

				add-to-cache "${cachepath}" key "%[msgid]"
				keep
			}

Now run:

	$ sfeed_mbox ~/.sfeed/feeds/* > ~/.sfeed/mbox
	$ fdm -f ~/.sfeed/fdm.conf fetch

Now you can view feeds in mutt(1) for example.

- - -

Read from mbox and filter duplicate messages using the fdm program and deliver
it to a SMTP server. This works similar to the rss2email program.
fdm is available at: https://github.com/nicm/fdm

fdm config file (~/.sfeed/fdm.conf):

	set unmatched-mail keep

	account "sfeed" mbox "%[home]/.sfeed/mbox"
		$cachepath = "%[home]/.sfeed/fdm.cache"
		cache "${cachepath}"

		# Check if message is in in cache by Message-ID.
		match case "^Message-ID: (.*)" in headers
			action {
				tag "msgid" value "%1"
			}
			continue

		# If it is in the cache, stop.
		match matched and in-cache "${cachepath}" key "%[msgid]"
			action {
				keep
			}

		# Not in the cache, process it and add to cache.
		match case "^X-Feedname: (.*)" in headers
			action {
				# Connect to a SMTP server and attempt to deliver the
				# mail to it.
				# Of course change the server and e-mail below.
				smtp server "codemadness.org" to "hiltjo@codemadness.org"

				add-to-cache "${cachepath}" key "%[msgid]"
				keep
			}

Now run:

	$ sfeed_mbox ~/.sfeed/feeds/* > ~/.sfeed/mbox
	$ fdm -f ~/.sfeed/fdm.conf fetch

Now you can view feeds in mutt(1) for example.

- - -

Convert mbox to separate maildirs per feed and filter duplicate messages using
procmail(1).

procmail_maildirs.sh file:

	maildir="$HOME/feeds"
	feedsdir="$HOME/.sfeed/feeds"
	procmailconfig="$HOME/.sfeed/procmailrc"

	# message-id cache to prevent duplicates.
	mkdir -p "${maildir}/.cache"

	if ! test -r "${procmailconfig}"; then
		echo "Procmail configuration file \"${procmailconfig}\" does not exist or is not readable." >&2
		echo "See procmailrc.example for an example." >&2
		exit 1
	fi

	find "${feedsdir}" -type f -exec printf '%s\n' {} \; | while read -r d; do
		name=$(basename "${d}")
		mkdir -p "${maildir}/${name}/cur"
		mkdir -p "${maildir}/${name}/new"
		mkdir -p "${maildir}/${name}/tmp"
		printf 'Mailbox %s\n' "${name}"
		sfeed_mbox "${d}" | formail -s procmail "${procmailconfig}"
	done

Procmailrc(5) file:

	# Example for use with sfeed_mbox(1).
	# The header X-Feedname is used to split into separate maildirs. It is
	# assumed this name is sane.

	MAILDIR="$HOME/feeds/"

	:0
	* ^X-Feedname: \/.*
	{
		FEED="$MATCH"

		:0 Wh: "msgid_$FEED.lock"
		| formail -D 1024000 ".cache/msgid_$FEED.cache"

		:0
		"$FEED"/
	}

Now run:

	$ procmail_maildirs.sh

Now you can view feeds in mutt(1) for example.

- - -

Incremental data updates using If-Modified-Since

For servers that support it some incremental updates and bandwidth-saving can
be done by using the "If-Modified-Since" HTTP header.

The curl -z option can be used to send the modification date of the local feed
file so the server can make the decision to respond with incremental data.

You can do this by overriding the fetch() function in the sfeedrc file and
adding the -z option:

	# fetch(name, url, feedfile)
	fetch() {
		curl -z "$3" "$2"
	}

This comes at a cost of some privacy. For example there can be a fingerprinting
vector of the local modification date of the feed file.

- - -

Incremental data updates using ETag caching

For servers that support it some incremental updates and bandwidth-saving can
be done by using the "ETag" HTTP header.

Create a directory for storing the ETags per feed:

	mkdir -p ~/.sfeed/etags/

The curl ETag options can be used to send the previous ETag header value.  You
can do this by overriding the fetch() function in the sfeedrc file and adding
the --etag-save and --etag-compare options:

	# fetch(name, url, feedfile)
	fetch() {
		etag="$HOME/.sfeed/etags/$(basename "$3")"
		curl --etag-save "${etag}" --etag-compare "${etag}" "$2"
	}

This comes at a cost of some privacy. For example there can be a unique
generated ETag.

- - -

CDN's blocking requests due to missing User-Agent

sfeed_update will not send the "User-Agent" header by default for privacy
reasons.  Some CDNs like Cloudflare don't like this and will block such HTTP
requests.

A custom User-Agent can be set by using the curl -H option, like so:

	curl -H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:78.0) Gecko/20100101 Firefox/78.0'

The above example string pretends to be a Windows 10 (x86-64) machine running
Firefox 78.

- - -

Shellscript to export existing newsboat cached items from sqlite3 to the sfeed
TSV format.
	
	#!/bin/sh
	# Export newsbeuter/newsboat cached items from sqlite3 to the sfeed TSV format.
	# The data is split per file per feed with the name of the newsboat title/url.
	# Dependencies: sqlite3, awk.
	#
	# Usage: create some directory to store the feeds, run this script.
	#
	# Assumes "html" for content-type (Newsboat only handles HTML content).
	# Assumes feednames are unique and a feed title is set.
	
	# newsboat cache.db file.
	cachefile="$HOME/.newsboat/cache.db"
	test -n "$1" && cachefile="$1"
	
	# dump data.
	# .mode ascii: Columns/rows delimited by 0x1F and 0x1E
	# get the first fields in the order of the sfeed(5) format.
	sqlite3 "$cachefile" <<!EOF |
	.headers off
	.mode ascii
	.output
	SELECT
		i.pubDate, i.title, i.url, i.content, i.guid, i.author,
		i.enclosure_url,
		f.rssurl AS rssurl, f.title AS feedtitle --,
		-- i.id, i.unread, i.enclosure_type, i.enqueued, i.flags, i.deleted,
		-- i.base
	FROM rss_feed f
	INNER JOIN rss_item i ON i.feedurl = f.rssurl
	ORDER BY
		i.feedurl ASC, i.pubDate DESC;
	.quit
	!EOF
	# convert to sfeed(5) TSV format.
	awk '
	BEGIN {
		FS = "\x1f";
		RS = "\x1e";
	}
	# strip all control-chars for normal fields.
	function strip(s) {
		gsub("[[:cntrl:]]", "", s);
		return s;
	}
	# escape chars in content field.
	function escape(s) {
		gsub("\\\\", "\\\\", s);
		gsub("\n", "\\n", s);
		gsub("\t", "\\t", s);
		return s;
	}
	function feedname(url, title) {
		gsub("/", "_", title);
		return title;
	}
	{
		fname = feedname($8, $9);
		if (!feed[fname]++) {
			print "Writing file: \"" fname "\" (title: " $9 ", url: " $8 ")" > "/dev/stderr";
		}
	
		print $1 "\t" strip($2) "\t" strip($3) "\t" escape($4) "\t" \
			"html" "\t" strip($5) "\t" strip($6) "\t" strip($7) \
			> fname;
	}'


License
-------

ISC, see LICENSE file.


Author
------

Hiltjo Posthuma <hiltjo@codemadness.org>
