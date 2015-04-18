/*************************************************************************
   client-udp.c
   
**************************************************************************

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT 
   HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
   OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR 
   CONTRIBUTORS ARE AWARE OF THE POSSIBILITY OF SUCH DAMAGE.

*************************************************************************/

#include "client-udp.h"

int client_udp( char *buffer, char *address, int port ) {
	struct sockaddr_in server;
	int  sock, length = sizeof( server );
	
	// Create socket
	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( sock == -1 ) {
		fprintf( stderr, "ERROR in client_udp: Could not create client socket\n" );
		return 1;
	}
	
	memset( (char *) &server, 0, length );
	server.sin_family = AF_INET;
	server.sin_port   = htons( port );
	
	if ( inet_aton( address, &server.sin_addr ) == 0 ) {
		fprintf( stderr, "ERROR in client_udp: Invalid server address %s, should be x.x.x.x\n", address );
		return 1;
	}
	
	// Send to remote server
	if ( sendto( sock, buffer, strlen( buffer ), 0, (const struct sockaddr *) &server, length ) < 0 ) {
		fprintf( stderr, "ERROR in client_udp: sendto \"%s\" failed\n", buffer );
		return 1;
	}
	
	close( sock );
	return 0;
}
