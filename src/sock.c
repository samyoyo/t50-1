/*
 *  T50 - Experimental Mixed Packet Injector
 *
 *  Copyright (C) 2010 - 2014 - T50 developers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

static socket_t fd;

/* Socket configuration */
void createSocket(void)
{
	socklen_t len;
	unsigned n = 1, *nptr = &n;

	/* Setting SOCKET RAW. */
	if( (fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1 )
	{
		perror("error opening raw socket");
		exit(EXIT_FAILURE);
	}

	/* Setting IP_HDRINCL. */
	if( setsockopt(fd, IPPROTO_IP, IP_HDRINCL, nptr, sizeof(n)) == -1 )
	{
		perror("error setting socket options");
		exit(EXIT_FAILURE);
	}

/* Taken from libdnet by Dug Song. */
#ifdef SO_SNDBUF
	len = sizeof(n);
	/* Getting SO_SNDBUF. */
	if ( getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &n, &len) == -1 )
	{
		perror("error getting socket buffer");
		exit(EXIT_FAILURE);
	}

	/* Setting the maximum SO_SNDBUF in bytes.
	 * 128      =  1 kilobit
	 * 10485760 = 10 megabytes */
	for (n += 128; n < 10485760; n += 128)
	{
		/* Setting SO_SNDBUF. */
		if ( setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &n, len) == -1 )
		{
			if(errno == ENOBUFS)	
				break;

			perror("error setting socket buffer");
			exit(EXIT_FAILURE);
		}
	}
#endif /* SO_SNDBUF */

#ifdef SO_BROADCAST
	/* Setting SO_BROADCAST. */
	if( setsockopt(fd, SOL_SOCKET, SO_BROADCAST, nptr, sizeof(n)) == -1 )
	{
		perror("error setting socket broadcast");
		exit(EXIT_FAILURE);
	}
#endif /* SO_BROADCAST */

#ifdef SO_PRIORITY
	if( setsockopt(fd, SOL_SOCKET, SO_PRIORITY, nptr, sizeof(n)) == -1 )
	{
		perror("error setting socket priority");
		exit(EXIT_FAILURE);
	}
#endif /* SO_PRIORITY */
}

void sendPacket(const void * const buffer, size_t size, const struct config_options * const co)
{
  struct sockaddr_in sin;

  assert(buffer != NULL);
  assert(size > 0);
  assert(co != NULL);

  sin.sin_family      = AF_INET; 
  sin.sin_port        = htons(IPPORT_RND(co->dest)); 
  sin.sin_addr.s_addr = co->ip.daddr; 

  if ((sendto(fd, 
              buffer, size, 
              MSG_NOSIGNAL, 
              (struct sockaddr *)&sin, 
              sizeof(struct sockaddr)) == -1) && 
      (errno != EPERM)) 
  {
    close(fd);
    ERROR("Error sending packet.");
    exit(EXIT_FAILURE);
  }
}
