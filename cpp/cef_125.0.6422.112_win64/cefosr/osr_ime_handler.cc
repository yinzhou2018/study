#include "osr_ime_handler.h"
#include "osr_utils.h"
#include "osr_window.h"

#include <msctf.h>
#include <windowsx.h>
#include <iostream>

#pragma comment(lib, "Imm32.lib")

#define ColorUNDERLINE \
  0xFF000000  // Black SkColor value for underline,
              // same as Blink.
#define ColorBKCOLOR \
  0x00000000  // White SkColor value for background,
              // same as Blink.

namespace {

// Determines whether or not the given attribute represents a selection
bool IsSelectionAttribute(char attribute) {
  return (attribute == ATTR_TARGET_CONVERTED || attribute == ATTR_TARGET_NOTCONVERTED);
}

// Helper function for OsrImeHandler::GetCompositionInfo() method,
// to get the target range that's selected by the user in the current
// composition string.
void GetCompositionSelectionRange(HIMC imc, uint32_t* target_start, uint32_t* target_end) {
  uint32_t attribute_size = ::ImmGetCompositionString(imc, GCS_COMPATTR, nullptr, 0);
  if (attribute_size > 0) {
    uint32_t start = 0;
    uint32_t end = 0;
    std::vector<char> attribute_data(attribute_size);

    ::ImmGetCompositionString(imc, GCS_COMPATTR, &attribute_data[0], attribute_size);
    for (start = 0; start < attribute_size; ++start) {
      if (IsSelectionAttribute(attribute_data[start])) {
        break;
      }
    }
    for (end = start; end < attribute_size; ++end) {
      if (!IsSelectionAttribute(attribute_data[end])) {
        break;
      }
    }

    *target_start = start;
    *target_end = end;
  }
}

// Helper function for OsrImeHandler::GetCompositionInfo() method, to get
// underlines information of the current composition string.
void GetCompositionUnderlines(HIMC imc,
                              uint32_t target_start,
                              uint32_t target_end,
                              std::vector<CefCompositionUnderline>& underlines) {
  int clause_size = ::ImmGetCompositionString(imc, GCS_COMPCLAUSE, nullptr, 0);
  int clause_length = clause_size / sizeof(uint32_t);
  if (clause_length) {
    std::vector<uint32_t> clause_data(clause_length);

    ::ImmGetCompositionString(imc, GCS_COMPCLAUSE, &clause_data[0], clause_size);
    for (int i = 0; i < clause_length - 1; ++i) {
      cef_composition_underline_t underline = {};
      underline.range.from = clause_data[i];
      underline.range.to = clause_data[i + 1];
      underline.color = ColorUNDERLINE;
      underline.background_color = ColorBKCOLOR;
      underline.thick = 0;

      // Use thick underline for the target clause.
      if (underline.range.from >= target_start && underline.range.to <= target_end) {
        underline.thick = 1;
      }
      underlines.push_back(underline);
    }
  }
}

}  // namespace

LRESULT OsrImeHandler::OnIMESetContext(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  if (!wParam) {
    IMECancelComposition();
  } else {
    // We handle the IME Composition Window ourselves (but let the IME Candidates
    // Window be handled by IME through DefWindowProc()), so clear thed
    // ISC_SHOWUICOMPOSITIONWINDOW flag:
    lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
    ::DefWindowProc(hwnd_, msg, wParam, lParam);

    MoveImeWindow();
  }

  handled = true;
  return 0;
}

LRESULT OsrImeHandler::OnIMEStartComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  handled = true;
  return 0;
}

LRESULT OsrImeHandler::OnIMEComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  CefString cTextStr;
  if (GetResult(lParam, cTextStr)) {
    // Send the text to the browser. The |replacement_range| and
    // |relative_cursor_pos| params are not used on Windows, so provide
    // default invalid values.
    owner_->browser_->GetHost()->ImeCommitText(cTextStr, CefRange::InvalidRange(), 0);
    ResetComposition();
  }

  std::vector<CefCompositionUnderline> underlines;
  int composition_start = 0;
  if (GetComposition(lParam, cTextStr, underlines, composition_start)) {
    // Send the composition string to the browser. The |replacement_range|
    // param is not used on Windows, so provide a default invalid value.
    owner_->browser_->GetHost()->ImeSetComposition(cTextStr, underlines, CefRange::InvalidRange(),
                                                   CefRange(composition_start, composition_start));

    // Update the Candidate Window position. The cursor is at the end so
    // subtract 1. This is safe because IMM32 does not support non-zero-width
    // in a composition. Also,  negative values are safely ignored in
    // MoveImeWindow
    UpdateCaretPosition(composition_start - 1);
  } else {
    IMECancelComposition();
  }

  handled = true;
  return 0;
}

