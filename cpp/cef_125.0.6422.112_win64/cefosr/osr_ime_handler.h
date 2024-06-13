#ifndef CEF_CEFOSR_OSR_IME_HANDLER_H_
#define CEF_CEFOSR_OSR_IME_HANDLER_H_

#include <windows.h>
#include <vector>

#include "include/internal/cef_types_wrappers.h"

class OsrWindow;

// Handles IME for the native parent window that hosts an off-screen browser.
// This object is only accessed on the CEF UI thread.
class OsrImeHandler {
 public:
  explicit OsrImeHandler(OsrWindow* owner);

  LRESULT OnIMESetContext(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnIMEStartComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnIMEComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);
  LRESULT OnIMEEndComposition(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled);

  void OnImeCompositionRangeChanged(const CefRange& selection_range, const std::vector<CefRect>& character_bounds);
  void OnImeKeydown(UINT virtual_code);

  // Enables the IME attached to the given window.
  void EnableIME();

  // Disables the IME attached to the given window.
  void DisableIME();

  // Cancels an ongoing composition of the IME.
  void CancelIME();

 private:
  void IMECancelComposition();

  // Retrieves whether or not there is an ongoing composition.
  bool is_composing() const { return is_composing_; }

  // Cleans up the all resources attached to the given IMM32Manager object, and
  // reset its composition status.
  void CleanupComposition();

  // Resets the composition status and cancels the ongoing composition.
  void ResetComposition();

  // Retrieves a composition result of the ongoing composition if it exists.
  bool GetResult(LPARAM lparam, CefString& result);

  // Retrieves the current composition status of the ongoing composition.
  // Includes composition text, underline information and selection range in the
  // composition text. IMM32 does not support char selection.
  bool GetComposition(LPARAM lparam,
                      CefString& composition_text,
                      std::vector<CefCompositionUnderline>& underlines,
                      int& composition_start);

  // Updates the IME caret position of the given window.
  void UpdateCaretPosition(uint32_t index);

  // Updates the composition range. |selected_range| is the range of characters
  // that have been selected. |character_bounds| is the bounds of each character
  // in view device coordinates.
  void ChangeCompositionRange(const CefRange& selection_range, const std::vector<CefRect>& character_bounds);

  // Updates the position of the IME windows.
  void MoveImeWindow();

  // Retrieves the composition information.
  void GetCompositionInfo(HIMC imm_context,
                          LPARAM lparam,
                          CefString& composition_text,
                          std::vector<CefCompositionUnderline>& underlines,
                          int& composition_start);

  // Retrieves a string from the IMM.
  bool GetString(HIMC imm_context, WPARAM lparam, int type, CefString& result);

  // Represents whether or not there is an ongoing composition.
  bool is_composing_ = false;

  // The current composition character range and its bounds.
  std::vector<CefRect> composition_bounds_;

  // The current cursor index in composition string.
  uint32_t cursor_index_;

  // The composition range in the string. This may be used to determine the
  // offset in composition bounds.
  CefRange composition_range_;

  // Hwnd associated with this instance.
  HWND hwnd_;

  std::string simulated_composition_text_;

  OsrWindow* owner_;
};  // OsrImeHandler

#endif  // CEF_CEFOSR_OSR_IME_HANDLER_H_
