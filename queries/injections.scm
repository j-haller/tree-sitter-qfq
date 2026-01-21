; QFQ Language Injection Queries
; Injects SQL, HTML, and JavaScript/Twig based on the assignment keyword

; SQL injection for sql = and altsql = assignments
((level_assignment
  keyword: (level_keyword) @_kw
  value: (value) @injection.content)
 (#any-of? @_kw "sql" "altsql")
 (#set! injection.language "sql"))

; HTML injection for HTML-related keywords
; shead, stail = table/section headers and footers
; head, tail = general headers and footers
; rbeg, rend, renr = row begin/end
; fbeg, fend = field begin/end
; lbeg, lend = list begin/end
; libeg, liend = list item begin/end
; althead = alternative header
; content = general content
((level_assignment
  keyword: (level_keyword) @_kw
  value: (value) @injection.content)
 (#any-of? @_kw "shead" "stail" "head" "tail" "rbeg" "rbgd" "rend" "renr" "fbeg" "fend" "fsep" "lbeg" "lend" "libeg" "liend" "althead" "content")
 (#set! injection.language "html"))

; Twig/JavaScript injection for twig = assignments
((level_assignment
  keyword: (level_keyword) @_kw
  value: (value) @injection.content)
 (#eq? @_kw "twig")
 (#set! injection.language "twig"))

; Note: Inline SQL in QFQ variables (like {{SELECT ... AS _var}}) requires
; more complex handling and is not currently supported by this injection config.
; The variable syntax is parsed as variable_name:modifiers which doesn't preserve
; the SQL structure needed for proper injection.