LRESULT OsrImeHandler::OnIMEEndComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) {
  if (!owner_->browser_) {
    return 0;
  }

  IMECancelComposition();
  return 0;
}

void OsrImeHandler::OnImeCompositionRangeChanged(const CefRange& selection_range,
                                                 const std::vector<CefRect>& character_bounds) {
  // Convert from view coordinates to device coordinates.
  CefRenderHandler::RectList device_bounds;
  CefRenderHandler::RectList::const_iterator it = character_bounds.begin();
  for (; it != character_bounds.end(); ++it) {
    auto rect = *it;
    rect.x = osr_utils::LogicalToDevice(rect.x, owner_->scale_factor_);
    rect.y = osr_utils::LogicalToDevice(rect.y, owner_->scale_factor_);
    rect.width = osr_utils::LogicalToDevice(rect.width, owner_->scale_factor_);
    rect.height = osr_utils::LogicalToDevice(rect.height, owner_->scale_factor_);
    device_bounds.push_back(rect);
  }

  ChangeCompositionRange(selection_range, device_bounds);
}

// We need simulate composition behavior for sogou IME beacause sogou IME will not send IME_xxx_COMPOSITION messages
// during composition.
void OsrImeHandler::OnImeKeydown(UINT virtual_code) {
  if (is_composing_ || !owner_->browser_) {
    return;
  }

  // Both cursor index and element index are 0-based.
  // cursor index: '*', element index: '#', their layout is: '*#*#*#*#*#*#*#*#*'
  bool update_composition = true;
  if (VK_ESCAPE == virtual_code) {
    IMECancelComposition();
    update_composition = false;
  } else if (VK_BACK == virtual_code) {
    if (simulated_composition_text_.empty()) {
      update_composition = false;
    } else {
      simulated_composition_text_.erase(cursor_index_ - 1, 1);
      cursor_index_--;
      if (simulated_composition_text_.empty()) {
        IMECancelComposition();
        update_composition = false;
      }
    }
  } else if (VK_LEFT == virtual_code || VK_RIGHT == virtual_code) {
    if (simulated_composition_text_.empty()) {
      update_composition = false;
    } else {
      cursor_index_ += (VK_LEFT == virtual_code) ? -1 : 1;
      if (cursor_index_ >= static_cast<uint32_t>(simulated_composition_text_.size())) {
        cursor_index_ = static_cast<uint32_t>(simulated_composition_text_.size());
      }
    }
  } else if (VK_HOME == virtual_code || VK_END == virtual_code) {
    if (simulated_composition_text_.empty()) {
      update_composition = false;
    } else {
      cursor_index_ = (VK_HOME == virtual_code) ? 0 : static_cast<uint32_t>(simulated_composition_text_.size());
    }
  } else if (virtual_code >= 'A' && virtual_code <= 'z') {
    // Convert to lower case if caps lock not on to avoid the problem that the sogou IME always convert virtual key to
    // upper case.
    if (virtual_code <= 'Z' && !(GetKeyState(VK_CAPITAL) & 0x1)) {
      virtual_code += 'a' - 'A';
    }

    if (simulated_composition_text_.empty()) {
      cursor_index_ = 0;
    }
    simulated_composition_text_.insert(simulated_composition_text_.begin() + cursor_index_,
                                       static_cast<char>(virtual_code));
    cursor_index_++;
  } else {
    update_composition = false;
  }

  if (update_composition) {
    CefString composition_text(simulated_composition_text_);
    std::vector<CefCompositionUnderline> underlines;
    CefCompositionUnderline underline;
    underline.range = {0, (uint32_t)simulated_composition_text_.size()};
    underline.background_color = ColorBKCOLOR;
    underline.color = ColorUNDERLINE;
    underline.thick = 0;
    underlines.push_back(underline);
    owner_->browser_->GetHost()->ImeSetComposition(composition_text, underlines, CefRange::InvalidRange(),
                                                   CefRange(cursor_index_, cursor_index_));
  }
}

