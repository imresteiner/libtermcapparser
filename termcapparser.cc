#include "puttyparser.hh"

#include "terminal.h"

#include "putty/termcapparser.hh"
#include "state_p.hh"
#include <sstream>

#include <ostream>

using namespace Putty;

namespace
{

  static const int CONTROL_SEQUENCE_LENGTH = 8;
  static const char ESCAPE_CHAR = '\033';

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
  /**
   * Determine if a character is a unicode combining character.
   *
   * For information about combining characters, see:
   *
   *   http://en.wikipedia.org/wiki/Combining_character
   *
   * For the unicode character sheets, see:
   *
   *   Combining Diacritical Marks: http://www.unicode.org/charts/PDF/U0300.pdf
   *   Combining Diacritical Marks Supplement: http://www.unicode.org/charts/PDF/U1DC0.pdf
   *   Combining Diacritical Marks for Symbols: http://www.unicode.org/charts/PDF/U20D0.pdf
   *   Combining Half Marks: http://www.unicode.org/charts/PDF/UFE20.pdf
   *
   */
  bool is_combining_character(wchar_t chr)
  {
    if (likely(chr < 0x0300))
      return false;

    // Diacritical marks
    if (chr >= 0x0300 && chr <= 0x036F)
      return true;

    // Diacritical marks supplement - range 1
    if (chr >= 0x1DC0 && chr <= 0x1DE6)
      return true;

    // Diacritical marks supplement - range 2
    if (chr >= 0x1DFC && chr <= 0x1DFF)
      return true;

    // Diacritical marks for symbols
    if (chr >= 0x20D0 && chr <= 0x20F0)
      return true;

    // Half marks
    if (chr >= 0xFE20 && chr <= 0xFE26)
      return true;

    return false;
  }

  /**
   * Helper class for changing the value of a variable temporarily (just for the given scope)
   * in an exception-safe manner. When the destructor is called (when exiting the scope), the
   * value is re-set to its old value.
   */
  template < typename _Type >
    class TemporalValueChange
    {
    public:
      /**
       * Initialize helper object. The reference value is changed to the new value.
       *
       * @param ref Reference to variable to change.
       * @param value New value of the variable.
       */
      TemporalValueChange(_Type &ref, const _Type &value)
        : ref(ref),
          oldval(ref)
      {
        ref = value;
      }

      /**
       * Destroy the helper object. The referenced variable is set to its old value.
       */
      ~TemporalValueChange()
      {
        ref = oldval;
      }

    private:
      _Type &ref;     /**< Variable reference */
      _Type oldval;   /**< Old value of the variable (before change) */
    };


    inline void move_on(const char *&data, int &size, int offset)
    {
      data = data + offset;
      size -= offset;
    }

    inline bool is_control_sequence_partial(int data_length)
    {
      return data_length < CONTROL_SEQUENCE_LENGTH;
    }

    inline int get_first_control_sequence_pos(const char *data, int len)
    {
      int check_length = len - 1;
      for (int pos = 0; pos < check_length; ++pos)
        {
          if ((data[pos] == ESCAPE_CHAR) && (data[pos + 1] == 'P'))
            {
              return pos;
            }
        }
      return -1;
    }
}

TermcapParser::TermcapParser(const char *charset, int terminal_buffer_height)
  : terminal_buffer_height(terminal_buffer_height),
    log_callback(0)
{
  /* Create an instance structure and initialise to zeroes */
  inst = snew(struct gui_data);
  memset(inst, 0, sizeof(*inst));

  /* Set the termcap parser object */
  inst->parser = this;

  /* initialize unicode config */
  init_ucs(&inst->ucsdata, charset, 0, CS_UTF8, VT_UNICODE);

  /* Enable ANSI and XTerm 256 colors */
  inst->cfg.ansi_colour = 1;
  inst->cfg.xterm_256_colour = 1;

  /* Configure BiDirectional and Arabic text support, 0 means enable and 1 means disable */
  inst->cfg.bidi = 1;
  inst->cfg.arabicshaping = 1;

  /* DO NOT REMOVE! This config option ensures that an erase will be done with the proper bg color. */
  inst->cfg.bce = 1;

  /*
   * LF implies a CR-LF since in some cases (like ssh command execution) only LF is sent.
   * By default, we don't enable this but replace_standalone_linefeeds() can be used to
   * change this behaviour.
   */
  inst->cfg.lfhascr = 0;

  /*
   * Set autowrap long lines.
   */
  inst->cfg.wrap_mode = 1;

  /* initialize terminal structure */
  inst->term = term_init(&inst->cfg, &inst->ucsdata, inst);
  inst->term->ldisc = 0;

  /* Set log context */
  inst->logctx = NULL;
  term_provide_logctx(inst->term, inst->logctx);

  /* set terminal visual size: row number, column number, saveline option */
  set_terminal_size(80, 24);
}

TermcapParser::~TermcapParser()
{
  term_free(inst->term);
  sfree(inst);
}

