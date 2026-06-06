#pragma once

#include <istream>
#include <string>

namespace calculator {

class InputReader {
 public:
  virtual ~InputReader() = default;

  virtual bool ReadLine(std::string& line) = 0;
};

class StdinReader : public InputReader {
 public:
  explicit StdinReader(std::istream& in);

  bool ReadLine(std::string& line) override;

 private:
  std::istream& in_;
};

class LinenoiseReader : public InputReader {
 public:
  LinenoiseReader();

  ~LinenoiseReader() override;

  bool ReadLine(std::string& line) override;

 private:
  std::string history_path_;
};

}  // namespace calculator
