#include <stdlib.h>

#include "puttyparser.hh"
#include "putty/termcapparser.hh"

__BEGIN_DECLS

static int send_raw_mouse;

void fatal_message_box(void *window, char *msg)
{
  fprintf(stderr, "%s\n", msg);
}

void fatalbox(char *p, ...)
{
  va_list ap;
  char *msg;

  va_start(ap, p);
  msg = dupvprintf(p, ap);
  va_end(ap);

  fatal_message_box(NULL, msg);
  sfree(msg);
}

/* Dummy routine, only required in plink. */

void ldisc_update(void *frontend, int echo, int edit)
{
}

int from_backend(void *frontend, int is_stderr, const char *data, int len)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  return term_data(inst->term, is_stderr, data, len);
}

/*
 * Ask whether to wipe a session log file before writing to it.
 * Returns 2 for wipe, 1 for append, 0 for cancel (don't log).
 */
int askappend(void *frontend, Filename filename,
              void (*callback)(void *ctx, int result), void *ctx)
{
  return 2;
}

void logevent(void *frontend, const char *string)
{
}

/*
 * Minimise or restore the window in response to a server-side
 * request.
 */
void set_iconic(void *frontend, int iconic)
{
}

/*
 * Move the window in response to a server-side request.
 */
void move_window(void *frontend, int x, int y)
{
}

/*
 * Move the window to the top or bottom of the z-order in response
 * to a server-side request.
 */
void set_zorder(void *frontend, int top)
{
}

/*
 * Refresh the window in response to a server-side request.
 */
void refresh_window(void *frontend)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  term_invalidate(inst->term);
}

/*
 * Maximise or restore the window in response to a server-side
 * request.
 */
void set_zoomed(void *frontend, int zoomed)
{
}

/*
 * Report whether the window is iconic, for terminal reports.
 */
int is_iconic(void *frontend)
{
  return 0;
}

/*
 * Report the window's position, for terminal reports.
 */
void get_window_pos(void *frontend, int *x, int *y)
{
  *x = *y = 0;
}

/*
 * Report the window's pixel size, for terminal reports.
 */
void get_window_pixels(void *frontend, int *x, int *y)
{
  *x = *y = 0;
}

/*
 * Return the window or icon title.
 */
char *get_window_title(void *frontend, int icon)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  return icon ? inst->icontitle : inst->wintitle;
}

static void update_mouseptr(struct gui_data *inst)
{
}

void frontend_keypress(void *handle)
{
}

void timer_change_notify(long next)
{
}

/*
 * set or clear the "raw mouse message" mode
 */
void set_raw_mouse_mode(void *frontend, int activate)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  activate = activate && !inst->cfg.no_mouse_rep;
  send_raw_mouse = activate;
  update_mouseptr(inst);
}

void request_resize(void *frontend, int w, int h)
{
}

void palette_set(void *frontend, int n, int r, int g, int b)
{
  struct gui_data *inst = (struct gui_data *)frontend;

  if (static_cast<size_t>(n) >= inst->parser->get_palette().colors.size())
    return;

  inst->parser->get_palette().colors[n] = Putty::RgbColor{
                                          static_cast<uint8_t>(r),
                                          static_cast<uint8_t>(g),
                                          static_cast<uint8_t>(b)};
}

void palette_reset(void *frontend)
{
  struct gui_data *inst = (struct gui_data *)frontend;

  inst->parser->get_palette() = Putty::Palette{};
}

void write_clip(void *frontend, wchar_t * data, int *attr, int len, int must_deselect)
{
}

void request_paste(void *frontend)
{
}

void get_clip(void *frontend, wchar_t ** p, int *len)
{
  struct gui_data *inst = (struct gui_data *)frontend;

  if (p)
    {
      *p = inst->pastein_data;
      *len = inst->pastein_data_len;
    }
}

void set_title(void *frontend, char *title)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  strncpy(inst->wintitle, title, lenof(inst->wintitle));
  inst->wintitle[lenof(inst->wintitle)-1] = '\0';
}

void set_icon(void *frontend, char *title)
{
  struct gui_data *inst = (struct gui_data *)frontend;
  strncpy(inst->icontitle, title, lenof(inst->icontitle));
  inst->icontitle[lenof(inst->icontitle)-1] = '\0';
}

void set_sbar(void *frontend, int total, int start, int page)
{
}

void sys_cursor(void *frontend, int x, int y)
{
}

/*
 * This is still called when mode==BELL_VISUAL, even though the
 * visual bell is handled entirely within terminal.c, because we
 * may want to perform additional actions on any kind of bell (for
 * example, taskbar flashing in Windows).
 */
void do_beep(void *frontend, int mode)
{
}

int char_width(Context ctx, int uc)
{
  return 1;
}

Context get_ctx(void *frontend)
{
  return frontend;
}

void free_ctx(Context ctx)
{
}

void do_text(Context ctx, int x, int y, wchar_t *text, int len,
             unsigned long attr, int lattr)
{
  struct gui_data *inst = (struct gui_data *)ctx;

  std::wstring str(text, len);
  inst->parser->update_display(x, y, str, attr, lattr);
}

void do_cursor(Context ctx, int x, int y, wchar_t *text, int len,
               unsigned long attr, int lattr)
{
}

void modalfatalbox(char *p, ...)
{
  va_list ap;
  fprintf(stderr, "FATAL ERROR: ");

  va_start(ap, p);
  vfprintf(stderr, p, ap);
  va_end(ap);

  fputc('\n', stderr);

  exit(1);
}

/*
 * dummies from uxprint.c
 */

printer_job *printer_start_job(char *printer)
{
  return NULL;
}

void printer_job_data(printer_job *pj, void *data, int len)
{
}

void printer_finish_job(printer_job *pj)
{
}

/*
 * dummies from logging.c
 */

/*
 * Flush any open log file.
 */
void logflush(void *handle)
{
}

/*
 * Log session traffic.
 */
void logtraffic(void *handle, unsigned char c, int logmode)
{
}

/*
 * Get the number of lines in the scrollback.
 */
int sblines(Terminal *term)
{
  int sblines = count234(term->scrollback);

  if (term->cfg.erase_to_scrollback && term->alt_which && term->alt_screen)
    {
      sblines += term->alt_sblines;
    }
  return sblines;
}

__END_DECLS
