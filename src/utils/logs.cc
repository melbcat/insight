/*-
 * Copyright (C) 2010-2014, Centre National de la Recherche Scientifique,
 *                          Institut Polytechnique de Bordeaux,
 *                          Universite de Bordeaux.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <set>
#include <cassert>
#include <list>
#include <cstdlib>

#include "logs.hh"

#ifdef NDEBUG
# define DEBUG_IS_ON false
#else
# define DEBUG_IS_ON true
#endif

using namespace std;

typedef std::multiset<logs::Listener *> listener_set;
typedef listener_set::iterator listener_iterator;
typedef void (*listener_callback)(const std::string &msg);

static listener_set LISTENERS;
static int debug_level = 0;
static list<string> debug_blocks;
bool logs::debug_is_on = DEBUG_IS_ON;

class StdStreamListener : public logs::Listener
{
private:
  int max_debug_level;
  int tabsize;
  ostream *out;
  bool enable_warnings;
public:
  StdStreamListener () : max_debug_level(-1), tabsize (0), out (&std::cout),
			 enable_warnings (true) { }

  ~StdStreamListener () { }

  void set_max_level (int level) {
    max_debug_level = level;
  }

  void set_tabsize (int tab) {
    tabsize = tab;
  }

  void set_out (ostream &o) {
    out = &o;
  }

  void set_enable_warnings (bool value) {
    enable_warnings = value;
  }

  void error (const std::string &msg) {
    cerr << msg << endl;
  }

  void warning (const std::string &msg) {
    if (enable_warnings)
      {
	cout << msg << endl;
	cout.flush ();
      }
  }

  void display (const std::string &msg) {
    cout << msg << endl;
    cout.flush ();
  }

  void debug (const std::string &msg, int level) {
    if (max_debug_level < 0 || level <= max_debug_level)
      {
	*out << string (level * tabsize, ' ') << msg << endl;
	out->flush ();
      }
  }
};

static StdStreamListener *STDLISTENER = NULL;


std::string logs::DEBUG_ENABLED_PROP = "logs.debug.enabled";
std::string logs::STDIO_ENABLED_PROP = "logs.stdio.enabled";
std::string logs::STDIO_ENABLE_WARNINGS_PROP = "logs.stdio.enable-warnings";
std::string logs::STDIO_DEBUG_IS_CERR_PROP = "logs.stdio.debug.is_cerr";
std::string logs::STDIO_DEBUG_MAXLEVEL_PROP = "logs.stdio.debug.maxlevel";
std::string logs::STDIO_DEBUG_TABSIZE_PROP = "logs.stdio.debug.tabsize";

void
logs::init (const ConfigTable &cfg)
{
  debug_is_on = cfg.get_boolean (DEBUG_ENABLED_PROP);

  if (cfg.get_boolean (STDIO_ENABLED_PROP))
    {
      STDLISTENER = new StdStreamListener ();
      if (cfg.get_boolean (STDIO_DEBUG_IS_CERR_PROP))
	STDLISTENER->set_out (cerr);

      STDLISTENER->set_enable_warnings (cfg.get_boolean (STDIO_ENABLE_WARNINGS_PROP));

      int maxlevel = cfg.get_integer (STDIO_DEBUG_MAXLEVEL_PROP, -1);
      STDLISTENER->set_max_level (maxlevel);

      int tabsize = cfg.get_integer (STDIO_DEBUG_TABSIZE_PROP, 2);
      STDLISTENER->set_tabsize (tabsize);

      logs::add_listener (STDLISTENER);
    }
}

void
logs::terminate ()
{
  if (STDLISTENER != NULL)
    delete STDLISTENER;
}

void
logs::add_listener (Listener *listener, bool once)
{
  assert (listener != NULL);
  if (LISTENERS.find (listener) != LISTENERS.end () && once)
    return;

  LISTENERS.insert (listener);
}

void
logs::remove_listener (Listener *listener)
{
  assert (listener != NULL);
  LISTENERS.erase (listener);
}


void
logs::inc_debug_level ()
{
  debug_level++;
}

void
logs::dec_debug_level ()
{
  assert (debug_level > 0);
  debug_level--;
}

void
logs::start_debug_block (const string &msg)
{
  debug_blocks.push_front (msg);
  logs::debug << msg << endl;
  inc_debug_level ();
}

void
logs::end_debug_block ()
{
  dec_debug_level ();
  string msg = debug_blocks.front ();
  debug_blocks.pop_front ();
  logs::debug << msg << " terminated " << endl;
}

static void
s_logs_error (const string &msg)
{
  for (listener_iterator i = LISTENERS.begin (); i != LISTENERS.end (); i++)
    (*i)->error (msg);
}

static void
s_logs_warning (const string &msg)
{
  for (listener_iterator i = LISTENERS.begin (); i != LISTENERS.end (); i++)
    (*i)->warning (msg);
}

static void
s_logs_display (const string &msg)
{
  for (listener_iterator i = LISTENERS.begin (); i != LISTENERS.end (); i++)
    (*i)->display (msg);
}

static void
s_logs_debug (const string &msg)
{
  if (! logs::debug_is_on)
    return;

  for (listener_iterator i = LISTENERS.begin (); i != LISTENERS.end (); i++)
    (*i)->debug (msg, debug_level);
}

/*
 *
 * OSTREAMS for logging
 *
 */
class filter : public std::streambuf
{
public:
  static const size_t BUFF_SIZE = 1024;

  filter (listener_callback cb)
    : streambuf (), out_buf (new char[BUFF_SIZE + 1]), line (), callback (cb)
  {

    this->setg (0, 0, 0);
    this->setp (out_buf, out_buf + BUFF_SIZE - 1);
  }

  ~filter()
  {
    delete [] out_buf;
  }

protected:
  virtual int_type overflow (int_type c) {
    char *ibegin = this->pbase ();
    char *iend = this->pptr();

    setp (out_buf, out_buf + BUFF_SIZE + 1);

    for (char *i = ibegin; i != iend; i++)
      {
	if (*i == '\n')
	  {
	    callback (line);
	    line = string ();
	  }
	else
	  line += *i;
      }
    if ( ! traits_type::eq_int_type (c, traits_type::eof ()))
      {
	if (traits_type::to_char_type(c) == '\n')
	  {
	    callback (line);
	    line = string ();
	  }
	else
	  {
	    line += traits_type::to_char_type(c);
	  }
      }

    return traits_type::not_eof(c);
  }

  virtual int_type sync() {
    return traits_type::eq_int_type (this->overflow (traits_type::eof ()),
				     traits_type::eof ()) ? -1 : 0;
  }

private:
  char *out_buf;
  string line;
  listener_callback callback;
};

std::ostream logs::error (new filter (&s_logs_error));
std::ostream logs::warning (new filter (&s_logs_warning));
std::ostream logs::display (new filter (&s_logs_display));
std::ostream logs::debug (new filter (&s_logs_debug));

void
logs::fatal_error (const std::string &msg)
{
  logs::error << msg << endl;
  abort ();
}

void
logs::check (const std::string &msg, bool cond)
{
  if (! cond)
    logs::fatal_error (msg);
}

const std::string logs::separator = string (80, '=');
