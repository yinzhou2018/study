#include "repl.h"

#include <unistd.h>

#include <string>

#include "input_reader.h"

namespace calculator {

Repl::Repl() : out_{std::cout} {
  bool interactive = static_cast<bool>(isatty(fileno(stdin)));
  if (interactive) {
    reader_ = std::make_unique<LinenoiseReader>();
  } else {
    reader_ = std::make_unique<StdinReader>(std::cin);
  }
}

Repl::Repl(std::unique_ptr<InputReader> reader, std::ostream& out)
    : reader_{std::move(reader)}, out_{out} {}

void Repl::Run() {
  out_.get() << "Calculator v1.0\n";
  out_.get() << "Type 'quit' or 'exit' to quit.\n";

  std::string line;
  while (reader_->ReadLine(line)) {
    if (IsExitCommand(line))
      break;
    ProcessLine(line);
  }
}

void Repl::ProcessLine(const std::string& line) {
  auto result = evaluator::EvaluateExpression(line);
  if (!result.ok()) {
    out_.get() << "Error: " << result.error().message << "\n";
    return;
  }
  out_.get() << result.value() << "\n";
}

bool Repl::IsExitCommand(const std::string& line) const {
  std::string trimmed = line;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
  trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
  return trimmed == "quit" || trimmed == "exit";
}

}  // namespace calculator
