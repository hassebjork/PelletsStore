/*************************************************************************
   config.c

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

#include "config.h"

/* Process a config file line with a single string var on it
 * Must be of the form 'StringVar=str' with no spaces or special characters before the '='.  
 * After the = whitespace is treated as part of the string. 
 */
int configStringVar( char *buf, char *matchStr, char *destStr ) {
	int len = strlen( matchStr );
	if ( strncmp( buf, matchStr, len ) == 0 ) {
		int i = len;
		int j = 0;

		if ( buf[i] != 0 ) /* skip '=' */
			i++;
		while ( ( ( buf[i] == ' ' ) || ( buf[i] =='\t' ) ) && ( i < READ_BUFSIZE ) )
			i++;
		while ( ( buf[i] != 0 ) && ( buf[i] != '\r' ) && ( buf[i] != '\n' ) 
			&& ( i < READ_BUFSIZE ) && ( j < ( MAX_TAG_SIZE - 1 ) ) )
			destStr[j++] = buf[i++];
		destStr[j] = 0;
		return 1;
	}
	return 0;
}

/* Process a config file line with a integer on it
 * Must be of the form 'numericVar=n' with no spaces or special characters. 
 */
int configIntVar( char *buf, char *matchStr, int *valp ) {
	int varCount, temp = 0, len = strlen( matchStr );
	if ( strncmp( buf, matchStr, len ) == 0 ) {
		varCount = sscanf( &buf[len+1], "%d", &temp );
		if ( varCount == 1 ) 
			*valp = temp;
		return 1;
	}
	return 0;   
}

/* Process a config file line with a float var  on it
 * Must be of the form 'floatVar=n.n' with no spaces or special characters. */
int configFloatVar( char *buf, char *matchStr, float *valp ) {
	int varCount, len = strlen( matchStr );
	float temp = 0.0;
	if ( strncmp( buf, matchStr, len ) == 0 ) {
		varCount = sscanf( &buf[len+1], "%f", &temp );
		if ( varCount == 1 ) 
			*valp = temp;
		return 1;
	}
	return 0;   
}
