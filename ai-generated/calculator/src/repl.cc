#include "repl.h"

#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <string>

#include "input_reader.h"

namespace calculator {

Repl::Repl(std::istream& in, std::ostream& out) : out_{out} {
  bool interactive = static_cast<bool>(isatty(fileno(stdin)));
  if (interactive) {
    reader_ = std::make_unique<LinenoiseReader>();
  } else {
    reader_ = std::make_unique<Stdin1Reader>(in);
  }
}

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
  std::string trimmed = line;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
  trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
  if (trimmed.empty())
    return;

  Lexer lexer(trimmed);
  auto tokens = lexer.Tokenize();
  if (!tokens.ok()) {
    out_.get() << "Error: " << tokens.error().message << "\n";
    return;
  }

  Parser parser(std::move(tokens.value()));
  auto expr = parser.Parse();
  if (!expr.ok()) {
    out_.get() << "Error: " << expr.error().message << "\n";
    return;
  }

  auto result = evaluator::Evaluate(expr.value());
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
