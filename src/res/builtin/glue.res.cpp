#include "core/core.h"

using namespace tex;

inline static Glue* g(const std::string& name, float space, float stretch, float shrink) {
  return new Glue(space, stretch, shrink, name);
}

std::vector<Glue*> Glue::_glueTypes = {
  g("default", 0, 0, 0),
  g("thin", 3, 0, 0),
  g("med", 4, 4, 2),
  g("thick", 5, 0, 5),
};

/*
 GLUE TABLE
 Page 181 in [The TeXBook]
 -------------------------------------------------------
       ORD   OP    BIN   REL   OPEN  CLOSE  PUNCT  INNER
 ORD    0     1    (2)   (3)    0     0      0     (1)
 OP     1     1     *    (3)    0     0      0     (1)
 BIN   (2)   (2)    *     *    (2)    *      *     (2)
 REL   (3)   (3)    *     0    (3)    0      0     (3)
 OPEN  (0)    0     *     0     0     0      0      0
 CLOSE (0)    1    (2)   (3)    0     0      0     (1)
 PUNCT (1)   (1)    *    (1)   (1)   (1)    (1)    (1)
 INNER (1)    1    (2)   (3)   (1)    0     (1)    (1)

 0: no space
 1: thin space
 2: medium space
 3: thick space

 The table entry is parenthesized if the space is to be inserted only in
 display and text styles, not in script and scriptscript styles.

 Some of the entries in the table are ‘*’; such cases never arise, because
 Bin atoms must be preceded and followed by atoms compatible with the
 nature of binary operations.
*/
const char Glue::_table[TYPE_COUNT][TYPE_COUNT][STYLE_COUNT] = {
  {"0000", "1111", "2200", "3300", "0000", "0000", "0000", "1100"},
  {"1111", "1111", "0000", "3300", "0000", "0000", "0000", "1100"},
  {"2200", "2200", "0000", "0000", "2200", "0000", "0000", "2200"},
  {"3300", "3300", "0000", "0000", "3300", "0000", "0000", "3300"},
  {"0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000"},
  {"0000", "1111", "2200", "3300", "0000", "0000", "0000", "1100"},
  {"1100", "1100", "0000", "1100", "1100", "1100", "1100", "1100"},
  {"1100", "1111", "2200", "3300", "1100", "0000", "1100", "1100"},
};