void
TermcapParser::data_input(const char *data, int len)
{
  const char *data_to_process = data;
  int data_to_process_len = len;

  if (buffered_data_input.size() > 0)
    {
      buffered_data_input.append(data, len);
      data_to_process = buffered_data_input.c_str();
      data_to_process_len = buffered_data_input.size();
    }

  while (data_to_process_len > 0)
    {
      if (data_to_process_len == 1 && data_to_process[0] == ESCAPE_CHAR)
        {
          buffered_data_input.resize(1);
          buffered_data_input[0] = ESCAPE_CHAR;
          break;
        }

      int first_control_sequence_pos = get_first_control_sequence_pos(data_to_process, data_to_process_len);

      if (first_control_sequence_pos != -1)
        {
          data_input_filtered(data_to_process, first_control_sequence_pos);
          move_on(data_to_process, data_to_process_len, first_control_sequence_pos);

          if (is_control_sequence_partial(data_to_process_len))
            {
              buffered_data_input = std::string(data_to_process, data_to_process_len);
              break;
            }

          // Skip the control sequence
          move_on(data_to_process, data_to_process_len, CONTROL_SEQUENCE_LENGTH);
        }
      else // Leftover
        {
          data_input_filtered(data_to_process, data_to_process_len);

          data_to_process = 0;
          data_to_process_len = 0;
        }

      if (data_to_process_len <= 0)
        buffered_data_input.clear();
    }
}

void
TermcapParser::data_input_filtered(const char *data, int len)
{
  /* inject input in the terminal */
  term_data(inst->term, 0, data, len);
}

void
TermcapParser::set_cell(int row, int col, const std::wstring &characters, uint64_t attr) const
{
  bool success = ::set_cell(state, row, col, characters, attr);
  if (!success)
    {
      std::ostringstream stream;
      stream << "Invalid position requested to be updated; row='" << row << "', " << "col='" << col << "'";
      log_message(stream.str());
    }
}

void
TermcapParser::set_buffer_size(int width, int height)
{
  inst->width = width;
  inst->height = height;
}

void
TermcapParser::set_terminal_size(int width, int height)
{
  resize_display(state, width, height);

  int max_height = std::max(terminal_buffer_height, height);
  term_size(inst->term, height, width, max_height - height);
  set_buffer_size(width, max_height);
}

void
TermcapParser::copy_term_content_to_cache(int offset, int row_count) const
{
  term_scroll(inst->term, -1, offset);

  std::wstring characters;
  int absolute_offset = 0;
  for (int row = 0; row < row_count; ++row)
    {
      if (!is_valid_row(state, offset + row))
        {
          std::ostringstream stream;
          stream << "Invalid row requested to be updated; row='" << row << "', offset='" << offset << "'";
          log_message(stream.str());
          continue;
        }

      state.rows[offset + row + state.buffer_size].attributes = inst->term->disptext[row]->lattr;
      for (auto col = 0; col < inst->term->cols; ++col)
        {
          int relative_offset;
          absolute_offset = col;

          // load the character and all its combining characters
          do
            {
              characters.push_back( inst->term->disptext[row]->chars[ absolute_offset ].chr );
              relative_offset = inst->term->disptext[row]->chars[ absolute_offset ].cc_next;
              absolute_offset += relative_offset;
            } while(relative_offset != 0);
          set_cell(offset + row, col, characters, inst->term->disptext[row]->chars[col].attr);
          characters.clear();
        }
    }
}

void
TermcapParser::log_message(const std::string &message) const
{
  if (log_callback)
    log_callback(message);
}

const State &
TermcapParser::get_state() const
{
  int buffer_line_count = sblines(inst->term);

  if (buffer_line_count < 0)
    {
      std::ostringstream stream;
      stream << "Negative scrollback lines received; number='" << buffer_line_count << "'";
      log_message(stream.str());
      buffer_line_count = 0;
    }

  if (buffer_line_count > terminal_buffer_height)
    {
      std::ostringstream stream;
      stream << "Too big scrollback lines received; number='" << buffer_line_count << "'";
      log_message(stream.str());
      buffer_line_count = terminal_buffer_height;
    }

  resize(state, inst->term->cols, inst->term->rows, buffer_line_count);
  state.palette = palette;

  /* update terminal display buffer */
  term_update(inst->term);

  {
    /* write buffer content */
    int offset;
    for (offset = -buffer_line_count; offset <= -inst->term->rows; offset += inst->term->rows)
      {
        copy_term_content_to_cache(offset, inst->term->rows);
      }
    /* write remainder buffer content which is less than a complete terminal screen */
    copy_term_content_to_cache(offset, -offset);
  }

  state.cursor = {inst->term->curs.x, inst->term->curs.y, inst->term->cursor_on == 1};

  /*
   * Scroll to the current content of the display. The content will be updated
   * automatically by update_display.
   */
  term_scroll(inst->term, -1, 0);

  state.alternate_screen = (inst->term->alt_which != 0);

  return state;
}

void
TermcapParser::replace_standalone_linefeeds(bool enable)
{
  inst->term->cfg.lfhascr = (enable ? 1 : 0);
}

void
TermcapParser::clear_buffer()
{
  term_clrsb(inst->term);
}

void
TermcapParser::update_display(int x, int y, const std::wstring &str, unsigned long attr, long lattr)
{
  std::wstring chr;
  if (!is_valid_row(state, y))
    {
      std::ostringstream stream;
      stream << "Invalid row requested to be updated; row='" << y << "'";
      log_message(stream.str());
      return;
    }

  state.rows[y + state.buffer_size].attributes = lattr;
  for (std::wstring::const_iterator it = str.begin(); it != str.end(); ++it)
    {
      if (it != str.begin() && !is_combining_character(*it))
        {
          set_cell(y, x, chr, attr);
          ++x;
          chr.clear();
        }
      chr.push_back(*it);
    }

  if (!chr.empty())
    set_cell(y, x, chr, attr);
}

void
TermcapParser::set_log_callback(LogCallback log_callback)
{
  this->log_callback = log_callback;
}
