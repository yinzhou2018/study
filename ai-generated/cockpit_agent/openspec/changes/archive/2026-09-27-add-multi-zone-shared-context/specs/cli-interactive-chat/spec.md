## ADDED Requirements

### Requirement: Multi-zone simulation in REPL
The interactive REPL SHALL allow the user to select or switch the simulated audio zone for subsequent input. The REPL SHALL automatically prefix user input with the selected zone and `user=guest`, and SHALL display the target zone parsed from assistant replies.

#### Scenario: Switch simulated zone
- **WHEN** the user switches the simulated zone to `front_passenger`
- **THEN** subsequent user messages are prefixed with `[zone=front_passenger,user=guest]`

#### Scenario: Display routed reply
- **WHEN** the assistant returns a reply prefixed with `[zone=driver]`
- **THEN** the REPL displays the reply as targeted to the driver zone
