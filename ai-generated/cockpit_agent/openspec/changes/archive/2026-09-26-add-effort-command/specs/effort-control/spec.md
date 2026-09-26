## ADDED Requirements

### Requirement: Effort Mode Definition
The system SHALL define four effort modes: `none`, `low`, `high`, and `max`, where `none` disables reasoning entirely, `low` enables brief reasoning, `high` enables standard-depth reasoning, and `max` enables maximum-depth reasoning. The default effort mode SHALL be `high`.

#### Scenario: Default effort mode
- **WHEN** the system starts without an explicit effort setting
- **THEN** the current effort mode is `high`

#### Scenario: Effort mode mapping
- **WHEN** the system maps an effort mode to an LLM request payload
- **THEN** `none` omits `reasoning_effort` and sets `reasoning: false`; `low` sets `reasoning_effort: "low"`; `high` sets `reasoning_effort: "high"`; `max` sets `reasoning_effort: "max"`

### Requirement: Effort State on Agent
The `CockpitAgent` SHALL maintain an `effort` attribute representing the current effort mode. The agent SHALL pass this value to the LLM client's `chat` and `chat_stream` methods on every call.

#### Scenario: Agent passes effort to LLM client
- **WHEN** the agent calls `self.llm.chat_stream(messages, tools, temperature, effort)`
- **THEN** the effort parameter carries the current effort mode value

#### Scenario: Effort change takes effect on next turn
- **WHEN** the user changes effort mode via `/effort low` and then sends a message
- **THEN** the LLM request payload for that turn includes `reasoning_effort: "low"`

### Requirement: LLM Client Effort Parameter
Both `OpenAICompatibleLLM` and `MockLLMClient` SHALL accept an optional `effort` keyword argument in their `chat` and `chat_stream` methods. When `effort` is provided and is not `None`, the client SHALL include the corresponding `reasoning_effort` field in the request payload.

#### Scenario: OpenAICompatibleLLM includes reasoning_effort in payload
- **WHEN** `effort="high"` is passed to `OpenAICompatibleLLM.chat_stream`
- **THEN** the request payload includes `"reasoning_effort": "high"`

#### Scenario: Effort none disables reasoning
- **WHEN** `effort="none"` is passed to `OpenAICompatibleLLM.chat_stream`
- **THEN** the request payload includes `"reasoning": false` and does not include `reasoning_effort`

#### Scenario: Effort None means no reasoning field
- **WHEN** `effort` is `None` (not set) is passed to `OpenAICompatibleLLM.chat_stream`
- **THEN** the request payload does not include `reasoning_effort` or `reasoning` fields, leaving the API default behavior

#### Scenario: MockLLMClient accepts effort parameter
- **WHEN** `effort="low"` is passed to `MockLLMClient.chat_stream`
- **THEN** the mock client accepts the parameter without error and proceeds with its mock logic unchanged

### Requirement: Startup Effort Override
The system SHALL support a `--effort` command-line argument in `main.py` that overrides the default effort mode at startup.

#### Scenario: Start with low effort
- **WHEN** user runs `python main.py --interactive --effort low`
- **THEN** the REPL starts with effort mode set to `low`

#### Scenario: Invalid effort at startup
- **WHEN** user runs `python main.py --interactive --effort invalid`
- **THEN** the system prints an error message listing valid modes and exits

#### Scenario: No effort flag uses default
- **WHEN** user runs `python main.py --interactive` without `--effort`
- **THEN** the effort mode defaults to `high`
