/**
 * @mainpage Putty termcap parser library
 *
 * The purpose of this library is to provide a C++ interface for the terminal
 * emulation part of putty. This library implements no display logic, only parsing.
 *
 * The parser can be accessed through the Putty::TermcapParser class. All other classes
 * should be used through this class. Because of this, most of the classes are
 * immutable (but their protected interfaces expose methods that can change the
 * object states).
 */
#ifndef PUTTY_TERMCAPPARSER_HH_INCLUDE
#define PUTTY_TERMCAPPARSER_HH_INCLUDE

#include <putty/state.hh>
#include <putty/palette.hh>

/** Forward declaration of the handle structure used by the putty core code. */
struct gui_data;

/**
 * Namespace for Putty C++ interface classes.
 */
namespace Putty
{

  /**
   * The LogCallback is used to log warning messages.
   */
  typedef void (*LogCallback)(const std::string &message);

  /**
   * Termcap parser wrapping for the terminal code in putty.
   */
  class TermcapParser
  {
  public:
    /**
     * Constructor.
     *
     * @param charset Character encoding of the terminal emulation.
     * @param terminal_buffer_height Terminal buffer height, zero means buffer is disabled.
     */
    TermcapParser(const char *charset, int terminal_buffer_height = 100000);

    /**
     * Destructor.
     */
    ~TermcapParser();

    /**
     * Parse input data. The input is a stream of data that putty
     * will parse after filtering unhandled sequences.
     *
     * @param data Raw termcap data.
     * @param len Length of data.
     */
    void data_input(const char *data, int len);

    /**
     * Set the terminal size.
     *
     * @param width Width of the terminal.
     * @param height Height of the terminal.
     */
    void set_terminal_size(int width, int height);

    /**
     * Get current terminal state. The result is only a reference
     * to an internal State object, so calling this function more
     * than once does not carry any overhead penalty unless the
     * terminal is changed. The result will be deleted once this
     * happens making any outside references invalid, so if the state
     * needs to be stored longer, it must be copied.
     *
     * @return Current terminal state.
     */
    const State &get_state() const;

    /**
     * Enable lfhascr. When enabled, putty will handle LF characters
     * without any CR as CR-LF-s.
     *
     * @param enable If set, standalone LF's will be replaced with CRLF
     */
    void replace_standalone_linefeeds(bool enable);

    /**
     * Clear the buffer content which is not on screen.
     */
    void clear_buffer();

    /**
     * Retrieve the palette.
     */
    Palette &get_palette()
    { return palette; }

    /**
     * Retrieve the palette, const version
     */
    const Palette &get_palette() const
    { return palette; }

    /**
     * Update the display with the given data.
     */
    void update_display(int x, int y, const std::wstring &str, unsigned long attr, long lattr);

    /**
     * Change the log callback function.
     *
     * @param log_callback The callback function
     */
    void set_log_callback(LogCallback log_callback);

  protected:
    /**
     * Set the buffer size.
     *
     * @param width Width of the buffer.
     * @param height Height of the buffer.
     */
    void set_buffer_size(int width, int height);

    /**
     * Parse input data. The input is a raw stream of termcap data
     * that putty will parse.
     *
     * @param data Filtered termcap data.
     * @param len Length of data.
     */
    void data_input_filtered(const char *data, int len);

  private:
    void set_cell(int row, unsigned col, const std::wstring &characters, Cell::Attributes attr) const;

    /**
     * Copy the content of the terminal into the cached state.
     *
     * @param offset Row offset to copy
     * @param row_count Number of rows to copy.
     */
    void copy_term_content_to_cache(int offset, unsigned row_count) const;

    /**
     * Log message if the log callback is set.
     *
     * @param message The message to log
     */
    void log_message(const std::string &message) const;

  protected:
    struct gui_data *inst;                /**< Putty internal terminal representation. */
    mutable State state;                  /**< Current terminal state */

    Palette palette;                      /**< Currently used palette */

    int terminal_buffer_height;           /**< Terminal buffer height */
    LogCallback log_callback;             /**< Log callback function */

    std::string buffered_data_input;      /**< Buffer data input when control sequence was not complete */
  };

} // namespace Putty

#endif // PUTTY_TERMCAPPARSER_HH_INCLUDE
