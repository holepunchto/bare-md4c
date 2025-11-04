#include <assert.h>
#include <bare.h>
#include <js.h>
#include <jstl.h>
#include <md4c.h>
#include <md4c-html.h>
#include <utf.h>

using bare_md4c_onpush_t = js_function_t<int, int, int, std::optional<std::string>, std::optional<js_object_t>>;
using bare_md4c_onchunk_t = js_function_t<void, std::string>;

struct bare_md4c_t {
  js_env_t *env;
  bare_md4c_onpush_t &onpush;
};

struct bare_md4c_html_t {
  js_env_t *env;
  bare_md4c_onchunk_t &onwrite;
};

enum bare_md4c_event_t {
  EV_BLOCK_ENTER = 0,
  EV_BLOCK_LEAVE,
  EV_SPAN_ENTER,
  EV_SPAN_LEAVE,
  EV_TEXT
};

static int
bare_md4c_marshall_mdattribute(js_env_t *env, js_object_t &target, const std::string &name, MD_ATTRIBUTE &attr) {
  int err = 0;

  js_object_t obj;

  err = js_create_object(env, obj);
  if (err < 0) return err;

  err = js_set_property(env, target, name, obj);
  if (err < 0) return err;

  err = js_set_property(env, obj, "len", attr.size);
  if (err < 0) return err;

  if (!attr.size) return 0;

  err = js_set_property(env, obj, "text", std::string(attr.text, attr.size));
  if (err < 0) return err;

  std::vector<std::tuple<int, int>> substrings;
  substrings.reserve(attr.size);

  assert(attr.substr_offsets[0] == 0);

  for (int i = 0; attr.substr_offsets[i] != attr.size; i++) {
    substrings.emplace_back(attr.substr_types[i], attr.substr_offsets[i]);
  }

  js_array_t array;
  err = js_create_array(env, substrings, array);
  if (err < 0) return err;

  err = js_set_property(env, obj, "substrings", array);
  if (err < 0) return err;

  return 0;
}

static int
bare_md4c_marshall_mdblock_detail(js_env_t *env, MD_BLOCKTYPE type, void *detail, js_object_t &obj) {
  int err;
  switch (type) {
    // blocks without details
  case MD_BLOCK_DOC:
  case MD_BLOCK_HTML:
  case MD_BLOCK_P:
  case MD_BLOCK_HR:
  case MD_BLOCK_QUOTE:
  case MD_BLOCK_THEAD:
  case MD_BLOCK_TBODY:
  case MD_BLOCK_TR:
    break;

    // blocks with specified details

  case MD_BLOCK_UL: {
    auto ul = reinterpret_cast<MD_BLOCK_UL_DETAIL *>(detail);

    err = js_set_property(env, obj, "mark", std::string(ul->mark, 1));
    if (err < 0) return err;

    err = js_set_property(env, obj, "isTight", static_cast<bool>(ul->is_tight));
    if (err < 0) return err;
  } break;

  case MD_BLOCK_OL: {
    auto ol = reinterpret_cast<MD_BLOCK_OL_DETAIL *>(detail);

    err = js_set_property(env, obj, "markDelimiter", std::string(ol->mark_delimiter, 1));
    if (err < 0) return err;

    err = js_set_property(env, obj, "start", ol->start);
    if (err < 0) return err;

    err = js_set_property(env, obj, "isTight", static_cast<bool>(ol->is_tight));
    if (err < 0) return err;
  } break;

  case MD_BLOCK_LI: {
    auto li = reinterpret_cast<MD_BLOCK_LI_DETAIL *>(detail);

    err = js_set_property(env, obj, "isTask", static_cast<bool>(li->is_task));
    if (err < 0) return err;

    err = js_set_property(env, obj, "taskMark", std::string(li->task_mark, 1));
    if (err < 0) return err;

    err = js_set_property(env, obj, "taskMarkOffset", li->task_mark_offset);
    if (err < 0) return err;
  } break;

  case MD_BLOCK_H: {
    auto h = reinterpret_cast<MD_BLOCK_H_DETAIL *>(detail);

    err = js_set_property(env, obj, "level", h->level);
    if (err < 0) return err;
  } break;

  case MD_BLOCK_CODE: {
    auto code = reinterpret_cast<MD_BLOCK_CODE_DETAIL *>(detail);

    err = js_set_property(env, obj, "fenceChar", std::string(code->fence_char, 1));
    if (err < 0) return err;

    err = bare_md4c_marshall_mdattribute(env, obj, "lang", code->lang);
    if (err < 0) return err;

    err = bare_md4c_marshall_mdattribute(env, obj, "info", code->info);
    if (err < 0) return err;
  } break;

  case MD_BLOCK_TABLE: {
    auto table = reinterpret_cast<MD_BLOCK_TABLE_DETAIL *>(detail);

    err = js_set_property(env, obj, "columns", table->col_count);
    if (err < 0) return err;

    err = js_set_property(env, obj, "rowsHead", table->head_row_count);
    if (err < 0) return err;

    err = js_set_property(env, obj, "rowsBody", table->body_row_count);
    if (err < 0) return err;
  } break;

  case MD_BLOCK_TH:
  case MD_BLOCK_TD: {
    auto td = reinterpret_cast<MD_BLOCK_TD_DETAIL *>(detail);

    err = js_set_property(env, obj, "align", static_cast<int>(td->align));
    if (err < 0) return err;
  } break;

  default:
    fprintf(stderr, "[MD4C] unknown MD_BLOCKTYPE=%i\n", type);
    assert(0 && "MD_BLOCKTYPE");
    return -1;
  }

  return 0;
}

