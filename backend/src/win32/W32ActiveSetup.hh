// W32ActiveSetup.hh --- Active Setup for Workrave
//
// Copyright (C) 2012 Ray Satiro
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef W32ACTIVESETUP_HH
#define W32ACTIVESETUP_HH

class W32ActiveSetup
{
public:
    static const REGSAM registry_view;
    static const wchar_t component_path[];
    static const wchar_t guid_autorun[];

    static const WCHAR *W32ActiveSetup::get_user_profile_dir();

    static bool W32ActiveSetup::update(
        const std::wstring &guid
        );

    static bool W32ActiveSetup::update_all();

private:
    enum reg
    {
        HKCU,
        HKLM
    };

    static bool W32ActiveSetup::is_os_64();

    static bool W32ActiveSetup::check_guid(
        const enum reg reg, // HKLM or HKCU
        const std::wstring &guid
        );

    static bool W32ActiveSetup::is_guid_enabled(
        const std::wstring &guid
        );

    static bool W32ActiveSetup::read_from_registry_value( 
        const enum reg reg, // HKLM or HKCU
        const std::wstring &guid,
        const std::wstring &value,
        std::wstring &data // out
        );

    static bool W32ActiveSetup::write_to_registry_value(
        const std::wstring &guid,
        const std::wstring &value,
        const std::wstring &data
        );

    static bool W32ActiveSetup::get_version(
        const enum reg reg, // HKLM or HKCU
        const std::wstring &guid,
        std::vector<DWORD> &version // out
        );

    static bool W32ActiveSetup::set_version(
        const std::wstring &guid,
        const std::vector<DWORD> &version
        );

    static DWORD WINAPI W32ActiveSetup::create_process(
        LPVOID lpParam
        );
};

#endif // W32ACTIVESETUP_HH
