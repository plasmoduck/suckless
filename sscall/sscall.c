/* See LICENSE file for copyright and license details */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <ao/ao.h>
#include <pthread.h>
#include <speex/speex_resampler.h>
#include <opus/opus.h>

#include "list.h"
#include "arg.h"

char *argv0;

/* Input/Output PCM buffer size */
#define FRAME_SIZE (320)
/* Input/Output compressed buffer size */
#define COMPRESSED_BUF_SIZE (1500)

/* Command line option, bits per sample */
static int fbits;
/* Command line option, samples per second (in a single channel) */
static int frate;
/* Command line option, number of channels */
static int fchan;
/* Command line option, device driver ID */
static int fdevid;
/* Command line option, verbosity flag */
static int fverbose;

/* Opus encoder state */
static OpusEncoder *opus_enc;
/* Opus decoder state */
static OpusDecoder *opus_dec;
/* TX/RX Speex resampler state */
static SpeexResamplerState *speex_resampler_tx;
static SpeexResamplerState *speex_resampler_rx;
/* Libao handle */
static ao_device *device;
/* Output PCM thread */
static pthread_t playback_thread;
/* Input PCM thread */
static pthread_t capture_thread;

/* Compressed header at the start
 * of each compressed packet */
struct compressed_header {
	/* Start of frame signature */
	uint32_t sig;
	uint32_t timestamp;
} __attribute__ ((packed));

/* Shared buf between enqueue_for_playback()
 * and playback thread */
struct compressed_buf {
	/* Compressed buffer */
	unsigned char *buf;
	/* Compressed buffer size */
	size_t len;
	struct list_head list;
	/* Pointer to the header of this buffer */
	struct compressed_header *hdr;
} compressed_buf;

/* Private structure for the
 * capture thread */
struct capture_priv {
	/* Input file descriptor */
	int fd;
	/* Client socket */
	int sockfd;
	/* Client address info */
	struct addrinfo *servinfo;
} capture_priv;

/* Lock that protects compressed_buf */
static pthread_mutex_t compressed_buf_lock;
/* Condition variable on which ao_play() blocks */
static pthread_cond_t tx_pcm_cond;

/* State of the playback thread */
struct playback_state {
	int quit;
} playback_state;

/* State of the capture thread */
struct capture_state {
	int quit;
} capture_state;

/* Lock that protects playback_state */
static pthread_mutex_t playback_state_lock;
/* Lock that protects capture_state */
static pthread_mutex_t capture_state_lock;

/* Set to 1 when SIGINT is received */
static volatile sig_atomic_t handle_sigint;

/* Play back audio from the client */
static void *
playback(void *data)
{
	struct compressed_buf *cbuf;
	struct list_head *iter, *q;
	struct playback_state *state = data;
	struct timespec ts;
	struct timeval tp;
	int rc;
	opus_int16 pcm[FRAME_SIZE];
	spx_int16_t *pcm_sample_convert;
	spx_uint32_t inlen;
	spx_uint32_t outlen;
	int ret;

	/* Prepare the resampler configuration */
	/* Input length is in frames */
	inlen = FRAME_SIZE;
	outlen = (FRAME_SIZE * 16000) / 8000;
	outlen *= frate;
	/* Output length is in bytes */
	outlen /= 16000;
	pcm_sample_convert = malloc(outlen);
	if (!pcm_sample_convert)
		err(1, "malloc");

	do {
		pthread_mutex_lock(&compressed_buf_lock);
		gettimeofday(&tp, NULL);
		/* Convert from timeval to timespec */
		ts.tv_sec = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec * 1000;
		/* Default to a 3 second wait internal */
		ts.tv_sec += 3;

		if (list_empty(&compressed_buf.list)) {
			/* Wait in the worst case 3 seconds to give some
			 * grace to perform cleanup if necessary */
			rc = pthread_cond_timedwait(&tx_pcm_cond,
						    &compressed_buf_lock,
						    &ts);
			if (rc == ETIMEDOUT)
				if (fverbose)
					printf("Output thread is starving...\n");
		}

		pthread_mutex_lock(&playback_state_lock);
		if (state->quit) {
			pthread_mutex_unlock(&playback_state_lock);
			pthread_mutex_unlock(&compressed_buf_lock);
			break;
		}
		pthread_mutex_unlock(&playback_state_lock);

		/* Dequeue, decode and play buffers via libao */
		list_for_each_safe(iter, q, &compressed_buf.list) {
			cbuf = list_entry(iter, struct compressed_buf,
					  list);

			/* Decode compressed buffer */
			ret = opus_decode(opus_dec, cbuf->buf, cbuf->len,
					  pcm, FRAME_SIZE, 0);
			if (ret < 0) {
				warnx("Failed to decode input packet: %d", ret);
				/* Play silence if the decode failed */
				memset(pcm_sample_convert, 0, outlen);
			} else {
				/* Sample convert the RX path */
				speex_resampler_process_int(speex_resampler_rx,
							    0, (void *)pcm, &inlen,
							    (void *)pcm_sample_convert,
							    &outlen);
				/* Outlen is in frames, convert to bytes */
				outlen *= 2;
			}

			/* Play via libao */
			ao_play(device, (void *)pcm_sample_convert,
				outlen);

			free(cbuf->buf);
			list_del(&cbuf->list);
			free(cbuf);
		}
		pthread_mutex_unlock(&compressed_buf_lock);
	} while (1);

	free(pcm_sample_convert);

	pthread_exit(NULL);

	return NULL;
}

