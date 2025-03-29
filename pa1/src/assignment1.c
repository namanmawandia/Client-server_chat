// /**
//  * @assignment1
//  * @author  Team Members <namanmaw@buffalo.edu><aveerama@buffalo.edu>
//  * @version 1.0
//  *
//  * @section LICENSE
//  *
//  * This program is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU General Public License as
//  * published by the Free Software Foundation; either version 2 of
//  * the License, or (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful, but
//  * WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//  * General Public License for more details at
//  * http://www.gnu.org/copyleft/gpl.html
//  *
//  * @section DESCRIPTION
//  *
//  * This contains the main function. Add further description here....
//  */


#define _POSIX_C_SOURCE 200112L

#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>    
#include <unistd.h>    
#include <netdb.h>    
#include <arpa/inet.h>

#include "../include/global.h"

#include "../include/logger.h"

#include "../include/client.h"

#include "../include/server.h"

#include "../include/common.h"


#define min(a, b)(((a) < (b)) ? (a) : (b))
#define MSIZE 500
#define MSIZEBACK 500 * 200
#define STDIN 0

//function declaration

void h_init(bool is_server, char * port);
void h_set_hname_ip(struct host * h);


// function definitions
/////////////////////////////////////////////////////////////////////

//  Host Initialization
void h_init(bool is_server, char * port) {
    localhost = malloc(sizeof(struct host));
    memcpy(localhost -> port_num, port, sizeof(localhost -> port_num));
    localhost -> is_server = is_server;
    h_set_hname_ip(localhost);
    if (is_server) {
        server__init();
    } else {
        client__init();
    }
}

void h_set_hname_ip(struct host * h) {
    char hostbuffer[MSIZE];
    char * IPbuffer;
    struct hostent * host_entry;
    int hostname;
    // get hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // retrieve host info
    host_entry = gethostbyname(hostbuffer);
    memcpy(h -> ip_addr, inet_ntoa( * ((struct in_addr * ) host_entry -> h_addr_list[0])), sizeof(h -> ip_addr));
    memcpy(h -> hostname, hostbuffer, sizeof(h -> hostname));
    return;
}


int main(int argc, char ** argv) {
    /*Init. Logger*/
    cse4589_init_log(argv[2]);

    /*Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

    /*Start Here*/
	//////////////////////////////////////////////////////////////////////////
    if (argc != 3) {
        exit(EXIT_FAILURE);
    }

    // initialise the host
    h_init(strcmp(argv[1], "s") == 0, argv[2]);

	//////////////////////////////////////////////////////////////////////////
    return 0;
}