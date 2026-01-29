/**
 * External scanner for QFQ tree-sitter grammar
 *
 * Handles multi-line expressions that can span multiple lines and include
 * comments as separate entities. Expressions end when a new statement,
 * block, or block-end is detected.
 */

#include "tree_sitter/parser.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

enum TokenType {
    EXPRESSION_CONTENT,
    LINE_CONTENT,  // For level_expression - content that doesn't start with a keyword
};

// Base keywords that can start a new base_statement
static const char* BASE_KEYWORDS[] = {
    "form",
    "r",
    "dbIndex",
    "debugShowBodyText",
    "sqlLog",
    "sqlLogMode",
    "render",
    "performanceReport",
    NULL
};

// Level keywords that can start a new level_statement
static const char* LEVEL_KEYWORDS[] = {
    "fbeg",
    "fend",
    "fsep",
    "fskipwrap",
    "shead",
    "stail",
    "head",
    "tail",
    "rbeg",
    "rbgd",
    "rend",
    "renr",
    "rsep",
    "lbeg",
    "lend",
    "libeg",
    "liend",
    "sql",
    "twig",
    "althead",
    "altsql",
    "content",
    "function",
    NULL
};

/**
 * Check if a character is an opening brace
 */
static bool is_opening_brace(int32_t c) {
    return c == '{' || c == '[' || c == '(' || c == '<';
}

/**
 * Check if a character is a closing brace
 */
static bool is_closing_brace(int32_t c) {
    return c == '}' || c == ']' || c == ')' || c == '>';
}

/**
 * Check if a character is alphanumeric (for identifiers)
 */
static bool is_alnum(int32_t c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

/**
 * Skip whitespace (spaces and tabs only)
 */
static void skip_whitespace(TSLexer *lexer) {
    while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
        lexer->advance(lexer, false);
    }
}

/**
 * Check if the upcoming text matches a keyword followed by optional whitespace and '='
 * NOTE: This advances the lexer - only use when you're ready to consume
 */
static bool check_keyword(TSLexer *lexer, const char* keyword) {
    int len = strlen(keyword);

    // Check each character of the keyword
    for (int i = 0; i < len; i++) {
        if (lexer->lookahead != keyword[i]) {
            return false;
        }
        lexer->advance(lexer, false);
    }

    // After keyword, must not be followed by alphanumeric (word boundary)
    if (is_alnum(lexer->lookahead)) {
        return false;
    }

    // Skip optional whitespace
    skip_whitespace(lexer);

    // Must be followed by '='
    return lexer->lookahead == '=';
}

/**
 * Check if we're at a block start: optional identifier followed by opening brace and newline
 */
static bool check_block_start(TSLexer *lexer) {
    // Skip optional identifier
    while (is_alnum(lexer->lookahead)) {
        lexer->advance(lexer, false);
    }

    // Skip optional whitespace between identifier and brace
    skip_whitespace(lexer);

    // Check for opening brace
    if (!is_opening_brace(lexer->lookahead)) {
        return false;
    }
    lexer->advance(lexer, false);

    // Skip optional whitespace after brace
    skip_whitespace(lexer);

    // Must be followed by newline (or EOF)
    return lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer);
}

/**
 * Check if we're at a block end: closing brace followed by newline
 */
static bool check_block_end(TSLexer *lexer) {
    // Check for closing brace
    if (!is_closing_brace(lexer->lookahead)) {
        return false;
    }
    lexer->advance(lexer, false);

    // Skip optional whitespace after brace
    skip_whitespace(lexer);

    // Must be followed by newline (or EOF)
    return lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer);
}

/**
 * Check if a word matches any keyword in a list
 */
static bool is_keyword(const char* word, const char** keywords) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Check if we're at a line boundary that terminates an expression
 * This is called when we're at the start of a line (after newline was consumed)
 *
 * IMPORTANT: mark_end should be called BEFORE this function.
 * This function advances the lexer for lookahead.
 */
