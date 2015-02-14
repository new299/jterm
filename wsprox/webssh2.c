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
char username[1024];
char password[1024];

int webssh2_setuserpass(char *username_in,char *password_in) {
  strcpy(username,username_in);
  strcpy(password,password_in);

  size_t username_len = strlen(username_in);
  size_t password_len = strlen(password_in);
  int n=0;
  for(n=0;n<username_len;n++) username_in[n]=0;
  for(n=0;n<password_len;n++) password_in[n]=0;
  return 1;
}

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
  //fprintf(stderr,"webssh2_connect\n");
  struct sockaddr_in sin;


  hostaddr = htonl(0x7F000001);

  rc = libssh2_init (0);
  if(rc != 0) {
    //fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
    return 1;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(22);
  sin.sin_addr.s_addr = hostaddr;
  int r = connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in));
  session = libssh2_session_init();

  // uncomment following to enable libssh2 debug trace output
  // libssh2_trace(session,LIBSSH2_TRACE_SOCKET | LIBSSH2_TRACE_CONN | LIBSSH2_TRACE_TRANS | LIBSSH2_TRACE_ERROR);
  libssh2_session_set_blocking(session,0);
  //fprintf(stderr,"session id: %d",session);
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
    //fprintf(stderr, "Fingerprint: ");
    //TODO: add this to interface
    //for(i = 0; i < 20; i++) {
        //fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    //}
    //fprintf(stderr, "\n");
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

    if(userauthlist != NULL) {
      fprintf(stderr, "Authentication methods: %s\n", userauthlist);
      return 1;
    } else return 0;

}

int webssh2_authenticate() {
        /* We could authenticate via password */
        if (libssh2_userauth_password(session, username, password)) {
            fprintf(stderr, "\tAuthentication by password failed!\n");
            return 0;
        } else {
            fprintf(stderr, "\tAuthentication by password succeeded.\n");
            size_t username_len = strlen(username);
            size_t password_len = strlen(password);
            int n=0;
            for(n=0;n<username_len;n++) username[n]=0;
            for(n=0;n<password_len;n++) password[n]=0;
           
            return 1;
        }
}

int webssh2_requestshell() {
    /* Request a shell */
    if (!(channel = libssh2_channel_open_session(session))) {
        fprintf(stderr, "Unable to open a session\n");
        return 0;
    }
    return 1;
}

int webssh2_setenv() {
    /* Some environment variables may be set,
     * It's up to the server which ones it'll allow though
     */
    libssh2_channel_setenv(channel, "TERM", "xterm");
    return 1;
}

int webssh2_setterm() {

    /* Request a terminal with 'vanilla' terminal emulation
     * See /etc/termcap for more options
     */
    if (libssh2_channel_request_pty(channel, "xterm")) {
      fprintf(stderr, "Failed requesting pty\n");
    }
  return 1;
}

int webssh2_getshell() {

    /* Open a SHELL on that pty */
    if (libssh2_channel_shell(channel)) {
        fprintf(stderr, "Unable to request shell on allocated pty\n");
      return 0;
    }
    return 1;
}

int webssh2_read(char *buffer,int size) {
  buffer[0]=0;
  ssize_t res = libssh2_channel_read(channel,buffer,size); 
  //printf("read buffer %d %d: %s ***",size,res,buffer);
  return res;
}

int webssh2_write(char *buffer,int size) {
  ssize_t res = libssh2_channel_write(channel,buffer,size); 
  //printf("write buffer %d %d: %s ***",size,res,buffer);
  return res;
}

int webssh2_resize(int cols,int rows) {
  if(channel == 0) return 1;
  if(libssh2_channel_eof(channel)!=0) {return -1;}
  libssh2_channel_request_pty_size(channel,cols,rows);
}

int webssh2_channel_closed() {
  return libssh2_channel_eof(channel);
}
