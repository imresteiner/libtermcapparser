#ifndef PUTTY_ATTRIBUTES_HH_INCLUDE
#define PUTTY_ATTRIBUTES_HH_INCLUDE

#include <string>

#include <putty/palette.hh>

namespace Putty
{

  /**
   * Represents a cell in the display state of the terminal. A cell
   * contains a character represented as an unsigned long and the
   * cell attributes (foreground color, background color, bold, etc.).
   *
   * A cell is considered valid if the character is not 0, since the 0
   * character is not used to represent anything.
   */
  class Cell
  {
    friend class Row;

  public:
    /**
     * Compatibility definition so old interface is still usable.
     *
     * TODO: Remove after integration.
     */
    typedef unsigned long Character;

    /** Character attributes */
    typedef unsigned long Attributes;
    /** Color code */
    typedef unsigned short Color;

  public:
    /**
     * Character attribute flags.
     */
    enum Flags
    {
      NARROW = 0x800000U,
      WIDE = 0x400000U,
      BOLD = 0x040000U,
      UNDER = 0x080000U,
      REVERSE = 0x100000U,
      BLINK = 0x200000U,
    };

    /**
     * There are some special cases for color values. In these
     * cases we translate it to a "normal" RGB value.
     */
    enum ColorSettings
    {
      DEFAULT_FOREGROUND = 0,
      DEFAULT_FOREGROUND_BOLD = 1,
      DEFAULT_BACKGROUND = 2,
      DEFAULT_BACKGROUND_BOLD = 3,
      CURSOR_FOREGROUND = 4,
      CURSOR_BACKGROUND = 5,

      COLOR_SETTING_COUNT,
    };

    /** Mask of character flags */
    static const Attributes FlagMask;

    /** Foreground color mask */
    static const Attributes FgColorMask;
    /** Foreground color shift */
    static const int FgColorShift;

    /** Background color mask */
    static const Attributes BgColorMask;
    /** Background color shift */
    static const int BgColorShift;

    /** Invalid color combination */
    static const Attributes InvalidColor;
    /** Mask of valid part of the attribute data */
    static const Attributes ValidMask;

    /** Default foreground color code */
    static const Color DefaultForeground;
    /** Default background color code */
    static const Color DefaultBackground;

    /** Default cell attributes */
    static const Attributes DefaultAttributes;

  public:
    /**
     * Copy constructor.
     */
    Cell(const Cell &other);

    /**
     * Assign another cell to this cell.
     *
     * @param other Other cell.
     * @return Reference to this cell.
     */
    Cell &operator=(const Cell &other);

    /** Get cell characters and its combining characters. */
    inline const std::wstring& get_characters() const
    { return characters; }

    /**
     * Compatibility definition so old interface is still usable.
     *
     * TODO: Remove after integration.
     */
    inline unsigned long get_character() const
    {
      if (characters.empty())
        return 0;

      return characters[0];
    }

    /** Get cell character attributes. */
    inline Attributes get_attributes() const
    { return attr; }

    /** Return true if the character is narrow. */
    inline bool narrow() const
    { return attr & NARROW; }

    /** Return true if the character is wide. */
    inline bool wide() const
    { return attr & WIDE; }

    /** Return true if the character is bold. */
    inline bool bold() const
    { return attr & BOLD; }

    /** Return true if the character is underscored. */
    inline bool under() const
    { return attr & UNDER; }

    /** Return true if the character is reversed. */
    inline bool reverse() const
    { return attr & REVERSE; }

    /** Return true if the character is blinking. */
    inline bool blink() const
    { return attr & BLINK; }

    /** Return foreground color code of the cell. */
    inline Color foreground_code() const
    { return (Color)((attr & FgColorMask) >> FgColorShift); }

    /** Return background color code of the cell. */
    inline Color background_code() const
    { return (Color)((attr & BgColorMask) >> BgColorShift); }

  protected:
    /**
     * Construct an empty cell. The empty cell is invalid.
     */
    Cell();

    /**
     * Create a cell with the character and attributes set
     *
     * @param characters This wstring contains the character, and its combining characters
     * @param attr Character attributes of the cell.
     * @param combining_chrs List of the combining characters of chr.
     */
    Cell(const std::wstring &characters, Attributes attr);

    /**
     * Set cell values.
     *
     * @param characters This wstring contains the character, and its combining characters
     * @param attr Character attributes of the cell.
     * @param combining_chrs List of the combining characters of chr.
     */
public:
    void set(const std::wstring &characters, Attributes attr);

    void set_characters(const std::wstring& characters)
    { this->characters = characters; }

    void set_attributes(Attributes attributes)
    { this->attr = attributes & ValidMask; }

  private:
    /** Character value of the cell. */
    std::wstring characters;
    /** Attribute value of the cell. */
    Attributes attr;
  };

} // namespace Putty

#endif // PUTTY_ATTRIBUTES_HH_INCLUDE
