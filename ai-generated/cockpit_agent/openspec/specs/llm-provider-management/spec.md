### Requirement: Provider Profile Registry
The system SHALL maintain a registry of OpenAI-compatible provider profiles in `llm_config.py`. Each profile SHALL contain `base_url`, `api_key`, `model`, and `build_effort_payload`. The registry SHALL identify one `DEFAULT_PROVIDER_ID` used when no provider is explicitly selected.

#### Scenario: List configured providers
- **WHEN** the system queries the provider registry
- **THEN** it returns a non-empty mapping of provider id to profile, and the default provider id is one of the registered keys

#### Scenario: Provider profile fields
- **WHEN** the system reads a provider profile from the registry
- **THEN** the profile contains `base_url`, `api_key`, `model`, and `build_effort_payload`

#### Scenario: Unknown provider id rejected
- **WHEN** the system is asked to use a provider id not present in the registry
- **THEN** it raises a `ValueError` listing valid provider ids

### Requirement: Runtime Provider Switching on Agent
`CockpitAgent` SHALL hold a `provider_id` attribute representing the active provider. The agent SHALL provide a `set_provider(provider_id)` method that validates the id, updates the active provider, and reconfigures the underlying LLM client without discarding conversation history.

#### Scenario: Switch provider at runtime
- **WHEN** `agent.set_provider("deepseek")` is called with a valid provider id
- **THEN** `agent.provider_id` becomes `deepseek` and the agent's LLM client is reconfigured with the new provider's `base_url`, `api_key`, `model`, and `provider_id`

#### Scenario: Switch to invalid provider fails
- **WHEN** `agent.set_provider("nonexistent")` is called
- **THEN** a `ValueError` is raised and `agent.provider_id` remains unchanged

#### Scenario: Conversation history preserved after switch
- **WHEN** the provider is switched after several turns of conversation
- **THEN** the existing `agent.messages` list is unchanged and the next LLM call uses the newly configured provider

### Requirement: CLI Provider Command
The interactive REPL SHALL support a `/provider` command. Without arguments it SHALL print the current provider id and the list of available provider ids. With a valid argument it SHALL switch the active provider and print a confirmation including the new model. With an invalid argument it SHALL print an error and list available provider ids.

#### Scenario: Check current provider
- **WHEN** the user types `/provider` without arguments
- **THEN** the system prints the current provider id and all available provider ids

#### Scenario: Switch provider via command
- **WHEN** the user types `/provider deepseek`
- **THEN** the system switches the active provider, prints a confirmation, and the next LLM call uses the new provider

#### Scenario: Invalid provider via command
- **WHEN** the user types `/provider nonexistent`
- **THEN** the system prints an error message listing available provider ids and does not change the active provider

### Requirement: Startup Provider Selection
The system SHALL support a `--provider` command-line argument in `main.py` that selects the active provider at startup. The argument value SHALL be a key in the provider registry. When omitted, the system SHALL use `DEFAULT_PROVIDER_ID`.

#### Scenario: Start with explicit provider
- **WHEN** the user runs `python main.py --interactive --provider deepseek`
- **THEN** the agent starts with `provider_id` set to `deepseek` and the LLM client configured from the deepseek profile

#### Scenario: Default provider when omitted
- **WHEN** the user runs `python main.py --interactive` without `--provider`
- **THEN** the agent starts with `provider_id` equal to `DEFAULT_PROVIDER_ID`

#### Scenario: Invalid provider at startup
- **WHEN** the user runs `python main.py --interactive --provider nonexistent`
- **THEN** the system prints an error listing valid provider ids and exits
