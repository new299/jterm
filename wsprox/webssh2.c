/*
 * Sample showing how to do SSH2 connect.
 *
 * The sample code has default values for host name, user name, password
 * and path to copy, but you can specify them on the command line like:
 *
 * "ssh2 host user password [-p|-i|-k]"
 */

#include "libssh2_config.h"
#include "../libssh2/src/libssh2_priv.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>


const char *keyfile1="~/.ssh/id_rsa.pub";
const char *keyfile2="~/.ssh/id_rsa";
const char *username="new";
const char *password="1";


static void kbd_callback(const char *name, int name_len,
                         const char *instruction, int instruction_len,
                         int num_prompts,
                         const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                         LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                         void **abstract)
{
    (void)name;
    (void)name_len;
    (void)instruction;
    (void)instruction_len;
    if (num_prompts == 1) {
        responses[0].text = strdup(password);
        responses[0].length = strlen(password);
    }
    (void)prompts;
    (void)abstract;
} /* kbd_callback */
    

unsigned long hostaddr;
int rc, sock, i, auth_pw = 0;
const char *fingerprint;
char *userauthlist;
LIBSSH2_SESSION *session;
LIBSSH2_CHANNEL *channel;

//PHASE1
int webssh2_connect() {
  fprintf(stderr,"webssh2_connect\n");
  struct sockaddr_in sin;


  hostaddr = htonl(0x7F000001);

  rc = libssh2_init (0);
  if(rc != 0) {
    fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
    return 1;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(22);
  sin.sin_addr.s_addr = hostaddr;
  int r = connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in));
  session = libssh2_session_init();
  libssh2_trace(session,LIBSSH2_TRACE_SOCKET | LIBSSH2_TRACE_CONN | LIBSSH2_TRACE_TRANS | LIBSSH2_TRACE_ERROR);
  libssh2_session_set_blocking(session,0);
  fprintf(stderr,"session id: %d",session);
  return r;
}

//PHASE2, return of 0==fail
int webssh2_handshake() {
    /* Create a session instance and start it up. This will trade welcome
     * banners, exchange keys, and setup crypto, compression, and MAC layers
     */
    return libssh2_session_handshake(session, sock);
}

//PHASE3
int webssh2_fingerprint() {

    /* At this point we havn't authenticated. The first thing to do is check
     * the hostkey's fingerprint against our known hosts Your app may have it
     * hard coded, may go to a file, may present it to the user, that's your
     * call
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(stderr, "Fingerprint: ");
    for(i = 0; i < 20; i++) {
        fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(stderr, "\n");
    return 0;
}

void debug(char *str) {
    char buf[2048];
    strcpy(buf,"console.log('");
    strcpy(buf+strlen(buf),str);
    strcpy(buf+strlen(buf),"');");
    emscripten_run_script(buf);
}

//PHASE4
int webssh2_authcheck() {

    /* check what authentication methods are available */
    userauthlist = libssh2_userauth_list(session, username, strlen(username));
    //rc = _libssh2_wait_socket(sess, entry_time);

    //fprintf(stderr, "Session state auth: %d",(session->userauth_list_state));
    char buf[1024];
    sprintf(buf,"Session errno: %d",libssh2_session_last_errno(session));
    debug(buf);

    //if(libssh2_session_last_errno(session) == LIBSSH2_ERROR_EAGAIN) return 0;
    if(userauthlist != NULL) {
      fprintf(stderr, "Authentication methods: %s\n", userauthlist);
      return 1;
    } else return 0;

}

int webssh2_authenticate() {
        /* We could authenticate via password */
        if (libssh2_userauth_password(session, username, password)) {
            fprintf(stderr, "\tAuthentication by password failed!\n");
            //goto shutdown;
            return 0;
        } else {
            fprintf(stderr, "\tAuthentication by password succeeded.\n");
            return 1;
        }
}

int webssh2_negotiate() {

    if (strstr(userauthlist, "password") != NULL) {
        auth_pw |= 1;
    }
    if (strstr(userauthlist, "keyboard-interactive") != NULL) {
        auth_pw |= 2;
    }
    if (strstr(userauthlist, "publickey") != NULL) {
        auth_pw |= 4;
    }

    /* if we got an 4. argument we set this option if supported */
    if (auth_pw & 1) {
        /* We could authenticate via password */
        if (libssh2_userauth_password(session, username, password)) {
            fprintf(stderr, "\tAuthentication by password failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr, "\tAuthentication by password succeeded.\n");
        }
    } else if (auth_pw & 2) {
        /* Or via keyboard-interactive */
        if (libssh2_userauth_keyboard_interactive(session, username,
                                                  &kbd_callback) ) {
            fprintf(stderr,
                "\tAuthentication by keyboard-interactive failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr,
                "\tAuthentication by keyboard-interactive succeeded.\n");
        }
    } else if (auth_pw & 4) {
        /* Or by public key */
        if (libssh2_userauth_publickey_fromfile(session, username, keyfile1,
                                                keyfile2, password)) {
            fprintf(stderr, "\tAuthentication by public key failed!\n");
            goto shutdown;
        } else {
            fprintf(stderr, "\tAuthentication by public key succeeded.\n");
        }
    } else {
        fprintf(stderr, "No supported authentication methods found!\n");
        goto shutdown;
    }

    /* Request a shell */
    if (!(channel = libssh2_channel_open_session(session))) {
        fprintf(stderr, "Unable to open a session\n");
        goto shutdown;
    }

    /* Some environment variables may be set,
     * It's up to the server which ones it'll allow though
     */
    libssh2_channel_setenv(channel, "FOO", "bar");

    /* Request a terminal with 'vanilla' terminal emulation
     * See /etc/termcap for more options
     */
    if (libssh2_channel_request_pty(channel, "vanilla")) {
        fprintf(stderr, "Failed requesting pty\n");
        goto skip_shell;
    }

    /* Open a SHELL on that pty */
    if (libssh2_channel_shell(channel)) {
        fprintf(stderr, "Unable to request shell on allocated pty\n");
        goto shutdown;
    }

    /* At this point the shell can be interacted with using
     * libssh2_channel_read()
     * libssh2_channel_read_stderr()
     * libssh2_channel_write()
     * libssh2_channel_write_stderr()
     *
     * Blocking mode may be (en|dis)abled with: libssh2_channel_set_blocking()
     * If the server send EOF, libssh2_channel_eof() will return non-0
     * To send EOF to the server use: libssh2_channel_send_eof()
     * A channel can be closed with: libssh2_channel_close()
     * A channel can be freed with: libssh2_channel_free()
     */

  skip_shell:
    if (channel) {
        libssh2_channel_free(channel);
        channel = NULL;
    }

    /* Other channel types are supported via:
     * libssh2_scp_send()
     * libssh2_scp_recv()
     * libssh2_channel_direct_tcpip()
     */

  shutdown:

    libssh2_session_disconnect(session,
                               "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done!\n");

    libssh2_exit();

    return 0;
}