static bool is_expression_boundary(TSLexer *lexer) {
    // Skip leading whitespace
    skip_whitespace(lexer);

    // Check for comment start
    if (lexer->lookahead == '#') {
        return true;
    }

    // Check for empty line or EOF
    if (lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer)) {
        return false;  // Empty line is not a boundary, continue expression
    }

    // Check for closing brace followed by newline
    if (is_closing_brace(lexer->lookahead)) {
        return check_block_end(lexer);
    }

    // Check for opening brace (block start without identifier)
    if (is_opening_brace(lexer->lookahead)) {
        return check_block_start(lexer);
    }

    // If it starts with alphanumeric, it could be:
    // 1. An identifier followed by a block (e.g., "foo {")
    // 2. A keyword followed by = (e.g., "sql =")
    // 3. Regular content
    if (is_alnum(lexer->lookahead)) {
        // Read the word
        char word[64];
        int word_len = 0;
        while (is_alnum(lexer->lookahead) && word_len < 63) {
            word[word_len++] = (char)lexer->lookahead;
            lexer->advance(lexer, false);
        }
        word[word_len] = '\0';

        // Skip whitespace after word
        skip_whitespace(lexer);

        // Check if it's a block start (word + opening brace + newline)
        if (is_opening_brace(lexer->lookahead)) {
            lexer->advance(lexer, false);  // consume the brace
            skip_whitespace(lexer);
            if (lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer)) {
                return true;  // It's a block start
            }
            // Brace not followed by newline - not a block boundary
            return false;
        }

        // Check if it's a keyword followed by =
        if (lexer->lookahead == '=') {
            if (is_keyword(word, BASE_KEYWORDS) || is_keyword(word, LEVEL_KEYWORDS)) {
                return true;  // It's a keyword statement
            }
        }

        // Neither block start nor keyword
        return false;
    }

    return false;
}

/**
 * Check if the current line starts with a keyword (for LINE_CONTENT)
 * Returns true if the line does NOT start with a keyword (i.e., is valid line content)
 */
static bool scan_line_content(TSLexer *lexer) {
    // Skip leading whitespace
    while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
        lexer->advance(lexer, false);
    }

    // Mark after whitespace - this is our starting position for line content
    lexer->mark_end(lexer);

    // Check for comment, newline, EOF - not line content
    if (lexer->lookahead == '#' ||
        lexer->lookahead == '\n' ||
        lexer->lookahead == '\r' ||
        lexer->eof(lexer)) {
        return false;
    }

    // Check for opening brace - might be block start
    if (is_opening_brace(lexer->lookahead)) {
        return false;  // Let the grammar handle this as a potential block
    }

    // Check for closing brace - might be block end
    if (is_closing_brace(lexer->lookahead)) {
        return false;  // Let the grammar handle this as a potential block end
    }

    // Read the first word (potential keyword)
    char word[64];
    int word_len = 0;
    while (is_alnum(lexer->lookahead) && word_len < 63) {
        word[word_len++] = (char)lexer->lookahead;
        lexer->advance(lexer, false);
    }
    word[word_len] = '\0';

    // If we read a word, check if it's a keyword followed by '='
    if (word_len > 0) {
        // Skip whitespace after the word
        while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
            lexer->advance(lexer, false);
        }

        // If followed by '=', check if it's a keyword
        if (lexer->lookahead == '=') {
            if (is_keyword(word, BASE_KEYWORDS) || is_keyword(word, LEVEL_KEYWORDS)) {
                return false;  // It's a keyword statement, not line content
            }
        }
    }

    // Not a keyword statement - this is line content
    // Mark current position (includes the word and whitespace we read)
    lexer->mark_end(lexer);

    // Continue reading to end of line
    while (lexer->lookahead != '\n' &&
        lexer->lookahead != '\r' &&
        !lexer->eof(lexer)) {
        lexer->advance(lexer, false);
        lexer->mark_end(lexer);
    }

    return true;
}

void *tree_sitter_qfq_external_scanner_create() {
    return NULL;
}

void tree_sitter_qfq_external_scanner_destroy(void *payload) {
    // Nothing to free
}

void tree_sitter_qfq_external_scanner_reset(void *payload) {
    // No state to reset
}

unsigned tree_sitter_qfq_external_scanner_serialize(void *payload, char *buffer) {
    return 0;  // No state to serialize
}

