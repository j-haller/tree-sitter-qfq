package tree_sitter_qfq_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_qfq "github.com/j-halla/tree-sitter-qfq/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_qfq.Language())
	if language == nil {
		t.Errorf("Error loading QFQ grammar")
	}
}