void OsrImeHandler::IMECancelComposition() {
  DCHECK(owner_->browser_);
  owner_->browser_->GetHost()->ImeCancelComposition();
  ResetComposition();
}

OsrImeHandler::OsrImeHandler(OsrWindow* owner) : cursor_index_(std::numeric_limits<uint32_t>::max()), owner_(owner) {
  hwnd_ = owner->hwnd_;
}

void OsrImeHandler::MoveImeWindow() {
  if (GetFocus() != hwnd_ || composition_bounds_.empty()) {
    return;
  }

  uint32_t location = cursor_index_;

  if (location == std::numeric_limits<uint32_t>::max()) {
    location = composition_range_.from;
  }

  if (location >= composition_range_.from) {
    location -= composition_range_.from;
  }

  if (location >= composition_bounds_.size()) {
    location = (uint32_t)composition_bounds_.size() - 1;
  }

  CefRect rc = composition_bounds_[location];

  HIMC imc = ::ImmGetContext(hwnd_);
  if (imc) {
    CANDIDATEFORM candidate_position = {0, CFS_CANDIDATEPOS, {rc.x, rc.y}, {0, 0, 0, 0}};
    ::ImmSetCandidateWindow(imc, &candidate_position);
    candidate_position = {0, CFS_EXCLUDE, {rc.x, rc.y}, {rc.x, rc.y, rc.x + rc.width, rc.y + rc.height}};
    ::ImmSetCandidateWindow(imc, &candidate_position);

    // Special handling for sogou IME:
    // 1. Y-position need move down some pixels for not overlay input when the scale factor is greater than 1.0.
    // 2. Calling ImmSetCandidateWindow doest not work for sogou IME, but calling ImmSetCompositionWindow does.
    if (owner_->scale_factor_ > 1.0f) {
      rc.y += 8;
    }
    COMPOSITIONFORM composition_position = {CFS_POINT, {rc.x, rc.y}, {0, 0, 0, 0}};
    ::ImmSetCompositionWindow(imc, &composition_position);

    ::ImmReleaseContext(hwnd_, imc);
  }
}

void OsrImeHandler::CleanupComposition() {
  // Notify the IMM attached to the given window to complete the ongoing
  // composition (when given window is de-activated while composing and
  // re-activated) and reset the composition status.
  if (is_composing_) {
    HIMC imc = ::ImmGetContext(hwnd_);
    if (imc) {
      ::ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
      ::ImmReleaseContext(hwnd_, imc);
    }
    ResetComposition();
  }
}

void OsrImeHandler::ResetComposition() {
  // Reset the composition status.
  is_composing_ = false;
  cursor_index_ = std::numeric_limits<uint32_t>::max();
  simulated_composition_text_.clear();
}

