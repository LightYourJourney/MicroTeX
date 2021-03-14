#include <utility>

#include "unimath/uni_font.h"
#include "unimath/uni_symbol.h"

using namespace tex;
using namespace std;

MathVersion::MathVersion(
  c32 digit, c32 latinSmall, c32 latinCapital, c32 greekSmall, c32 greekCapital
) noexcept: _codepoints{0, digit, latinSmall, latinCapital, greekSmall, greekCapital} {}

pair<MathType, c32> MathVersion::ofChar(c32 codepoint) {
  if (codepoint >= '0' && codepoint <= '9') return {MathType::digit, codepoint - '0'};
  if (codepoint >= 'a' && codepoint <= 'z') return {MathType::latinSmall, codepoint - 'a'};
  if (codepoint >= 'A' && codepoint <= 'Z') return {MathType::latinCapital, codepoint - 'A'};
  if (codepoint >= 0x03B1 && codepoint <= 0x03C9) {  // α - ω
    return {MathType::greekSmall, codepoint - 0x03B1};
  }
  if (codepoint >= 0x0391 && codepoint <= 0x03A9) {  // Α - Ω
    return {MathType::greekCapital, codepoint - 0x0391};
  }
  return {MathType::none, codepoint};
}

c32 MathVersion::map(const c32 codepoint) const {
  auto[type, offset] = ofChar(codepoint);
  return _codepoints[static_cast<u8>(type)] + offset;
}

OtfFont::OtfFont(i32 id, string fontFile, const string& clmFile)
  : _id(id), _fontFile(std::move(fontFile)), _otf(sptr<const Otf>(Otf::fromFile(clmFile.c_str()))) {}

inline sptr<const OtfFont>& FontFamily::operator[](const string& styleName) {
  return _styles[styleName];
}

const string FontContext::_emptyVersionName;
const string FontContext::_defaultVersionName{"mathnormal"};

#define version(name, digit, latinSmall, latinCapital, greekSmall, greekCapital)                 \
  {                                                                                              \
    #name, sptrOf<const MathVersion>(digit, latinSmall, latinCapital, greekSmall, greekCapital)  \
  }

map<string, sptr<const MathVersion>> FontContext::_mathStyles{
  version(TeX, '0', 0x1D44E, 0x1D434, 0x1D6FC, 0x0391),
  version(ISO, '0', 0x1D44E, 0x1D434, 0x1D6FC, 0x1D6E2),
  version(French, '0', 0x1D44E, 'A', 0x03B1, 0x0391),
  version(upright, '0', 'a', 'A', 0x03B1, 0x0391),
};

map<string, sptr<const MathVersion>> FontContext::_mathVersions{
  {_emptyVersionName, _mathStyles["TeX"]},
  {_defaultVersionName, _mathStyles["TeX"]},
  version(mathrm, '0', 'a', 'A', 0x03B1, 0x0391),
  version(mathbf, 0x1D7CE, 0x1D41A, 0x1D400, 0x1D6C2, 0x1D6A8),
  version(mathit, '0', 0x1D44E, 0x1D434, 0x1D6FC, 0x1D6E2),
  version(mathbfit, '0', 0x1D482, 0x1D468, 0x1D736, 0x1D71C),
  version(mathcal, '0', 0x1D4B6, 0x1D49C, 0x03B1, 0x0391),
  version(mathbfcal, '0', 0x1D4EA, 0x1D4D0, 0x03B1, 0x0391),
  version(mathfrak, '0', 0x1D51E, 0x1D504, 0x03B1, 0x0391),
  version(mathbb, 0x1D7D8, 0x1D552, 0x1D538, 0x03B1, 0x0391),
  version(mathbffrak, '0', 0x1D586, 0x1D552, 0x03B1, 0x0391),
  version(mathsf, 0x1D7E2, 0x1D5BA, 0x1D5A0, 0x03B1, 0x0391),
  version(mathsfbf, 0x1D7EC, 0x1D5EE, 0x1D5D4, 0x1D770, 0x1D756),
  version(mathsfit, '0', 0x1D622, 0x1D608, 0x03B1, 0x0391),
  version(mathsfbfit, '0', 0x1D656, 0x1D63C, 0x1D7AA, 0x1D790),
  version(mathtt, 0x1D7F6, 0x1D68A, 0x1D670, 0x03B1, 0x0391),
};

int FontContext::_lastId = 0;
vector<sptr<const OtfFont>> FontContext::_fonts;

map<string, sptr<FontFamily>> FontContext::_mainFonts;
map<string, sptr<const OtfFont>> FontContext::_mathFonts;

u16 FontContext::fontStyleOf(const std::string& name) {
  static const map<string, FontStyle> nameStyle{
    {"mathrm",     FontStyle::rm},
    {"mathbf",     FontStyle::bf},
    {"mathit",     FontStyle::it},
    {"mathbfit",   FontStyle::bfit},
    {"mathcal",    FontStyle::cal},
    {"mathbfcal",  FontStyle::bfcal},
    {"mathfrak",   FontStyle::frak},
    {"mathbb",     FontStyle::bb},
    {"mathbffrak", FontStyle::bffrak},
    {"mathsf",     FontStyle::sf},
    {"mathsfbf",   FontStyle::sfbf},
    {"mathsfit",   FontStyle::sfit},
    {"mathsfbfit", FontStyle::sfbfit},
    {"mathtt",     FontStyle::tt},
  };
}

void FontContext::addMainFont(const string& versionName, const vector<FontSpec>& params) {
  auto* ptr = new FontFamily();
  FontFamily& f = *ptr;
  for (const auto&[style, font, clm] : params) {
    auto otf = sptrOf<const OtfFont>(_lastId++, font, clm);
    _fonts.push_back(otf);
    f[style] = otf;
  }
  _mainFonts[versionName] = sptr<FontFamily>(ptr);
}

void FontContext::addMathFont(const FontSpec& params) {
  const auto&[version, font, clm] = params;
  auto otf = sptrOf<OtfFont>(_lastId++, font, clm);
  _fonts.push_back(otf);
  _mathFonts[version] = otf;
}

void FontContext::setMathStyle(const string& styleName) {
  auto it = _mathStyles.find(styleName);
  if (it == _mathStyles.end()) return;
  _mathVersions[_defaultVersionName] = it->second;
}

sptr<const OtfFont> FontContext::getFont(i32 id) {
  if (id >= _fonts.size() || id < 0) return nullptr;
  return _fonts[id];
}

void FontContext::selectMathFont(const string& versionName) {
  _mathFont = _mathFonts[versionName];
}

void FontContext::selectMainFont(const string& versionName) {
  _mainFont = _mainFonts[versionName];
}

Char FontContext::getChar(c32 code, const string& style, bool isMathMode) const {
  if (isMathMode) {
    const auto ptr = _mathVersions[style];
    const MathVersion& version = ptr == nullptr ? *_mathVersions[""] : *ptr;
    const c32 unicode = version.map(code);
    return {code, unicode, _mathFont->_id, _mathFont->otf().glyphId(unicode)};
  } else {
    sptr<const OtfFont> font = (*_mainFont)[style];
    if (font == nullptr) font = (*_mainFont)[""];
    if (font == nullptr) font = _mathFont;
    return {code, code, font->_id, font->otf().glyphId(code)};
  }
}

Char FontContext::getChar(c32 code, FontStyle style, bool isMathMode) const {
  // TODO
}

Char FontContext::getChar(const std::string& symbol) const {
  const Symbol* sym = Symbol::get(symbol.c_str());
  if (sym == nullptr) return {};
}