static void
enqueue_for_playback(struct compressed_buf *cbuf)
{
	pthread_mutex_lock(&compressed_buf_lock);
	list_add_tail(&cbuf->list, &compressed_buf.list);
	pthread_cond_signal(&tx_pcm_cond);
	pthread_mutex_unlock(&compressed_buf_lock);
}

/* Parse the compressed packet and enqueue it for
 * playback */
static void
process_compressed_packet(const void *buf, size_t len)
{
	struct compressed_buf *cbuf;
	uint32_t sig;
	struct compressed_header *hdr;

	cbuf = malloc(sizeof(*cbuf));
	if (!cbuf)
		err(1, "malloc");
	memset(cbuf, 0, sizeof(*cbuf));

	cbuf->len = len - sizeof(*hdr);
	cbuf->buf = malloc(cbuf->len);
	if (!cbuf->buf)
		err(1, "malloc");

	memcpy(cbuf->buf, buf + sizeof(*hdr),
	       cbuf->len);

	hdr = (struct compressed_header *)buf;
	sig = ntohl(hdr->sig);
	if (sig != 0xcafebabe) {
		if (fverbose)
			warnx("Received corrupt packet: %lx\n",
			      (unsigned long)sig);
		free(cbuf->buf);
		free(cbuf);
		return;
	}
	cbuf->hdr = hdr;

	enqueue_for_playback(cbuf);
}

/* Input PCM thread, outbound path */
static void *
capture(void *data)
{
	struct capture_state *state = data;
	spx_int16_t *inbuf;
	unsigned char outbuf[COMPRESSED_BUF_SIZE];
	char inbuf_sample_convert[FRAME_SIZE];
	ssize_t inbytes, bytes;
	opus_int32 outbytes;
	spx_uint32_t inlen;
	spx_uint32_t outlen;
	ssize_t ret;
	int timestamp;
	struct compressed_header *hdr;
	opus_int32 max_data_bytes;

	/* Prepare Speex resampler configuration */
	outlen = FRAME_SIZE;
	inbytes = (FRAME_SIZE * frate) / 8000;
	inbytes *= frate;
	inbytes /= frate;
	inbuf = malloc(inbytes);
	if (!inbuf)
		err(1, "malloc");

	timestamp = 0;
	do {
		pthread_mutex_lock(&capture_state_lock);
		if (state->quit) {
			pthread_mutex_unlock(&capture_state_lock);
			break;
		}
		pthread_mutex_unlock(&capture_state_lock);

		bytes = read(capture_priv.fd, inbuf, inbytes);
		if (bytes > 0) {
			/* Input length should be in frames */
			inlen = bytes / 2;
			/* Sampler convert the TX path */
			speex_resampler_process_int(speex_resampler_tx,
						    0, (void *)inbuf, &inlen,
						    (void *)inbuf_sample_convert,
						    &outlen);

			/* Encode input buffer */
			max_data_bytes = sizeof(outbuf) - sizeof(*hdr);
			outbytes = opus_encode(opus_enc,
					       (const void *)inbuf_sample_convert,
					       FRAME_SIZE, outbuf + sizeof(*hdr),
					       max_data_bytes);
			if (outbytes < 0) {
				warnx("Failed to encode packet: %d", outbytes);
			} else if (outbytes == 1) {
				/* Don't need to transmit this one */
				continue;
			} else {
				/* Pre-append the header */
				hdr = (struct compressed_header *)outbuf;
				hdr->sig = htonl(0xcafebabe);
				hdr->timestamp = htonl(timestamp);
				timestamp += FRAME_SIZE;

				/* Send the buffer out */
				ret = sendto(capture_priv.sockfd, outbuf,
					     outbytes + sizeof(*hdr), 0,
					     capture_priv.servinfo->ai_addr,
					     capture_priv.servinfo->ai_addrlen);
				if (ret < 0)
					warn("sendto");
			}
		}
	} while (1);

	free(inbuf);

	pthread_exit(NULL);

	return NULL;
}

