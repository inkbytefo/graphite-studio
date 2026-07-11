#include "platform/FileDialogs.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

namespace platform {

// ── UTF Conversion Helpers ───────────────────────────────────────────────────

std::string WStringToUTF8(const std::wstring& wstr) {
#ifdef _WIN32
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
#else
    // On non-Windows platforms, wstring is rarely used; provide a basic stub
    return std::string(wstr.begin(), wstr.end());
#endif
}

std::wstring UTF8ToWString(const std::string& str) {
#ifdef _WIN32
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wpath(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wpath[0], size_needed);
    return wpath;
#else
    return std::wstring(str.begin(), str.end());
#endif
}

// ── Native File Dialogs ──────────────────────────────────────────────────────

#ifdef _WIN32

std::string OpenFileDialog() {
    wchar_t szFile[260] = { 0 };
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Görsel Dosyaları (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0Tüm Dosyalar (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        return WStringToUTF8(std::wstring(ofn.lpstrFile));
    }
    return "";
}

std::string SaveFileDialog() {
    wchar_t szFile[260] = { 0 };
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"PNG Görsel (*.png)\0*.png\0JPEG Görsel (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0BMP Görsel (*.bmp)\0*.bmp\0TGA Görsel (*.tga)\0*.tga\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn) == TRUE) {
        std::wstring path(ofn.lpstrFile);
        if (path.find(L'.') == std::wstring::npos) {
            if (ofn.nFilterIndex == 1) path += L".png";
            else if (ofn.nFilterIndex == 2) path += L".jpg";
            else if (ofn.nFilterIndex == 3) path += L".bmp";
            else if (ofn.nFilterIndex == 4) path += L".tga";
        }
        return WStringToUTF8(path);
    }
    return "";
}

#else

std::string OpenFileDialog() {
    return ""; // Fallback for other platforms
}

std::string SaveFileDialog() {
    return "";
}

#endif

} // namespace platform
