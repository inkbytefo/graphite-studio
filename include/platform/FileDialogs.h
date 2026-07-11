#pragma once

#include <string>

namespace platform {

// UTF-8 <-> UTF-16 conversion helpers (Windows-specific, no-op stubs on other platforms)
std::string WStringToUTF8(const std::wstring& wstr);
std::wstring UTF8ToWString(const std::string& str);

// Native file dialogs
// Returns empty string if the user cancels the dialog
std::string OpenFileDialog();
std::string SaveFileDialog();

} // namespace platform
