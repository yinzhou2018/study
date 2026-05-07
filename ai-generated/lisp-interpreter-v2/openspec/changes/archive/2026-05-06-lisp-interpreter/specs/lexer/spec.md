## ADDED Requirements

### Requirement: Tokenize atomic types
The lexer SHALL recognize and emit tokens for: LPAREN `(`, RPAREN `)`, QUOTE `'`, NUMBER (integer and floating-point), STRING (double-quoted with escape sequences), SYMBOL (identifier), BOOLEAN (`#t` and `#f`).

#### Scenario: Tokenize a number
- **WHEN** the lexer reads `42`
- **THEN** it SHALL emit a NUMBER token with value 42

#### Scenario: Tokenize a floating-point number
- **WHEN** the lexer reads `3.14`
- **THEN** it SHALL emit a NUMBER token with value 3.14

#### Scenario: Tokenize a string
- **WHEN** the lexer reads `"hello world"`
- **THEN** it SHALL emit a STRING token with value "hello world"

#### Scenario: Tokenize a boolean
- **WHEN** the lexer reads `#t`
- **THEN** it SHALL emit a BOOLEAN token with value true

#### Scenario: Tokenize a symbol
- **WHEN** the lexer reads `foo-bar`
- **THEN** it SHALL emit a SYMBOL token with value "foo-bar"

### Requirement: Handle whitespace and comments
The lexer SHALL skip whitespace and line comments starting with `;`.

#### Scenario: Skip line comment
- **WHEN** the lexer reads `; this is a comment\n42`
- **THEN** it SHALL skip the comment line and emit a NUMBER token with value 42

### Requirement: Track token locations
The lexer SHALL track line and column numbers for each token to support error reporting.

#### Scenario: Report location on error
- **WHEN** an unrecognized character appears at line 3, column 5
- **THEN** the lexer SHALL include location (3:5) in the error message

### Requirement: Handle quote shorthand
The lexer SHALL emit a QUOTE token for the `'` character.

#### Scenario: Tokenize quote shorthand
- **WHEN** the lexer reads `'(1 2 3)`
- **THEN** it SHALL emit tokens: QUOTE, LPAREN, NUMBER(1), NUMBER(2), NUMBER(3), RPAREN
