#include "input_reader.h"

#include <cstdlib>
#include <filesystem>
#include <string>

#include <linenoise.h>

namespace calculator {

StdinReader::StdinReader(std::istream& in) : in_{in} {}

bool StdinReader::ReadLine(std::string& line) {
  return static_cast<bool>(std::getline(in_, line));
}

LinenoiseReader::LinenoiseReader() {
  const char* home = std::getenv("HOME");
  if (home) {
    history_path_ = std::filesystem::path(home) / ".calculator_history";
    linenoiseHistoryLoad(history_path_.c_str());
  }
  linenoiseSetMultiLine(0);
}

LinenoiseReader::~LinenoiseReader() {
  if (!history_path_.empty()) {
    linenoiseHistorySave(history_path_.c_str());
  }
}

bool LinenoiseReader::ReadLine(std::string& line) {
  char* raw = linenoise("> ");
  if (!raw) {
    return false;
  }
  line = raw;
  linenoiseFree(raw);
  if (!line.empty()) {
    linenoiseHistoryAdd(line.c_str());
  }
  return true;
}

}  // namespace calculator
