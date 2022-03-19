// Copyright (C) 2010, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"

#include <initguid.h>

#include "W32Mixer.hh"

#include <windows.h>
#include <mmsystem.h>
#include <shobjidl.h>

W32Mixer::W32Mixer()
  : endpoint_volume(NULL)
{
}

W32Mixer::~W32Mixer()
{
  TRACE_ENTRY();
  if (endpoint_volume != NULL)
    {
      endpoint_volume->Release();
      endpoint_volume = NULL;
    }

  CoUninitialize();
}

bool
W32Mixer::set_mute(bool on)
{
  TRACE_ENTRY_PAR(on);

  bool was_muted = false;

  if (endpoint_volume != NULL)
    {
      was_muted = set_mute_mmdevice(on);
    }
  else
    {
      was_muted = set_mute_mixer(on);
    }

  return was_muted;
}

void
W32Mixer::init()
{
  TRACE_ENTRY();
  CoInitialize(NULL);

  HRESULT hr;

  IMMDeviceEnumerator *device_enum = NULL;
  IMMDevice *default_device = NULL;

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
  const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
  const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
  const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);
#endif

  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (LPVOID *)&device_enum);
  if (hr == S_OK)
    {
      hr = device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &default_device);
      device_enum->Release();
    }

  if (hr == S_OK)
    {
      hr = default_device->Activate(IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpoint_volume);

      default_device->Release();
    }

  TRACE_VAR(hr);
}

bool
W32Mixer::set_mute_mmdevice(bool on)
{
  TRACE_ENTRY_PAR(on);

  HRESULT hr;

  BOOL mute = FALSE;
  hr = endpoint_volume->GetMute(&mute);

  if (hr == S_OK)
    {
      TRACE_MSG("current mute is: {}", mute);
      hr = endpoint_volume->SetMute(on, NULL);
    }

  return mute;
}

bool
W32Mixer::set_mute_mixer(bool on)
{
  TRACE_ENTRY_PAR(on);

  MMRESULT result = MMSYSERR_NOERROR;
  MIXERLINE mixer_line;
  MIXERLINECONTROLS mixer_line_controls;
  MIXERCONTROL *mixer_control = NULL;
  int mute_control = -1;
  bool ret = false;

  memset(&mixer_line, 0, sizeof(MIXERLINE));
  memset(&mixer_line_controls, 0, sizeof(MIXERLINECONTROLS));

  mixer_line.cbStruct = sizeof(MIXERLINE);
  mixer_line_controls.cbStruct = sizeof(MIXERLINECONTROLS);

  result = mixerGetLineInfo(NULL, &mixer_line, MIXER_OBJECTF_MIXER | MIXER_GETLINEINFOF_DESTINATION);
  if (result == MMSYSERR_NOERROR)
    {
      mixer_control = new MIXERCONTROL[mixer_line.cControls];

      mixer_line_controls.dwLineID = mixer_line.dwLineID;
      mixer_line_controls.cControls = mixer_line.cControls;
      mixer_line_controls.cbmxctrl = sizeof(MIXERCONTROL);
      mixer_line_controls.pamxctrl = mixer_control;

      result = mixerGetLineControls(NULL, &mixer_line_controls, MIXER_OBJECTF_MIXER | MIXER_GETLINECONTROLSF_ALL);
    }

  if (result == MMSYSERR_NOERROR)
    {
      for (unsigned int i = 0; i < mixer_line_controls.cControls; i++)
        {
          if (mixer_control[i].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
            {
              mute_control = mixer_control[i].dwControlID;
              break;
            }
        }
    }

  if (result == MMSYSERR_NOERROR && mute_control != -1)
    {
      MIXERCONTROLDETAILS_BOOLEAN value;
      memset(&value, 0, sizeof(MIXERCONTROLDETAILS_BOOLEAN));

      value.fValue = FALSE;

      MIXERCONTROLDETAILS mixer_control_details;
      memset(&mixer_control_details, 0, sizeof(MIXERCONTROLDETAILS));

      mixer_control_details.cbStruct = sizeof(MIXERCONTROLDETAILS);
      mixer_control_details.dwControlID = mute_control;
      mixer_control_details.cMultipleItems = 0;
      mixer_control_details.cChannels = 1;
      mixer_control_details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      mixer_control_details.paDetails = &value;

      mixerGetControlDetails(NULL, &mixer_control_details, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER);
      ret = value.fValue;
      TRACE_MSG("current mute is: {}", ret);

      value.fValue = on;
      mixerSetControlDetails(NULL, &mixer_control_details, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER);
    }

  if (mixer_control != NULL)
    {
      delete[] mixer_control;
    }

  return ret;
}