void tree_sitter_qfq_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
    // No state to deserialize
}

bool tree_sitter_qfq_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
    bool at_line_start = (lexer->get_column(lexer) == 0);

    // If we're at the start of a line, check what kind of content this is
    if (at_line_start) {
        // Mark current position as safe return point
        lexer->mark_end(lexer);

        // Skip leading whitespace
        while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
            lexer->advance(lexer, false);
        }

        // Check for comment - neither LINE_CONTENT nor EXPRESSION_CONTENT
        if (lexer->lookahead == '#') {
            return false;
        }

        // Check for newline or EOF - not content
        if (lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer)) {
            return false;
        }

        // Check for braces - not line content, might be block
        if (is_opening_brace(lexer->lookahead) || is_closing_brace(lexer->lookahead)) {
            // For LINE_CONTENT, definitely not
            // For EXPRESSION_CONTENT, also return false to let grammar handle blocks
            return false;
        }

        // Read first word to check if it's a keyword
        char word[64];
        int word_len = 0;
        while (is_alnum(lexer->lookahead) && word_len < 63) {
            word[word_len++] = (char)lexer->lookahead;
            lexer->advance(lexer, false);
        }
        word[word_len] = '\0';

        // Skip whitespace after word
        while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
            lexer->advance(lexer, false);
        }

        // Check if it's a keyword followed by '='
        bool is_kw_statement = false;
        if (lexer->lookahead == '=' && word_len > 0) {
            if (is_keyword(word, BASE_KEYWORDS) || is_keyword(word, LEVEL_KEYWORDS)) {
                is_kw_statement = true;
            }
        }

        // Check if it's a block start (word + opening brace + newline)
        bool is_block_start = false;
        if (is_opening_brace(lexer->lookahead)) {
            lexer->advance(lexer, false);  // consume brace
            while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
                lexer->advance(lexer, false);
            }
            if (lexer->lookahead == '\n' || lexer->lookahead == '\r' || lexer->eof(lexer)) {
                is_block_start = true;
            }
        }

        if (is_kw_statement || is_block_start) {
            // This line starts a keyword statement or block
            // Neither LINE_CONTENT nor EXPRESSION_CONTENT should match
            return false;
        }

        // It's regular line content - mark position and continue reading
        lexer->mark_end(lexer);

        // For LINE_CONTENT, read to end of line and return
        if (valid_symbols[LINE_CONTENT]) {
            while (lexer->lookahead != '\n' &&
                lexer->lookahead != '\r' &&
                !lexer->eof(lexer)) {
                lexer->advance(lexer, false);
                lexer->mark_end(lexer);
            }
            lexer->result_symbol = LINE_CONTENT;
            return true;
        }

        // For EXPRESSION_CONTENT, we'll continue to the main loop
        // The lexer is positioned after the first word/content on this line
    }

    if (!valid_symbols[EXPRESSION_CONTENT]) {
        return false;
    }

    // Track if we've consumed any content
    bool has_content = at_line_start;  // We've already consumed content if we came from the line start check

    while (!lexer->eof(lexer)) {
        int32_t c = lexer->lookahead;

        // Handle newlines
        if (c == '\n' || c == '\r') {
            // Consume the newline
            lexer->advance(lexer, false);
            has_content = true;

            // Handle \r\n
            if (c == '\r' && lexer->lookahead == '\n') {
                lexer->advance(lexer, false);
            }

            // Mark end AFTER the newline - this is our safe return point
            lexer->mark_end(lexer);

            // Check if next line is a boundary
            if (is_expression_boundary(lexer)) {
                // Return with the position marked after the newline
                lexer->result_symbol = EXPRESSION_CONTENT;
                return has_content;
            }

            // Not a boundary - continue reading
            // Mark end to include what we've checked (whitespace etc)
            lexer->mark_end(lexer);
            continue;
        }

        // Regular character - consume it
        lexer->advance(lexer, false);
        has_content = true;
        lexer->mark_end(lexer);
    }

    // EOF reached
    if (has_content) {
        lexer->result_symbol = EXPRESSION_CONTENT;
        return true;
    }

    return false;
}
