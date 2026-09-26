## MODIFIED Requirements

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
