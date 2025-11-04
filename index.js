const binding = require('./binding')

exports.parse = function parse(input, onparse, flags = binding.MD_DIALECT_GITHUB, logDebug = false) {
  const err = binding.parse(input, onparse, flags, logDebug)

  if (err) throw new Error('parse error')
}

exports.toHTML = function toHTML(
  input,
  parserFlags = binding.MD_DIALECT_GITHUB,
  htmlFlags = 0
) {
  let html = ''

  function onwrite (chunk) {
    html += chunk
  }

  const res = binding.toHTML(input, onwrite, parserFlags, htmlFlags)

  if (res < 0) throw new Error('Markdown conversion failed')

  return html
}

exports.constants = {
  events: {
    BLOCK_ENTER: binding.EV_BLOCK_ENTER,
    BLOCK_LEAVE: binding.EV_BLOCK_LEAVE,
    SPAN_ENTER: binding.EV_SPAN_ENTER,
    SPAN_LEAVE: binding.EV_SPAN_LEAVE,
    TEXT: binding.EV_TEXT
  },

  block: {
    DOC: binding.MD_BLOCK_DOC,
    QUOTE: binding.MD_BLOCK_QUOTE,
    UL: binding.MD_BLOCK_UL,
    OL: binding.MD_BLOCK_OL,
    LI: binding.MD_BLOCK_LI,
    HR: binding.MD_BLOCK_HR,
    H: binding.MD_BLOCK_H,
    CODE: binding.MD_BLOCK_CODE,
    HTML: binding.MD_BLOCK_HTML,
    P: binding.MD_BLOCK_P,
    TABLE: binding.MD_BLOCK_TABLE,
    THEAD: binding.MD_BLOCK_THEAD,
    TBODY: binding.MD_BLOCK_TBODY,
    TR: binding.MD_BLOCK_TR,
    TH: binding.MD_BLOCK_TH,
    TD: binding.MD_BLOCK_TD
  },

  span: {
    EM: binding.MD_SPAN_EM,
    STRONG: binding.MD_SPAN_STRONG,
    A: binding.MD_SPAN_A,
    IMG: binding.MD_SPAN_IMG,
    CODE: binding.MD_SPAN_CODE,
    DEL: binding.MD_SPAN_DEL,
    LATEXMATH: binding.MD_SPAN_LATEXMATH,
    LATEXMATH_DISPLAY: binding.MD_SPAN_LATEXMATH_DISPLAY,
    WIKILINK: binding.MD_SPAN_WIKILINK,
    U: binding.MD_SPAN_U
  },

  text: {
    NORMAL: binding.MD_TEXT_NORMAL,
    NULLCHAR: binding.MD_TEXT_NULLCHAR,
    BR: binding.MD_TEXT_BR,
    SOFTBR: binding.MD_TEXT_SOFTBR,
    ENTITY: binding.MD_TEXT_ENTITY,
    CODE: binding.MD_TEXT_CODE,
    HTML: binding.MD_TEXT_HTML,
    LATEXMATH: binding.MD_TEXT_LATEXMATH
  },

  flags: {
    COLLAPSEWHITESPACE: binding.MD_FLAG_COLLAPSEWHITESPACE,
    PERMISSIVEATXHEADERS: binding.MD_FLAG_PERMISSIVEATXHEADERS,
    PERMISSIVEURLAUTOLINKS: binding.MD_FLAG_PERMISSIVEURLAUTOLINKS,
    PERMISSIVEEMAILAUTOLINKS: binding.MD_FLAG_PERMISSIVEEMAILAUTOLINKS,
    NOINDENTEDCODEBLOCKS: binding.MD_FLAG_NOINDENTEDCODEBLOCKS,
    NOHTMLBLOCKS: binding.MD_FLAG_NOHTMLBLOCKS,
    NOHTMLSPANS: binding.MD_FLAG_NOHTMLSPANS,
    TABLES: binding.MD_FLAG_TABLES,
    STRIKETHROUGH: binding.MD_FLAG_STRIKETHROUGH,
    PERMISSIVEWWWAUTOLINKS: binding.MD_FLAG_PERMISSIVEWWWAUTOLINKS,
    TASKLISTS: binding.MD_FLAG_TASKLISTS,
    LATEXMATHSPANS: binding.MD_FLAG_LATEXMATHSPANS,
    WIKILINKS: binding.MD_FLAG_WIKILINKS,
    UNDERLINE: binding.MD_FLAG_UNDERLINE,
    HARD_SOFT_BREAKS: binding.MD_FLAG_HARD_SOFT_BREAKS,
    PERMISSIVEAUTOLINKS: binding.MD_FLAG_PERMISSIVEAUTOLINKS,
    NOHTML: binding.MD_FLAG_NOHTML,

    // flag-combos
    DIALECT_COMMONMARK: binding.MD_DIALECT_COMMONMARK,
    DIALECT_GITHUB: binding.MD_DIALECT_GITHUB
  },

  align: {
    DEFAULT: binding.MD_ALIGN_DEFAULT,
    LEFT: binding.MD_ALIGN_LEFT,
    CENTER: binding.MD_ALIGN_CENTER,
    RIGHT: binding.MD_ALIGN_RIGHT
  },

  htmlFlags: {
    DEBUG: binding.MD_HTML_FLAG_DEBUG,
    VERBATIM_ENTITIES: binding.MD_HTML_FLAG_VERBATIM_ENTITIES,
    SKIP_UTF8_BOM: binding.MD_HTML_FLAG_SKIP_UTF8_BOM,
    XHTML: binding.MD_HTML_FLAG_XHTML
  }
}
