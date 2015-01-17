
webssh: webssh2.c emsocket.c
	emcc webssh2.c emsocket.c -I. -I../libssh2/src -I../libssh2/include -I../openssl-1.0.1j/include -lssh2 -lssl -lcrypto  -L../openssl-1.0.1j -L../libssh2/src/.libs -static -o webssh2.js -s EXPORTED_FUNCTIONS="['_webssh2_connect','_webssh2_handshake','_webssh2_fingerprint','_webssh2_authcheck','_webssh2_authenticate','_webssh2_requestshell','_webssh2_setenv','_webssh2_setterm','_webssh2_getshell','_webssh2_read','_webssh2_write','_webssh2_setuserpass']"
	emcc webvterm.c -I../libvterm/include -L../libvterm -lvterm -o webvterm.js -s EXPORTED_FUNCTIONS="['_webvterm_recv','_webvterm_initialisation','_webvterm_get_row']"
