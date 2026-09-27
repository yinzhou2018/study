## ADDED Requirements

### Requirement: Requesting zone passed to tool execution
The system SHALL pass the originating audio zone to the tool execution layer as `requesting_zone`. The tool execution layer SHALL use `requesting_zone` for permission and safety checks independently of any target zone specified in tool arguments.

#### Scenario: Driver-initiated tool call
- **WHEN** the driver triggers a vehicle control tool
- **THEN** the tool execution layer receives `requesting_zone=driver`

#### Scenario: Passenger-initiated tool call
- **WHEN** the front passenger triggers a vehicle control tool
- **THEN** the tool execution layer receives `requesting_zone=front_passenger`

### Requirement: Zone-based permission enforcement
The tool execution layer SHALL enforce zone-specific permission and safety rules based on `requesting_zone`. The system SHALL NOT rely solely on the model or System Prompt to enforce these rules.

#### Scenario: Restricted tool from rear zone
- **WHEN** a rear passenger attempts to trigger a tool restricted from that zone
- **THEN** the tool execution layer rejects the call

#### Scenario: Allowed cross-zone delegation
- **WHEN** a passenger triggers a tool that is permitted for that zone
- **THEN** the tool execution layer executes the tool
