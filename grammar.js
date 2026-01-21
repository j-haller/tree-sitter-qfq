/**
 * @file QFQ grammar for tree-sitter
 * @author Jan Haller <jan@haller.dev>
 * @license MIT
 *
 * Grammar rules derived from CodeMirror QFQ mode by Elias Villiger
 * SQL, HTML, and JavaScript are handled by separate parsers via injection
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

// QFQ Keywords from CodeMirror configuration
const QFQ_BASE_KEYWORDS = [
  'form', 'r', 'dbIndex', 'debugShowBodyText', 'sqlLog', 'sqlLogMode',
  'render', 'performanceReport'
];

const QFQ_LEVEL_KEYWORDS = [
  'fbeg', 'fend', 'fsep', 'fskipwrap', 'shead', 'stail', 'head', 'tail',
  'lbeg', 'lend', 'libeg', 'liend', 'rbeg', 'rbgd', 'rend', 'renr', 'rsep',
  'sql', 'twig', 'althead', 'altsql', 'content', 'fireIf', 'fireSubIf', 'function'
];

module.exports = grammar({
  name: "qfq",

  extras: $ => [/[ \t]/],


  rules: {
    source_file: $ => repeat($._line),

    _line: $ => choice(
      $.comment,
      $.base_assignment,
      $.level_assignment,
      $.block_start,
      $.block_end,
      $.value_line,
      $._newline
    ),

    _newline: $ => /\r?\n/,

    // Comments: lines starting with #
    comment: $ => seq(
      '#',
      optional($.comment_content),
      $._newline
    ),

    comment_content: $ => /[^\r\n]*/,

    // Base keyword assignments (top level): form = value
    base_assignment: $ => prec(5, seq(
      field('keyword', $.base_keyword),
      '=',
      field('value', optional($.value)),
      $._newline
    )),

    base_keyword: $ => choice(...QFQ_BASE_KEYWORDS),

    // Level keyword assignments: sql = ..., 10.sql = ...
    level_assignment: $ => prec(5, seq(
      optional(seq(
        field('level_path', $.level_path),
        '.'
      )),
      field('keyword', $.level_keyword),
      optional($.bracket_suffix),
      '=',
      field('value', optional($.value)),
      $._newline
    )),

    level_path: $ => prec.left(seq(
      $.level_identifier,
      repeat(seq('.', $.level_identifier))
    )),

    level_identifier: $ => /[\w-]+/,

    bracket_suffix: $ => seq('[', optional(/\d+/), ']'),

    level_keyword: $ => choice(...QFQ_LEVEL_KEYWORDS),

    // Block start: { on its own line (optionally with a numeric identifier)
    block_start: $ => seq(
      optional(field('name', $.block_name)),
      '{',
      $._newline
    ),

    // Block names are typically numeric identifiers
    block_name: $ => /\d+/,

    // Block end: }
    block_end: $ => '}',

    // Continuation lines (value content without assignment)
    value_line: $ => prec(-1, seq(
      $.value,
      $._newline
    )),

    // Value content on a single line
    value: $ => repeat1($._value_part),

    _value_part: $ => choice(
      $.qfq_variable,
      $.string,
      $.identifier,
      $.number,
      $.operator,
      $.punctuation
    ),

    // QFQ Variables: {{...}}
    qfq_variable: $ => seq(
      '{{',
      optional($.variable_prefix),
      $.variable_name,
      optional($.variable_modifiers),
      '}}'
    ),

    variable_prefix: $ => choice('&', '!'),

    variable_name: $ => /[\w._-]+/,

    variable_modifiers: $ => repeat1(seq(
      ':',
      optional($.modifier_value)
    )),

    modifier_value: $ => /[^:}]+/,

    // Strings
    string: $ => choice(
      $.single_quoted_string,
      $.double_quoted_string
    ),

    single_quoted_string: $ => seq(
      "'",
      repeat(choice(
        /[^'\\\r\n]+/,
        /\\./
      )),
      "'"
    ),

    double_quoted_string: $ => seq(
      '"',
      repeat(choice(
        /[^"\\\r\n]+/,
        /\\./
      )),
      '"'
    ),

    // Numbers
    number: $ => /-?\d+(\.\d+)?/,

    // Generic identifier
    identifier: $ => /[A-Za-z_][A-Za-z0-9_]*/,

    // Operators
    operator: $ => choice('<=', '>=', '<>', '!=', '<', '>', '=', '+', '-', '*', '/', '%', '|', '&'),

    // Punctuation
    punctuation: $ => choice(',', '.', '(', ')', ';', ':', '[', ']')
  }
});
