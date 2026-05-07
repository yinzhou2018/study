## ADDED Requirements

### Requirement: Parse S-expressions into AST
The parser SHALL convert a token stream into an AST using recursive list structure. Atoms (numbers, strings, booleans, symbols) SHALL be leaf nodes. Lists SHALL be represented as Pair chains (cons cells).

#### Scenario: Parse a number atom
- **WHEN** the parser receives a NUMBER token with value 42
- **THEN** it SHALL produce an AST node: Number(42)

#### Scenario: Parse a symbol atom
- **WHEN** the parser receives a SYMBOL token with value "foo"
- **THEN** it SHALL produce an AST node: Symbol("foo")

#### Scenario: Parse a simple list
- **WHEN** the parser receives tokens LPAREN, NUMBER(1), NUMBER(2), RPAREN
- **THEN** it SHALL produce an AST: Pair(Number(1), Pair(Number(2), Nil))

#### Scenario: Parse a nested list
- **WHEN** the parser receives tokens for `(+ 1 (* 2 3))`
- **THEN** it SHALL produce a nested Pair structure representing the nested S-expression

### Requirement: Parse quote shorthand
The parser SHALL expand `'expr` into `(quote expr)`.

#### Scenario: Expand quote on atom
- **WHEN** the parser receives QUOTE, NUMBER(42)
- **THEN** it SHALL produce Pair(Symbol("quote"), Pair(Number(42), Nil))

#### Scenario: Expand quote on list
- **WHEN** the parser receives QUOTE, LPAREN, NUMBER(1), NUMBER(2), RPAREN
- **THEN** it SHALL produce Pair(Symbol("quote"), Pair(Pair(Number(1), Pair(Number(2), Nil)), Nil))

### Requirement: Report syntax errors with location
The parser SHALL report syntax errors with line and column information.

#### Scenario: Report mismatched parenthesis
- **WHEN** the parser encounters `(+ 1 2` (missing closing paren)
- **THEN** it SHALL report a syntax error indicating unexpected end of input

#### Scenario: Report unexpected token
- **WHEN** the parser encounters `) 42` (unexpected closing paren)
- **THEN** it SHALL report a syntax error with the token's location
