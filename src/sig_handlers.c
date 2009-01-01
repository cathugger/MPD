/* the Music Player Daemon (MPD)
 * Copyright (C) 2003-2007 by Warren Dukes (warren.dukes@gmail.com)
 * (c) 2004 Nick Welch (mack@incise.org)
 * This project's homepage is: http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sig_handlers.h"
#include "log.h"
#include "main.h"
#include "event_pipe.h"

#include <glib.h>

#include <sys/signal.h>
#include <errno.h>
#include <string.h>

static void exit_signal_handler(G_GNUC_UNUSED int signum)
{
	g_main_loop_quit(main_loop);
}

static void reload_signal_handler(G_GNUC_UNUSED int signum)
{
	event_pipe_emit_fast(PIPE_EVENT_RELOAD);
}

static void
x_sigaction(int signum, const struct sigaction *act)
{
	if (sigaction(signum, act, NULL) < 0)
		g_error("sigaction() failed: %s", strerror(errno));
}

static void
handle_reload_event(void)
{
	DEBUG("got SIGHUP, rereading DB\n");
	cycle_log_files();
}

void initSigHandlers(void)
{
	struct sigaction sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	while (sigaction(SIGPIPE, &sa, NULL) < 0 && errno == EINTR) ;

	sa.sa_handler = exit_signal_handler;
	x_sigaction(SIGINT, &sa);
	x_sigaction(SIGTERM, &sa);

	event_pipe_register(PIPE_EVENT_RELOAD, handle_reload_event);
	sa.sa_handler = reload_signal_handler;
	x_sigaction(SIGHUP, &sa);
}