static int
bare_md4c_marshall_mdspan_detail(js_env_t *env, MD_SPANTYPE type, void *detail, js_object_t &obj) {
  int err;

  switch (type) {
    // spans without details
  case MD_SPAN_CODE:
  case MD_SPAN_STRONG:
  case MD_SPAN_EM:
  case MD_SPAN_U:
  case MD_SPAN_DEL:
    break;

  case MD_SPAN_A: {
    auto a = reinterpret_cast<MD_SPAN_A_DETAIL *>(detail);

    err = bare_md4c_marshall_mdattribute(env, obj, "href", a->href);
    if (err < 0) return err;

    err = bare_md4c_marshall_mdattribute(env, obj, "title", a->title);
    if (err < 0) return err;

    err = js_set_property(env, obj, "auto", static_cast<bool>(a->is_autolink));
    if (err < 0) return err;
  } break;

  case MD_SPAN_IMG: {
    auto img = reinterpret_cast<MD_SPAN_IMG_DETAIL *>(detail);

    err = bare_md4c_marshall_mdattribute(env, obj, "src", img->src);
    if (err < 0) return err;

    err = bare_md4c_marshall_mdattribute(env, obj, "title", img->title);
    if (err < 0) return err;
  } break;

  case MD_SPAN_WIKILINK: {
    auto wl = reinterpret_cast<MD_SPAN_WIKILINK_DETAIL *>(detail);

    err = bare_md4c_marshall_mdattribute(env, obj, "target", wl->target);
    if (err < 0) return err;
  } break;

  default:
    fprintf(stderr, "[MD4C] unknown MD_SPANTYPE=%i\n", type);
    assert(0 && "MD_SPANTYPE");
    return -1;
  }

  return 0;
}

static int
on_block_enter(MD_BLOCKTYPE type, void *detail, void *data) {
  auto ctx = reinterpret_cast<bare_md4c_t *>(data);

  js_object_t obj;
  int err = js_create_object(ctx->env, obj);
  if (err < 0) return err;

  err = bare_md4c_marshall_mdblock_detail(ctx->env, type, detail, obj);
  if (err < 0) return err;

  int res;

  err = js_call_function<
    js_type_options_t{},
    int,
    int,
    int,
    std::optional<std::string>,
    std::optional<js_object_t>>(ctx->env, ctx->onpush, EV_BLOCK_ENTER, type, std::nullopt, obj, res);

  if (err < 0) return err;

  return res;
}

