## 1. Multi-zone Context Model

- [x] 1.1 Add the multi-zone system prompt contract to `SYSTEM_PROMPT_TEMPLATE`.
- [x] 1.2 Define supported audio zones and the default `user=guest` identifier.
- [x] 1.3 Update `CockpitAgent.chat()` and `CockpitAgent.chat_stream()` to accept `zone_id` and `user_id`, and prefix user messages with `[zone=<zone>,user=<user>]`.
- [x] 1.4 Ensure all zones share the same `messages` history.

## 2. Zone-aware Tool Execution

- [x] 2.1 Pass `requesting_zone` from `CockpitAgent` into `ToolGateway.execute()`.
- [x] 2.2 Add zone-based permission rules to `ToolGateway`.
- [x] 2.3 Ensure tool safety checks still run independently of the model and System Prompt.

## 3. Output Routing and History Compression

- [x] 3.1 Parse assistant replies for `[zone=<zone>]` or `[broadcast]` prefixes.
- [x] 3.2 Route replies without a prefix back to the originating zone.
- [x] 3.3 Preserve `[zone=<zone>]` tags in the compressed history summary.

## 4. REPL Simulation

- [x] 4.1 Add a REPL command to select or switch the simulated audio zone.
- [x] 4.2 Prefix user input with the selected zone and `user=guest`.
- [x] 4.3 Display the parsed target zone for assistant replies.

## 5. Verification

- [x] 5.1 Add tests for user message tagging and shared history.
- [x] 5.2 Add tests for zone-aware tool permissions.
- [x] 5.3 Add tests for assistant output prefix parsing and fallback routing.
- [x] 5.4 Add tests for history compression preserving zone tags.
- [x] 5.5 Add REPL tests for zone switching and display.
- [x] 5.6 Run the full test suite.