void OsrImeHandler::GetCompositionInfo(HIMC imc,
                                       LPARAM lparam,
                                       CefString& composition_text,
                                       std::vector<CefCompositionUnderline>& underlines,
                                       int& composition_start) {
  // We only care about GCS_COMPATTR, GCS_COMPCLAUSE and GCS_CURSORPOS, and
  // convert them into underlines and selection range respectively.
  underlines.clear();

  uint32_t length = static_cast<uint32_t>(composition_text.length());

  // Find out the range selected by the user.
  uint32_t target_start = length;
  uint32_t target_end = length;
  if (lparam & GCS_COMPATTR) {
    GetCompositionSelectionRange(imc, &target_start, &target_end);
  }

  // Retrieve the selection range information. If CS_NOMOVECARET is specified
  // it means the cursor should not be moved and we therefore place the caret at
  // the beginning of the composition string. Otherwise we should honour the
  // GCS_CURSORPOS value if it's available.
  // TODO(suzhe): Due to a bug in WebKit we currently can't use selection range
  // with composition string.
  // See: https://bugs.webkit.org/show_bug.cgi?id=40805
  if (!(lparam & CS_NOMOVECARET) && (lparam & GCS_CURSORPOS)) {
    // IMM32 does not support non-zero-width selection in a composition. So
    // always use the caret position as selection range.
    int cursor = ::ImmGetCompositionString(imc, GCS_CURSORPOS, nullptr, 0);
    composition_start = cursor;
  } else {
    composition_start = 0;
  }

  // Retrieve the clause segmentations and convert them to underlines.
  if (lparam & GCS_COMPCLAUSE) {
    GetCompositionUnderlines(imc, target_start, target_end, underlines);
  }

  // Set default underlines in case there is no clause information.
  if (!underlines.size()) {
    CefCompositionUnderline underline;
    underline.color = ColorUNDERLINE;
    underline.background_color = ColorBKCOLOR;
    if (target_start > 0) {
      underline.range.from = 0;
      underline.range.to = target_start;
      underline.thick = 0;
      underlines.push_back(underline);
    }
    if (target_end > target_start) {
      underline.range.from = target_start;
      underline.range.to = target_end;
      underline.thick = 1;
      underlines.push_back(underline);
    }
    if (target_end < length) {
      underline.range.from = target_end;
      underline.range.to = length;
      underline.thick = 0;
      underlines.push_back(underline);
    }
  }
}

bool OsrImeHandler::GetString(HIMC imc, WPARAM lparam, int type, CefString& result) {
  if (!(lparam & type)) {
    return false;
  }
  LONG string_size = ::ImmGetCompositionString(imc, type, nullptr, 0);
  if (string_size <= 0) {
    return false;
  }

  // For trailing nullptr - ImmGetCompositionString excludes that.
  string_size += sizeof(WCHAR);

  std::vector<wchar_t> buffer(string_size);
  ::ImmGetCompositionString(imc, type, &buffer[0], string_size);
  result.FromWString(&buffer[0]);
  return true;
}

bool OsrImeHandler::GetResult(LPARAM lparam, CefString& result) {
  bool ret = false;
  HIMC imc = ::ImmGetContext(hwnd_);
  if (imc) {
    ret = GetString(imc, lparam, GCS_RESULTSTR, result);
    ::ImmReleaseContext(hwnd_, imc);
  }
  return ret;
}

bool OsrImeHandler::GetComposition(LPARAM lparam,
                                   CefString& composition_text,
                                   std::vector<CefCompositionUnderline>& underlines,
                                   int& composition_start) {
  bool ret = false;
  HIMC imc = ::ImmGetContext(hwnd_);
  if (imc) {
    // Copy the composition string to the CompositionText object.
    ret = GetString(imc, lparam, GCS_COMPSTR, composition_text);

    if (ret) {
      // Retrieve the composition underlines and selection range information.
      GetCompositionInfo(imc, lparam, composition_text, underlines, composition_start);

      // Mark that there is an ongoing composition.
      is_composing_ = true;

      simulated_composition_text_.clear();
    }

    ::ImmReleaseContext(hwnd_, imc);
  }
  return ret;
}

void OsrImeHandler::DisableIME() {
  CleanupComposition();
  ::ImmAssociateContextEx(hwnd_, nullptr, 0);
}

void OsrImeHandler::CancelIME() {
  if (is_composing_) {
    HIMC imc = ::ImmGetContext(hwnd_);
    if (imc) {
      ::ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
      ::ImmReleaseContext(hwnd_, imc);
    }
    ResetComposition();
  }
}

void OsrImeHandler::EnableIME() {
  // Load the default IME context.
  ::ImmAssociateContextEx(hwnd_, nullptr, IACE_DEFAULT);
}

void OsrImeHandler::UpdateCaretPosition(uint32_t index) {
  // Save the caret position.
  cursor_index_ = index;
  // Move the IME window.
  MoveImeWindow();
}

void OsrImeHandler::ChangeCompositionRange(const CefRange& selection_range, const std::vector<CefRect>& bounds) {
  composition_range_ = selection_range;
  composition_bounds_ = bounds;
  MoveImeWindow();
}
