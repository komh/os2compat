/*
 * getifaddrs() and freeifaddrs() test program
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * Modified for OS/2 kLIBC from:
 *   https://www.lemoda.net/freebsd/net-interfaces/index.html
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>
/* The following include files for the network interface. */
#include <sys/types.h>
#include <sys/socket.h>
#if 0
#include <ifaddrs.h>
#else
#include "getifaddrs.h"
#endif
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_types.h>
/* For "toupper". */
#include <ctype.h>
/* For "strerror". */
#include <string.h>
/* For "errno". */
#include <errno.h>

#include "test.h"

/* IPv6 stuffs missing in OS/2 kLIBC*/
#define INET6_ADDRSTRLEN    46

struct in6_addr
{
        uint8_t s6_addr[16];
};

struct sockaddr_in6
{
    uint8_t         sin6_len;
    uint8_t         sin6_family;
    uint16_t        sin6_port;
    uint32_t        sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t        sin6_scope_id;
};

/* Print an internet address. */

void
print_ip (const char * name, struct ifaddrs * ifaddrs_ptr, void * addr_ptr)
{
    if (addr_ptr) {

	/* This constant is defined in <netinet/in.h>. */

	char address[INET6_ADDRSTRLEN];

	inet_ntop (ifaddrs_ptr->ifa_addr->sa_family,
		   addr_ptr,
		   address, sizeof (address));

        /* The "toupper" capitalizes the first letter. */

	printf ("%c%s: <%s>\n", toupper (name[0]), name + 1, address);
    }
    else {
	printf ("No %s\n", name);
    }
}

/* Get a pointer to the address structure from a sockaddr. */

void *
get_addr_ptr (struct sockaddr * sockaddr_ptr)
{
    void * addr_ptr = 0;
    if (sockaddr_ptr->sa_family == AF_INET) {
        addr_ptr = &((struct sockaddr_in *)  sockaddr_ptr)->sin_addr;
    }
    else if (sockaddr_ptr->sa_family == AF_INET6) {
        addr_ptr = &((struct sockaddr_in6 *) sockaddr_ptr)->sin6_addr;
    }
    return addr_ptr;
}

/* Print the internet address. */

void
print_internet_address (struct ifaddrs * ifaddrs_ptr)
{
    void * addr_ptr;
    if (! ifaddrs_ptr->ifa_addr) {
	return;
    }
    addr_ptr = get_addr_ptr (ifaddrs_ptr->ifa_addr);
    print_ip ("internet address", ifaddrs_ptr, addr_ptr);
}

/* Print the netmask. */

void
print_netmask (struct ifaddrs * ifaddrs_ptr)
{
    void * addr_ptr;
    if (! ifaddrs_ptr->ifa_netmask) {
	return;
    }
    addr_ptr = get_addr_ptr (ifaddrs_ptr->ifa_netmask);
    print_ip ("netmask", ifaddrs_ptr, addr_ptr);
}

/* Print the mac address. */

void
print_mac_address (const char * mac_address)
{
    int mac_addr_offset;

    printf ("Mac address: ");
    for (mac_addr_offset = 0; mac_addr_offset < 6; mac_addr_offset++) {
        unsigned char byte;

        byte = (unsigned char) mac_address[mac_addr_offset];
	printf ("%02x", byte);
	if (mac_addr_offset != 5) {
	    printf (":");
        }
    }
    printf ("\n");
}

/* Adapted from http://othermark.livejournal.com/3005.html */

void
print_af_link (struct ifaddrs * ifaddrs_ptr)
{
    struct sockaddr_dl * sdl;

    sdl = (struct sockaddr_dl *) ifaddrs_ptr->ifa_addr;

    /* The types "IFT_ETHER" and so on are defined in
       <net/if_types.h>. */

    if (sdl->sdl_type == IFT_ETHER) {
	print_mac_address (LLADDR (sdl));
    }
    else if (sdl->sdl_type == IFT_LOOP) {
	printf ("Loopback.\n");
    }
    else if (sdl->sdl_type == IFT_USB) {
	printf ("USB interface.\n");
    }
    else {
	printf ("Link of type %d\n", sdl->sdl_type);
    }
}

void
print_internet_interface (struct ifaddrs * ifaddrs_ptr)
{
    print_internet_address (ifaddrs_ptr);
    print_netmask (ifaddrs_ptr);

    /* P2P interface destination */

    if (ifaddrs_ptr->ifa_dstaddr) {
	void * addr_ptr;

        addr_ptr = get_addr_ptr (ifaddrs_ptr->ifa_dstaddr);
	print_ip ("destination address", ifaddrs_ptr, addr_ptr);
    }

    /* Interface broadcast address */

    if (ifaddrs_ptr->ifa_broadaddr) {
	void * addr_ptr;

	addr_ptr = get_addr_ptr (ifaddrs_ptr->ifa_broadaddr);
	print_ip ("broadcast address", ifaddrs_ptr, addr_ptr);
    }
}

/* Adapted from
   http://publib.boulder.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/apis/getifaddrs.htm */


void
print_ifaddrs (struct ifaddrs * ifaddrs_ptr)
{
    while (ifaddrs_ptr) {
        printf ("Name: %s; Flags: 0x%x; ",
                ifaddrs_ptr->ifa_name, ifaddrs_ptr->ifa_flags);

        /* Decide what to do based on the family. */

        if (ifaddrs_ptr->ifa_addr->sa_family == AF_INET) {

            /* AF_INET is defined in <sys/socket.h>. */

            printf ("Family: AF_INET\n");
            print_internet_interface (ifaddrs_ptr);
        }
        else if (ifaddrs_ptr->ifa_addr->sa_family == AF_INET6) {
            printf ("Family: AF_INET6\n");
            print_internet_interface (ifaddrs_ptr);
        }
        else if (ifaddrs_ptr->ifa_addr->sa_family == AF_LINK) {
            printf ("Family: AF_LINK\n");
            print_af_link (ifaddrs_ptr);
        }
        else {
            printf ("Unknown family");
        }

        /* Print a line between this entry and the next. */

        printf ("\n");

        ifaddrs_ptr = ifaddrs_ptr->ifa_next;
    }
}

int main ()
{
    struct ifaddrs * ifaddrs_ptr;
    printf("Testing getifaddrs()...\n");
    TEST_NE(getifaddrs (& ifaddrs_ptr), -1 );
    printf("\n");
    print_ifaddrs (ifaddrs_ptr);
    /* Free the memory associated with "ifaddrs_ptr". */
    freeifaddrs (ifaddrs_ptr);
    printf("All tests PASSED\n");
    return 0;
}
