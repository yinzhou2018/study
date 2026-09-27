### Requirement: Multi-zone user message tagging
The system SHALL tag every user message with the originating audio zone and user identifier in the format `[zone=<zone_id>,user=<user_id>] <text>`. When no authenticated user is available, the system SHALL use `user=guest`.

#### Scenario: Driver sends a request
- **WHEN** the driver says “有点闷”
- **THEN** the system appends a user message beginning with `[zone=driver,user=guest] 有点闷`

#### Scenario: Passenger sends a request
- **WHEN** the front passenger says “把音量调大一点”
- **THEN** the system appends a user message beginning with `[zone=front_passenger,user=guest] 把音量调大一点`

### Requirement: Shared conversation history across zones
The system SHALL maintain one shared conversation history for all audio zones. The model SHALL use zone tags in that history to determine whether a later utterance continues an earlier topic, including when the later utterance originates from a different zone.

#### Scenario: Cross-zone follow-up
- **WHEN** the driver asks to open a window and the front passenger later says “再开大一点”
- **THEN** the model uses the shared history to infer that the passenger is referring to the window topic

#### Scenario: Independent topic
- **WHEN** a rear passenger starts a new topic unrelated to the previous conversation
- **THEN** the model treats it as a new request rather than forcing it to continue the previous topic

### Requirement: Model output routing prefix
The model SHALL prefix each assistant reply with either `[zone=<zone_id>]` or `[broadcast]`. The system SHALL parse this prefix to determine the target audio zone. If the model omits the prefix, the system SHALL route the reply to the originating zone.

#### Scenario: Reply to the requesting zone
- **WHEN** the model responds to a request from the driver
- **THEN** the reply begins with `[zone=driver]`

#### Scenario: Broadcast reply
- **WHEN** the model needs to inform all occupants
- **THEN** the reply begins with `[broadcast]`

#### Scenario: Missing routing prefix
- **WHEN** the model returns a reply without a routing prefix
- **THEN** the system routes the reply to the zone that originated the current request

### Requirement: Multi-zone system prompt contract
The system prompt SHALL define the multi-zone message format, shared-context behavior, cross-zone delegation, privacy constraints, and conflict priority rules for the model.

#### Scenario: Cross-zone delegation
- **WHEN** the front passenger says “帮我导航回家”
- **THEN** the model may act on the request on behalf of the passenger

#### Scenario: Privacy constraint
- **WHEN** a user asks the assistant to read a personal message
- **THEN** the model does not disclose that message to other audio zones
