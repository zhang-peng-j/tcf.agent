/*******************************************************************************
 * Copyright (c) 2007, 2008 Wind River Systems, Inc. and others.
 * All rights reserved. This program and the accompanying materials 
 * are made available under the terms of the Eclipse Public License v1.0 
 * which accompanies this distribution, and is available at 
 * http://www.eclipse.org/legal/epl-v10.html 
 *  
 * Contributors:
 *     Wind River Systems - initial API and implementation
 *******************************************************************************/

/*
 * Agent main module.
 */

#include "mdep.h"
#define CONFIG_MAIN
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include "asyncreq.h"
#include "events.h"
#include "trace.h"
#include "myalloc.h"
#include "channel.h"
#include "protocol.h"
#include "discovery.h"
#include "errors.h"

static char * progname;
static char * dest_url = DEFAULT_DISCOVERY_URL;

static void channel_server_connecting(Channel * c1) {
    PeerServer * ps;
    Channel * c2;
    Protocol * p1;
    Protocol * p2;

    trace(LOG_ALWAYS, "channel server connecting");

    ps = channel_peer_from_url(dest_url);
    if (ps == NULL) {
        trace(LOG_ALWAYS, "cannot parse peer url: %s", dest_url);
        channel_close(c1);
        return;
    }
    c2 = channel_connect(ps);
    peer_server_free(ps);
    if (c2 == NULL) {
        fprintf(stderr, "cannot connect to peer: %s\n", dest_url);
        channel_close(c1);
        return;
    }
    proxy_create(c1, c2);
    channel_start(c2);
}

static void channel_new_connection(ChannelServer * serv, Channel * c) {
    c->connecting = channel_server_connecting;
    channel_start(c);
}

#if defined(_WRS_KERNEL)
int tcf_log(void) {
#else   
int main(int argc, char ** argv) {
#endif
    int c;
    int ind;
    char * s;
    char * log_name = "-";
    char * url = "TCP:";
    PeerServer * ps;
    ChannelServer * serv;

#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ini_mdep();
    ini_trace();
    ini_asyncreq();
    ini_events_queue();

    log_mode = LOG_TCFLOG;

#if defined(_WRS_KERNEL)
    
    progname = "tcf";
    open_log_file("-");
    
#else
    
    progname = argv[0];

    /* Parse arguments */
    for (ind = 1; ind < argc; ind++) {
        s = argv[ind];
        if (*s != '-') {
            break;
        }
        s++;
        while ((c = *s++) != '\0') {
            switch (c) {
            case 'l':
            case 'L':
            case 's':
                if (*s == '\0') {
                    if (++ind >= argc) {
                        fprintf(stderr, "%s: error: no argument given to option '%c'\n", progname, c);
                        exit(1);
                    }
                    s = argv[ind];
                }
                switch (c) {
                case 'l':
                    log_mode = strtol(s, 0, 0);
                    break;

                case 'L':
                    log_name = s;
                    break;

                case 's':
                    url = s;
                    break;

                default:
                    fprintf(stderr, "%s: error: illegal option '%c'\n", progname, c);
                    exit(1);
                }
                s = "";
                break;

            default:
                fprintf(stderr, "%s: error: illegal option '%c'\n", progname, c);
                exit(1);
            }
        }
    }
    open_log_file(log_name);
    if (ind < argc) {
        dest_url = argv[ind++];
    }

#endif

    ps = channel_peer_from_url(url);
    if (ps == NULL) {
        fprintf(stderr, "invalid server URL (-s option value): %s\n", url);
        exit(1);
    }
    peer_server_addprop(ps, loc_strdup("Name"), loc_strdup("TCF Protocol Logger"));
    peer_server_addprop(ps, loc_strdup("Proxy"), loc_strdup(""));
    serv = channel_server(ps);
    if (serv == NULL) {
        fprintf(stderr, "cannot create TCF server\n");
        exit(1);
    }
    serv->new_conn = channel_new_connection;

    discovery_start(NULL); /* Client only */

    /* Process events - must run on the initial thread since ptrace()
     * returns ECHILD otherwise, thinking we are not the owner. */
    run_event_loop();
    return 0;
}
