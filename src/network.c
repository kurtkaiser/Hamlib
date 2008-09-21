/*
 *  Hamlib Interface - network communication low-level support
 *  Copyright (c) 2000-2008 by Stephane Fillod
 *
 *	$Id: network.c,v 1.1 2008-09-21 19:30:34 fillods Exp $
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/**
 * \addtogroup rig_internal
 * @{
 */

/**
 * \brief Network port IO
 * \file network.c
 */

#define WINVER 0x0501

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#include <netdb.h>	/* TODO */
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#elif HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif


#include "hamlib/rig.h"
#include "network.h"
#include "misc.h"

/**
 * \brief Open network port using rig.state data
 * \param rp port data structure (must spec port id eg hostname:port)
 * \return RIG_OK or < 0 if error
 */
int network_open(hamlib_port_t *rp) {

	int fd;				/* File descriptor for the port */
	int status;
	struct addrinfo hints, *res;
	char *portstr;
	char hostname[FILPATHLEN];

	if (!rp)
		return -RIG_EINVAL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	strcpy(hostname, rp->pathname);
	portstr = strchr(hostname, ':');
	if (!portstr)
		return -RIG_ECONF;
	*portstr++ = '\0';
    
	status=getaddrinfo(hostname, portstr, &hints, &res);
	if (status != 0) {
		rig_debug(RIG_DEBUG_ERR, "Cannot get host \"%s\": %s\n",
					rp->pathname, gai_strerror(errno));
		return -RIG_ECONF;
	}

	fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0)
		return -RIG_EIO;

	status = connect(fd, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	if (status < 0) {
		rig_debug(RIG_DEBUG_ERR, "Cannot open NET device \"%s\": %s\n",
					rp->pathname, strerror(errno));
		close(fd);
		return -RIG_EIO;
	}

	rp->fd = fd;

	return RIG_OK;
}

/** @} */
