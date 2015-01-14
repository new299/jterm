
webssh: webssh2.c emsocket.c
	emcc webssh2.c emsocket.c -I. -I../libssh2/src -I../libssh2/include -I../openssl-1.0.1j/include -lssh2 -lssl -lcrypto  -L../openssl-1.0.1j -L../libssh2/src/.libs -static -o webssh2.js -s EXPORTED_FUNCTIONS="['_webssh2_connect','_webssh2_negotiate','_webssh2_handshake','_webssh2_fingerprint','_webssh2_authcheck']"

