/**
 * @file Qfq grammar for tree-sitter
 * @author Jan Haller <jan@haller.dev>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

module.exports = grammar({
  name: "qfq",

  rules: {
    // TODO: add the actual grammar rules
    source_file: $ => "hello"
  }
});