static void
usage(void)
{
	fprintf(stderr,
		"usage: %s [OPTIONS] <remote-addr> <remote-port> <local-port>\n", argv0);
	fprintf(stderr, " -b\tBits per sample\n");
	fprintf(stderr, " -r\tSamples per second (in a single channel)\n");
	fprintf(stderr, " -c\tNumber of channels\n");
	fprintf(stderr, " -d\tOverride default driver ID\n");
	fprintf(stderr, " -v\tEnable verbose output\n");
	fprintf(stderr, " -V\tPrint version information\n");
	fprintf(stderr, " -h\tThis help screen\n");
}

static void
sig_handler(int signum)
{
	switch (signum) {
	case SIGINT:
		handle_sigint = 1;
		break;
	case SIGUSR1:
		fverbose = !fverbose;
		break;
	default:
		break;
	}
}

static void
set_nonblocking(int fd)
{
	int opts;

	opts = fcntl(fd, F_GETFL);
	if (opts < 0)
		err(1, "fcntl");
	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0)
		err(1, "fcntl");
}

static void
init_ao(int rate, int bits, int chans,
	int *devid)
{
	ao_sample_format format;
	int default_driver;

	ao_initialize();

	default_driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = bits;
	format.channels = chans;
	format.rate = rate;
	format.byte_format = AO_FMT_LITTLE;

	if (!*devid)
		*devid = default_driver;

	device = ao_open_live(*devid, &format, NULL);
	if (!device)
		errx(1, "Error opening output device: %d\n",
		     fdevid);
}

static void
init_speexdsp(void)
{
	int tmp;

	/* Init Speex resampler */
	speex_resampler_tx = speex_resampler_init(fchan, frate,
						  16000,
						  SPEEX_RESAMPLER_QUALITY_DESKTOP,
						  &tmp);
	speex_resampler_rx = speex_resampler_init(fchan, 16000,
						  frate,
						  SPEEX_RESAMPLER_QUALITY_DESKTOP,
						  &tmp);
}

static void
init_opus(void)
{
	int error;

	opus_enc = opus_encoder_create(16000, fchan,
				       OPUS_APPLICATION_VOIP, &error);
	if (error != OPUS_OK) {
		errx(1, "Cannot create opus encoder: %s",
		     opus_strerror(error));
	}

	opus_dec = opus_decoder_create(16000, fchan, &error);
	if (error != OPUS_OK) {
		errx(1, "Cannot create opus decoder: %s",
		     opus_strerror(error));
	}
}

static void
deinit_ao(void)
{
	ao_close(device);
	ao_shutdown();
}

static void
deinit_speexdsp(void)
{
	speex_resampler_destroy(speex_resampler_tx);
	speex_resampler_destroy(speex_resampler_rx);
}

static void
deinit_opus(void)
{
	opus_encoder_destroy(opus_enc);
	opus_decoder_destroy(opus_dec);
}