static int
on_block_leave(MD_BLOCKTYPE type, void *detail, void *data) {
  auto ctx = reinterpret_cast<bare_md4c_t *>(data);

  js_object_t obj;
  int err = js_create_object(ctx->env, obj);
  if (err < 0) return err;

  err = bare_md4c_marshall_mdblock_detail(ctx->env, type, detail, obj);
  if (err < 0) return err;

  int res;

  err = js_call_function<
    js_type_options_t{},
    int,
    int,
    int,
    std::optional<std::string>,
    std::optional<js_object_t>>(ctx->env, ctx->onpush, EV_BLOCK_LEAVE, type, std::nullopt, obj, res);

  if (err < 0) return err;

  return res;
}

static int
on_span_enter(MD_SPANTYPE type, void *detail, void *data) {
  auto ctx = reinterpret_cast<bare_md4c_t *>(data);

  js_object_t obj;
  int err = js_create_object(ctx->env, obj);
  if (err < 0) return err;

  err = bare_md4c_marshall_mdspan_detail(ctx->env, type, detail, obj);
  if (err < 0) return err;

  int res;
  err = js_call_function<
    js_type_options_t{},
    int,
    int,
    int,
    std::optional<std::string>,
    std::optional<js_object_t>>(ctx->env, ctx->onpush, EV_SPAN_ENTER, type, std::nullopt, obj, res);
  if (err < 0) return err;

  return res;
}

static int
on_span_leave(MD_SPANTYPE type, void *detail, void *data) {
  auto ctx = reinterpret_cast<bare_md4c_t *>(data);

  js_object_t obj;
  int err = js_create_object(ctx->env, obj);
  if (err < 0) return err;

  err = bare_md4c_marshall_mdspan_detail(ctx->env, type, detail, obj);
  if (err < 0) return err;

  int res;
  err = js_call_function<
    js_type_options_t{},
    int,
    int,
    int,
    std::optional<std::string>,
    std::optional<js_object_t>>(ctx->env, ctx->onpush, EV_SPAN_LEAVE, type, std::nullopt, obj, res);
  if (err < 0) return err;

  return res;
}

static int
on_text(MD_TEXTTYPE type, const MD_CHAR *txt, MD_SIZE len, void *data) {
  int err;
  auto ctx = reinterpret_cast<bare_md4c_t *>(data);

  int res;
  err = js_call_function<
    js_type_options_t{},
    int,
    int,
    int,
    std::optional<std::string>,
    std::optional<js_object_t>>(ctx->env, ctx->onpush, EV_TEXT, type, std::string(txt, len), std::nullopt, res);
  if (err < 0) return err;

  return res;
}

static void
on_debug_log(const char *message, void *data) {
  fprintf(stderr, "[MD4C]: %s\n", message);
}

static int
bare_md4c_parse(js_env_t *env, std::string md, bare_md4c_onpush_t handler, uint32_t flags, bool log_debug) {
  MD_PARSER parser = {
    .abi_version = 0,
    .flags = flags,
    .enter_block = on_block_enter,
    .leave_block = on_block_leave,
    .enter_span = on_span_enter,
    .leave_span = on_span_leave,
    .text = on_text,
    .debug_log = nullptr,
    .syntax = nullptr
  };

  if (log_debug) {
    parser.debug_log = on_debug_log;
  }

  bare_md4c_t ctx = {
    .env = env,
    .onpush = handler
  };

  return md_parse(md.c_str(), md.length(), &parser, &ctx);
}

static void
on_process_output(const MD_CHAR *chunk, MD_SIZE len, void *userptr) {
  auto ctx = reinterpret_cast<bare_md4c_html_t *>(userptr);

  int err = js_call_function<>(ctx->env, ctx->onwrite, std::string(chunk, len));
  assert(err == 0);
}

static int
bare_md4c_to_html(
  js_env_t *env,
  std::string md,
  bare_md4c_onchunk_t handler,
  uint32_t parser_flags,
  uint32_t renderer_flags
) {
  bare_md4c_html_t ctx = {
    .env = env,
    .onwrite = handler
  };
  return md_html(md.c_str(), md.length(), on_process_output, &ctx, parser_flags, renderer_flags);
}

