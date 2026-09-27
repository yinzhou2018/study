
### Requirement: Interactive REPL Entry Point
The system SHALL provide a command-line interactive REPL entry point accessible via `python main.py --interactive` (or `-i`), allowing users to engage in continuous multi-turn conversations with the agent.

#### Scenario: Start interactive mode
- **WHEN** user runs `python main.py --interactive`
- **THEN** the system displays a welcome banner listing available commands including `/effort` and a prompt, and waits for user input

#### Scenario: Start without arguments defaults to batch mode
- **WHEN** user runs `python main.py` without `--interactive`
- **THEN** the system runs the existing batch demo mode as before

#### Scenario: Exit via command
- **WHEN** user types `/exit` at the prompt
- **THEN** the system prints a farewell message and terminates the REPL

### Requirement: Streaming LLM Output
The system SHALL stream LLM responses token-by-token to stdout in real time, so the user can observe the thinking process and final reply as they are generated.

#### Scenario: Stream final reply tokens
- **WHEN** the LLM generates a text reply (no tool calls)
- **THEN** each token is printed to stdout immediately as received, prefixed with a label such as `[回复中]`

#### Scenario: Stream thinking content
- **WHEN** the LLM generates intermediate reasoning content before tool calls
- **THEN** the reasoning content is streamed token-by-token with a `[思考中]` prefix

#### Scenario: Mock client streaming
- **WHEN** using `MockLLMClient` in streaming mode
- **THEN** the mock client yields tokens with small delays to simulate real streaming behavior

### Requirement: Real-time Tool Call Display
The system SHALL display each tool call and its result in real time during the agent's tool-call loop, showing the tool name, arguments, execution status, and return value.

#### Scenario: Display tool call with arguments
- **WHEN** the LLM returns a tool call
- **THEN** the system prints the tool name and arguments in a formatted block with `[工具调用]` prefix

#### Scenario: Display tool result
- **WHEN** a tool execution completes
- **THEN** the system prints the tool name and result summary with `[工具结果]` prefix

#### Scenario: Multiple sequential tool calls
- **WHEN** the LLM returns multiple tool calls across multiple turns
- **THEN** each tool call and result is displayed in sequence as they execute

### Requirement: Interrupt Support
The system SHALL support Esc key interruption during LLM streaming or tool execution, safely returning to the prompt without crashing.

#### Scenario: Interrupt during LLM streaming
- **WHEN** user presses the Esc key while LLM tokens are being streamed
- **THEN** the system stops streaming, prints `[已打断]`, retains any completed tool call results in message history, and returns to the prompt

#### Scenario: Interrupt during tool execution
- **WHEN** user presses the Esc key while a tool is executing
- **THEN** the system stops the current operation, prints `[已打断]`, and returns to the prompt

#### Scenario: Interrupt preserves completed work
- **WHEN** an interrupt occurs after some tool calls have completed
- **THEN** the completed tool call results remain in the message history and are available for the next conversation turn

#### Scenario: Interrupt at the prompt does nothing
- **WHEN** user presses the Esc key while at the idle prompt (not during generation)
- **THEN** the system clears the current input line and re-displays the prompt without exiting

### Requirement: Session Commands
The system SHALL support slash commands for session management: `/exit`, `/clear`, `/history`, and `/effort`.

#### Scenario: Clear conversation history
- **WHEN** user types `/clear` at the prompt
- **THEN** the system resets the message history to the initial system prompt, prints a confirmation, and returns to the prompt

#### Scenario: View conversation history
- **WHEN** user types `/history` at the prompt
- **THEN** the system prints all messages in the conversation history with their role and a truncated content preview

#### Scenario: Check current effort mode
- **WHEN** user types `/effort` without arguments at the prompt
- **THEN** the system prints the current effort mode and returns to the prompt

#### Scenario: Switch effort mode
- **WHEN** user types `/effort <mode>` where mode is one of `none`, `low`, `high`, `max`
- **THEN** the system sets the agent's effort to the specified mode, prints a confirmation message, and returns to the prompt

#### Scenario: Invalid effort mode
- **WHEN** user types `/effort invalid` at the prompt
- **THEN** the system prints an error message listing valid modes (`none`, `low`, `high`, `max`) and returns to the prompt

#### Scenario: Unknown command
- **WHEN** user types an unrecognized `/command`
- **THEN** the system prints an error message listing available commands and returns to the prompt

### Requirement: Backward Compatibility
The system SHALL preserve the existing `CockpitAgent.chat()` method signature and behavior, ensuring the batch demo mode continues to work unchanged.

#### Scenario: Batch mode unchanged
- **WHEN** user runs `python main.py` without `--interactive`
- **THEN** the existing batch demo runs identically to before, with no behavioral changes

#### Scenario: chat() method still works
- **WHEN** `CockpitAgent.chat(user_query, verbose=True)` is called directly
- **THEN** it returns the final reply string as before, with verbose output to stdout

### Requirement: Multi-zone simulation in REPL
The interactive REPL SHALL allow the user to select or switch the simulated audio zone for subsequent input. The REPL SHALL automatically prefix user input with the selected zone and `user=guest`, and SHALL display the target zone parsed from assistant replies.

#### Scenario: Switch simulated zone
- **WHEN** the user switches the simulated zone to `front_passenger`
- **THEN** subsequent user messages are prefixed with `[zone=front_passenger,user=guest]`

#### Scenario: Display routed reply
- **WHEN** the assistant returns a reply prefixed with `[zone=driver]`
- **THEN** the REPL displays the reply as targeted to the driver zone
