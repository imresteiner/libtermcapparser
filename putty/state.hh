#ifndef PUTTY_STATE_HH_INCLUDE
#define PUTTY_STATE_HH_INCLUDE

#include <putty/cell.hh>
#include <putty/row.hh>
#include <putty/palette.hh>

#include <string>

namespace Putty
{

  /**
   * Terminal state object. Contains the content of a terminal
   * at any given time.
   */
  class State
  {
    friend class TermcapParser;

  public:
    /**
     * Copy another state.
     */
    State(const State &other);

    /**
     * Destructor.
     */
    virtual ~State();

    State &operator=(const State &other);

    /**
     * Get terminal width.
     */
    inline unsigned get_width() const
    { return width; }

    /**
     * Get terminal height.
     */
    inline unsigned get_height() const
    { return height; }

    /**
     * Get line count of terminal buffer.
     */
    inline unsigned get_buffer_size() const
    { return buffer_size; }

    /**
     * Whether it is an alternate screen or not.
     */
    inline bool is_alternate_screen() const
    { return alternate_screen; }

    /**
     * Get the character and attribute pair (a Cell object) at any given position.
     * Negative row number means buffer content zero or positive means screen content.
     *
     * @param row Row of the cell.
     * @param col Column of the cell.
     * @return Cell reference at the given position.
     */
    const Cell *get_cell(int row, unsigned col) const;
    Cell *get_cell(int row, unsigned col);

    /**
     * Get the given row.
     *
     * @param row Row position.
     * @return Reference to Row object.
     */
    const Row *get_row(int row) const;

    /**
     * Get the palette of the state.
     */
    const Palette &get_palette() const
    { return palette; }

    /**
     * Whether the cursor enabled flag is true.
     */
    bool is_cursor_enabled() const
    { return cursor_enabled; }

    /**
     * Get the cursor's x coordinate.
     */
    int get_cursor_x() const
    { return cursor_x; }

    /**
     * Get the cursor's y coordinate.
     */
    int get_cursor_y() const
    { return cursor_y; }

    /**
     * Set the state palette.
     *
     * @param palette The new palette. It will be copied.
     */
    void set_palette(const Palette &palette)
    { this->palette = palette; }

  protected:
    /**
     * Create an empty state.
     */
    State();

    void swap(State &other);

    /**
     * Set the character and attribute pair (a Cell object) at any given position.
     * Negative row number means buffer content zero or positive means screen content.
     *
     * @param row Row of the cell.
     * @param col Column of the cell.
     * @param chr Character of the cell.
     * @param attr Attributes of the cell.
     *
     * Returns if setting the cell was successful, or not (the row/column was out of bounds).
     */
    bool set_cell(int row, unsigned col, const std::wstring &characters, uint64_t attr);

    /**
     * Set the set_alternate_screen according to the given parameter.
     *
     * @param alternate_screen Is it an alternate screen?
     */
    void set_alternate_screen(bool alternate_screen);

    /**
     * Set cursor attributes.
     *
     * @param enabled Whether the cursor is enabled or not.
     * @param The x coordinate.
     * @param The y coordinate.
     */
    void set_cursor(bool enabled, int x, int y);

    /**
     * Get the given row. The internal version does not require a const object.
     *
     * @param row Row position.
     * @return Pointer to Row object, can be null.
     */
    Row *get_row_internal(int row);

    /**
     * Resize the state. Any data is kept or trimmed.
     *
     * @param a_width Width of the terminal.
     * @param a_height Height of the terminal.
     * @param a_buffer_size Number of rows in the buffer.
     * @param a_palette RGB color palette.
     */
    void resize(unsigned a_width, unsigned a_height, unsigned a_buffer_size);

    /**
     * Change the display area size of the state.
     *
     * @param a_width Width of the terminal.
     * @param a_height Height of the terminal.
     */
    void resize_display(unsigned a_width, unsigned a_height)
    { resize(a_width, a_height, buffer_size); }

  private:
    /** Width of the terminal */
    unsigned width;
    /** Height of the terminal */
    unsigned height;
    /** Line count of buffer */
    unsigned buffer_size;
    /** Is alternate screen? */
    bool alternate_screen;
    /** Is cursor enabled? */
    bool cursor_enabled;
    /** The cursor's x coordinate. */
    int cursor_x;
    /** The cursor's y coordinate. */
    int cursor_y;

    /** List of rows. */
    std::vector< Row > rows;

    /** Palette of the state */
    Palette palette;

  private:
    /**
     * Check if the coordinates are valid on the actual State.buffer_size
     *
     * @return true if the coordinates are invalid
     */
    bool is_invalid_row_value(int row) const;

    bool is_valid_cell(int row, unsigned column) const;
  };

} // namespace Putty

#endif // PUTTY_STATE_HH_INCLUDE
