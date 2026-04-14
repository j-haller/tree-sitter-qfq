# tree-sitter-qfq

> **Work in Progress** — This grammar is under active development and may be incomplete or unstable.

A [tree-sitter](https://tree-sitter.github.io/tree-sitter/) grammar for [QFQ](https://docs.qfq.io/) (Quick Form Query), a language for building web forms and reports.

Grammar rules are derived from the CodeMirror QFQ mode by Elias Villiger. SQL, HTML, and JavaScript embedded in QFQ reports are handled by separate parsers via tree-sitter language injection.

## File types

- `.qfqr` — QFQ report files

## Features

- Parses QFQ report structure: base statements, level definitions, and blocks
- Recognizes base keywords (`form`, `r`, `dbIndex`, `render`, etc.)
- Recognizes level keywords (`sql`, `head`, `tail`, `twig`, `content`, etc.)
- Comments (`#`)
- Multi-line expression support via external scanner
- Language injection for embedded SQL, HTML, and JavaScript

## Bindings

| Language | Available |
|----------|-----------|
| C        | yes       |
| Go       | yes       |
| Node.js  | yes       |
| Python   | yes       |
| Rust     | yes       |
| Swift    | yes       |

## Usage

### Node.js

```js
const Parser = require('tree-sitter');
const QFQ = require('tree-sitter-qfq');

const parser = new Parser();
parser.setLanguage(QFQ);

const tree = parser.parse('form = MyForm\nr = 10');
console.log(tree.rootNode.toString());
```

### Python

```python
import tree_sitter_qfq as tsqfq
from tree_sitter import Language, Parser

QFQ_LANGUAGE = Language(tsqfq.language())
parser = Parser(QFQ_LANGUAGE)

tree = parser.parse(b'form = MyForm\nr = 10')
print(tree.root_node)
```

## Development

### Prerequisites

- [Node.js](https://nodejs.org/)
- [tree-sitter CLI](https://tree-sitter.github.io/tree-sitter/creating-parsers#installation)

### Setup

```sh
npm install
```

### Generate parser

```sh
tree-sitter generate
```

### Run tests

```sh
tree-sitter test
```

### Interactive playground

```sh
npm start
```

## License

MIT — Jan Haller &lt;jan@haller.dev&gt;
