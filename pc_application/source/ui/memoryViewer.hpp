#pragma once

#include <cstdint>
#include <mio.hpp>
#include <system_error>
#include <wx/wx.h>

// This will use a wxListCtrl to list the memory locations currently shown
// You will be able to add values by using their memory viewer fancy string version
// The type will be specified and the data will be exported to a file on demand
class MemoryViewer : public wxFrame {
private:
public:
};