int
main(int argc, char *argv[])
{
	int recfd = STDIN_FILENO;
	ssize_t bytes;
	char buf[COMPRESSED_BUF_SIZE];
	int cli_sockfd, srv_sockfd;
	struct addrinfo cli_hints, *cli_servinfo, *p0, *p1;
	struct addrinfo srv_hints, *srv_servinfo;
	int rv;
	int ret;
	socklen_t addr_len;
	struct sockaddr_storage their_addr;
	char host[NI_MAXHOST];
	int optval;

        ARGBEGIN {
        case 'h':
                usage();
                exit(0);
                break;
        case 'b':
                fbits = strtol(EARGF(usage()), NULL, 10);
                break;
        case 'c':
                fchan = strtol(EARGF(usage()), NULL, 10);
                break;
        case 'r':
                frate = strtol(EARGF(usage()), NULL, 10);
                break;
        case 'd':
                fdevid = strtol(EARGF(usage()), NULL, 10);
                break;
        case 'v':
                fverbose = 1;
                break;
        case 'V':
                printf("%s\n", VERSION);
                exit(0);
        case '?':
        default:
                exit(1);
        } ARGEND

	if (argc != 3) {
		usage();
		exit(1);
	}

	if (!fbits)
		fbits = 16;

	if (!fchan)
		fchan = 1;
	else if (fchan != 1)
		errx(1, "Unsupported number of channels: %d",
		     fchan);

	if (!frate)
		frate = 16000;

	init_ao(frate, fbits, fchan, &fdevid);
	init_speexdsp();
	init_opus();

	if (fverbose) {
		printf("Bits per sample: %d\n", fbits);
		printf("Number of channels: %d\n", fchan);
		printf("Sample rate: %d\n", frate);
		printf("Default driver ID: %d\n", fdevid);
		fflush(stdout);
	}

	memset(&cli_hints, 0, sizeof(cli_hints));
	cli_hints.ai_family = AF_INET;
	cli_hints.ai_socktype = SOCK_DGRAM;

	rv = getaddrinfo(argv[0], argv[1], &cli_hints, &cli_servinfo);
	if (rv)
		errx(1, "getaddrinfo: %s", gai_strerror(rv));

	for (p0 = cli_servinfo; p0; p0 = p0->ai_next) {
		cli_sockfd = socket(p0->ai_family, p0->ai_socktype,
				    p0->ai_protocol);
		if (cli_sockfd < 0)
			continue;
		break;
	}

	if (!p0)
		errx(1, "failed to bind socket");

	memset(&srv_hints, 0, sizeof(srv_hints));
	srv_hints.ai_family = AF_INET;
	srv_hints.ai_socktype = SOCK_DGRAM;
	srv_hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(NULL, argv[2], &srv_hints, &srv_servinfo);
	if (rv)
		errx(1, "getaddrinfo: %s", gai_strerror(rv));

	for(p1 = srv_servinfo; p1; p1 = p1->ai_next) {
		srv_sockfd = socket(p1->ai_family, p1->ai_socktype,
				    p1->ai_protocol);
		if (srv_sockfd < 0)
			continue;
		optval = 1;
		ret = setsockopt(srv_sockfd, SOL_SOCKET,
				 SO_REUSEADDR, &optval, sizeof(optval));
		if (ret < 0) {
			close(srv_sockfd);
			warn("setsockopt");
			continue;
		}
		if (bind(srv_sockfd, p1->ai_addr, p1->ai_addrlen) < 0) {
			close(srv_sockfd);
			warn("bind");
			continue;
		}
		break;
	}

	if (!p1)
		errx(1, "failed to bind socket");

	INIT_LIST_HEAD(&compressed_buf.list);

	pthread_mutex_init(&compressed_buf_lock, NULL);
	pthread_cond_init(&tx_pcm_cond, NULL);

	pthread_mutex_init(&playback_state_lock, NULL);
	pthread_mutex_init(&capture_state_lock, NULL);

	ret = pthread_create(&playback_thread, NULL,
			     playback, &playback_state);
	if (ret) {
		errno = ret;
		err(1, "pthread_create");
	}

	capture_priv.fd = recfd;
	capture_priv.sockfd = cli_sockfd;
	capture_priv.servinfo = p0;

	set_nonblocking(capture_priv.fd);
	set_nonblocking(capture_priv.sockfd);

	ret = pthread_create(&capture_thread, NULL,
			     capture, &capture_state);
	if (ret) {
		errno = ret;
		err(1, "pthread_create");
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		err(1, "signal");

	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		err(1, "signal");

	/* Main processing loop, receive compressed data,
	 * parse and prepare for playback */
	do {
		/* Handle SIGINT gracefully */
		if (handle_sigint) {
			if (fverbose)
				printf("Interrupted, exiting...\n");
			break;
		}

		addr_len = sizeof(their_addr);
		bytes = recvfrom(srv_sockfd, buf,
				 sizeof(buf), MSG_DONTWAIT,
				 (struct sockaddr *)&their_addr,
				 &addr_len);
		if (bytes > 0) {
			if (fverbose) {
				ret = getnameinfo((struct sockaddr *)&their_addr,
						  addr_len, host,
						  sizeof(host), NULL, 0, 0);
				if (ret < 0) {
					warn("getnameinfo");
					snprintf(host, sizeof(host), "unknown");
				}
				printf("Received %zd bytes from %s\n",
				       bytes, host);
			}
			process_compressed_packet(buf, bytes);
		}
	} while (1);

	/* Prepare input thread to be killed */
	pthread_mutex_lock(&capture_state_lock);
	capture_state.quit = 1;
	pthread_mutex_unlock(&capture_state_lock);

	/* Wait for it */
	pthread_join(capture_thread, NULL);

	/* Prepare output thread to be killed */
	pthread_mutex_lock(&playback_state_lock);
	playback_state.quit = 1;
	pthread_mutex_unlock(&playback_state_lock);

	/* Wake up the output thread if it is
	 * sleeping */
	pthread_mutex_lock(&compressed_buf_lock);
	pthread_cond_signal(&tx_pcm_cond);
	pthread_mutex_unlock(&compressed_buf_lock);

	/* Wait for it */
	pthread_join(playback_thread, NULL);

	deinit_opus();
	deinit_speexdsp();
	deinit_ao();

	freeaddrinfo(cli_servinfo);
	freeaddrinfo(srv_servinfo);

	return 0;
}
