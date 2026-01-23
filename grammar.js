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
  'form',
  'r',
  'dbIndex',
  'debugShowBodyText',
  'sqlLog',
  'sqlLogMode',
  'render',
  'performanceReport'
];

const QFQ_RENDER_KEYWORDS = [
  'single',
  'both',
  'api'
]

const QFQ_OPENING_BRACES = [
  '{',
  '[',
  '(',
  '<'
]

const QFQ_CLOSING_BRACES = [
  '}',
  ']',
  ')',
  '>'
]

const QFQ_LEVEL_KEYWORDS = [
  'fbeg',
  'fend',
  'fsep',
  'fskipwrap',
  'shead',
  'stail',
  'head',
  'tail',
  'rbeg',
  'rbgd',
  'rend',
  'renr',
  'rsep',
  'lbeg',
  'lend',
  'libeg',
  'liend',
  'sql',
  'twig',
  'althead',
  'altsql',
  'content',
  'function'
];

const QFQ_STORES = [
  'F',
  'S',
  'R',
  'B',
  'C',
  'T',
  'V',
  'L',
  'Y',
  'U',
  'A',
  'W',
  'E',
  '0'
]

const QFQ_SANITIZE_CLASSES = [
  'alnumx',
  'digit',
  'numerical',
  'allbut',
  'all'
]

const QFQ_ESCAPE_ACTION_CLASSES = [
  'c',
  'C',
  'd',
  'f',
  'h',
  'H',
  'l',
  'L',
  'm',
  'p',
  'P',
  's',
  'S',
  't',
  'T',
  'w',
  'X',
  '',
  '-',
  'E',
  'D'
]

const QFQ_SQL_FUNCTIONS = [
  'QBAR',
  'QENT_ENCODE',
  'QENT_DECODE',
  'QCC',
  'QNL2BR',
  'QNBSP',
  'QLEFT',
  'QRIGHT',
  'QMORE',
  'QIFEMPTY',
  'QIFPREPEND',
  'QDATE_FORMAT',
  'QSLUGIFY',
  'QEND_SQUOTE',
  'QEND_SQUOTE',
  'QESC_SQUOTE',
  'QESC_DQUOTE',
  'QMANR',
  'strip_tags'
]

module.exports = grammar({
  name: "qfq",

  externals: $ => [
    $.expression_content,
    $.line_content,  // For level_expression - lines that don't start with keywords
  ],

  extras: $ => [
    /[ \t]/,
    $.comment,
  ],

  rules: {
    source_file: $ => repeat($._definition),

    _definition: $ => choice(
      $.base_statement,
      $.level_definition,
      $._newline
    ),

    base_statement: $ => seq(
      choice(...QFQ_BASE_KEYWORDS),
      '=',
      $.expression
    ),

    level_definition: $ => seq(
      optional($.identifier),
      $.block
    ),

    block: $ => seq(
      choice(...QFQ_OPENING_BRACES),
      repeat($._newline),
      repeat($._block_content),
      choice(...QFQ_CLOSING_BRACES),
    ),

    _block_content: $ => seq(
      choice(
        $.level_statement,
        $.level_expression,
        $.level_definition,
      ),
      repeat($._newline)
    ),

    level_statement: $ => seq(
      $.level_keyword,
      '=',
      $.expression
    ),

    level_keyword: $ => choice(...QFQ_LEVEL_KEYWORDS),

    level_expression: $ => $.line_content,

    // Expression can span multiple lines with comments interspersed
    expression: $ => repeat1($.expression_content),

    identifier: $ => /[a-zA-Z0-9]+/,

    _newline: $ => /\r?\n/,

    // Comments: lines starting with # (preceded only by whitespace)
    comment: $ => /#[^\r\n]*/
  }
});
