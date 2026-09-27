## MODIFIED Requirements

### Requirement: Effort Mode Definition
The system SHALL define four effort modes: `none`, `low`, `high`, and `max`, where `none` disables reasoning entirely, `low` enables brief reasoning, `high` enables standard-depth reasoning, and `max` enables maximum-depth reasoning. The default effort mode SHALL be `high`. The system SHALL map each effort mode to an LLM request payload according to the active provider's `build_effort_payload` function.

#### Scenario: Default effort mode
- **WHEN** the system starts without an explicit effort setting
- **THEN** the current effort mode is `high`

#### Scenario: Effort mode mapping for voyah
- **WHEN** the system maps an effort mode to an LLM request payload for the `voyah` provider
- **THEN** `none` sets `reasoning: false`; `low` sets `reasoning_effort: "low"`; `high` sets `reasoning_effort: "high"`; `max` sets `reasoning_effort: "max"`

#### Scenario: Effort mode mapping for deepseek
- **WHEN** the system maps an effort mode to an LLM request payload for the `deepseek` provider
- **THEN** `none` sets `enable_thinking: false`; `low` sets `enable_thinking: true` and `thinking_budget: 512`; `high` sets `enable_thinking: true` and `thinking_budget: 2048`; `max` sets `enable_thinking: true` and `thinking_budget: 4096`

#### Scenario: Effort mode mapping for default provider
- **WHEN** the system maps an effort mode with no provider specified
- **THEN** the system uses the `DEFAULT_PROVIDER_ID` provider's mapping

#### Scenario: Unknown provider falls back to default
- **WHEN** the system maps an effort mode for an unregistered provider id
- **THEN** the system uses the `DEFAULT_PROVIDER_ID` provider's mapping

### Requirement: Effort State on Agent
The `CockpitAgent` SHALL maintain an `effort` attribute representing the current effort mode. The agent SHALL pass this value to the LLM client's `chat` and `chat_stream` methods on every call, and the LLM client SHALL resolve the effort payload using its active provider id.

#### Scenario: Agent passes effort to LLM client
- **WHEN** the agent calls `self.llm.chat_stream(messages, tools, temperature, effort)`
- **THEN** the effort parameter carries the current effort mode value

#### Scenario: Effort change takes effect on next turn
- **WHEN** the user changes effort mode via `/effort low` and then sends a message
- **THEN** the LLM request payload for that turn reflects the active provider's `low` effort mapping

#### Scenario: Provider switch changes effort payload
- **WHEN** the agent switches from `voyah` to `deepseek` while effort remains `high`
- **THEN** the next LLM request payload uses the `deepseek` provider's `high` mapping

### Requirement: LLM Client Effort Parameter
Both `OpenAICompatibleLLM` and `MockLLMClient` SHALL accept an optional `effort` keyword argument in their `chat` and `chat_stream` methods. `OpenAICompatibleLLM` SHALL also accept an optional `provider_id` constructor argument and SHALL use it to select the effort payload mapping.

#### Scenario: OpenAICompatibleLLM uses provider payload mapping
- **WHEN** `effort="high"` is passed to `OpenAICompatibleLLM.chat` and the client is constructed with `provider_id="deepseek"`
- **THEN** the request payload includes `"enable_thinking": true` and `"thinking_budget": 2048`

#### Scenario: OpenAICompatibleLLM uses default provider when provider_id is None
- **WHEN** `effort="high"` is passed to `OpenAICompatibleLLM.chat_stream` and no `provider_id` is provided
- **THEN** the request payload includes `"reasoning_effort": "high"`

#### Scenario: Effort None means no reasoning field
- **WHEN** `effort` is `None` (not set) is passed to `OpenAICompatibleLLM.chat_stream`
- **THEN** the request payload does not include `reasoning_effort` or `reasoning` fields, leaving the API default behavior

#### Scenario: MockLLMClient accepts effort parameter
- **WHEN** `effort="low"` is passed to `MockLLMClient.chat_stream`
- **THEN** the mock client accepts the parameter without error and proceeds with its mock logic unchanged