static js_value_t *
bare_md4c_exports(js_env_t *env, js_value_t *exports) {
  int err;

  // functions
#define V(name, fn) \
  err = js_set_property<fn>(env, exports, name); \
  assert(err == 0);

  V("parse", bare_md4c_parse)
  V("toHTML", bare_md4c_to_html)

#undef V

  // constants
#define V(constant) \
  err = js_set_property(env, exports, #constant, static_cast<uint64_t>(constant)); \
  assert(err == 0);

  // block

  V(MD_BLOCK_DOC)
  V(MD_BLOCK_QUOTE)
  V(MD_BLOCK_UL)
  V(MD_BLOCK_OL)
  V(MD_BLOCK_LI)
  V(MD_BLOCK_HR)
  V(MD_BLOCK_H)
  V(MD_BLOCK_CODE)
  V(MD_BLOCK_HTML)
  V(MD_BLOCK_P)
  V(MD_BLOCK_TABLE)
  V(MD_BLOCK_THEAD)
  V(MD_BLOCK_TBODY)
  V(MD_BLOCK_TR)
  V(MD_BLOCK_TH)
  V(MD_BLOCK_TD)

  // span

  V(MD_SPAN_EM)
  V(MD_SPAN_STRONG)
  V(MD_SPAN_A)
  V(MD_SPAN_IMG)
  V(MD_SPAN_CODE)
  V(MD_SPAN_DEL)
  V(MD_SPAN_LATEXMATH)
  V(MD_SPAN_LATEXMATH_DISPLAY)
  V(MD_SPAN_WIKILINK)
  V(MD_SPAN_U)

  // text

  V(MD_TEXT_NORMAL)
  V(MD_TEXT_NULLCHAR)
  V(MD_TEXT_BR)
  V(MD_TEXT_SOFTBR)
  V(MD_TEXT_ENTITY)
  V(MD_TEXT_CODE)
  V(MD_TEXT_HTML)
  V(MD_TEXT_LATEXMATH)

  // flags

  V(MD_FLAG_COLLAPSEWHITESPACE)
  V(MD_FLAG_PERMISSIVEATXHEADERS)
  V(MD_FLAG_PERMISSIVEURLAUTOLINKS)
  V(MD_FLAG_PERMISSIVEEMAILAUTOLINKS)
  V(MD_FLAG_NOINDENTEDCODEBLOCKS)
  V(MD_FLAG_NOHTMLBLOCKS)
  V(MD_FLAG_NOHTMLSPANS)
  V(MD_FLAG_TABLES)
  V(MD_FLAG_STRIKETHROUGH)
  V(MD_FLAG_PERMISSIVEWWWAUTOLINKS)
  V(MD_FLAG_TASKLISTS)
  V(MD_FLAG_LATEXMATHSPANS)
  V(MD_FLAG_WIKILINKS)
  V(MD_FLAG_UNDERLINE)
  V(MD_FLAG_HARD_SOFT_BREAKS)
  V(MD_FLAG_PERMISSIVEAUTOLINKS)
  V(MD_FLAG_NOHTML)

  V(MD_DIALECT_COMMONMARK) // flag-combo
  V(MD_DIALECT_GITHUB) // flag-combo

  // alignment

  V(MD_ALIGN_DEFAULT)
  V(MD_ALIGN_LEFT)
  V(MD_ALIGN_CENTER)
  V(MD_ALIGN_RIGHT)

  // bare-md4c

  V(EV_BLOCK_ENTER)
  V(EV_BLOCK_LEAVE)
  V(EV_SPAN_ENTER)
  V(EV_SPAN_LEAVE)
  V(EV_TEXT)

  // md4c-html renderer

  V(MD_HTML_FLAG_DEBUG)
  V(MD_HTML_FLAG_VERBATIM_ENTITIES)
  V(MD_HTML_FLAG_SKIP_UTF8_BOM)
  V(MD_HTML_FLAG_XHTML)

#undef V
  return exports;
}

BARE_MODULE(bare_addon, bare_md4c_exports)
