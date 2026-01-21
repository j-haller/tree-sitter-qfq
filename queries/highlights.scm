; QFQ Syntax Highlighting Queries
; Based on CodeMirror QFQ mode by Elias Villiger

; Comments
(comment) @comment
(comment_content) @comment

; QFQ Base Keywords (top-level)
(base_keyword) @keyword

; QFQ Level Keywords (inside blocks)
(level_keyword) @keyword

; Level path identifiers (e.g., 10.10 in 10.10.sql)
(level_path
  (level_identifier) @tag)

; Bracket suffix (e.g., [2] in lbeg[2])
(bracket_suffix) @punctuation.bracket

; Block names (numeric identifiers)
(block_name) @number

; Block delimiters
(block_start "{" @punctuation.bracket)
(block_end) @punctuation.bracket

; QFQ Variables
(qfq_variable
  "{{" @punctuation.special
  "}}" @punctuation.special)

(variable_prefix) @operator

(variable_name) @variable

(variable_modifiers
  ":" @punctuation.delimiter)

(modifier_value) @string.special

; Strings
(single_quoted_string) @string
(double_quoted_string) @string

; Numbers
(number) @number

; Identifiers (SQL tables, columns, keywords - will be handled by injection)
(identifier) @variable

; Operators
(operator) @operator
"=" @operator

; Punctuation
(punctuation) @punctuation.delimiter
