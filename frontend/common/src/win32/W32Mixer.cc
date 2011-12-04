// W32Mixer.cc --- W32Audio mixer
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
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
#include "config.h"
#endif

#include <windows.h>
#include <initguid.h>
#include <mmsystem.h>
#include "debug.hh"

#include "IConfigurator.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "W32Mixer.hh"
#include "Util.hh"
#include <debug.hh>

using namespace std;
using namespace workrave;

#ifndef HAVE_MMDEVICEAPI_H
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e);
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5cdf2c82, 0x841e, 0x4546, 0x97, 0x22, 0x0c, 0xf7, 0x40, 0x78, 0x22, 0x9a);
DEFINE_GUID(IID_IMMDeviceEnumerator,  0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6);
#else
#define CLSID_MMDeviceEnumerator __uuidof(MMDeviceEnumerator)
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5cdf2c82, 0x841e, 0x4546, 0x97, 0x22, 0x0c, 0xf7, 0x40, 0x78, 0x22, 0x9a);
//#define IID_IAudioEndpointVolume __uuidof(IAudioEndpointVolume)
#define IID_IMMDeviceEnumerator __uuidof(IMMDeviceEnumerator)
#endif

W32Mixer::W32Mixer()
  : endpoint_volume(NULL)
{
}

W32Mixer::~W32Mixer()
{
  TRACE_ENTER("W32Mixer::~W32Mixer");

  if (endpoint_volume != NULL)
    {
      endpoint_volume->Release();
      endpoint_volume = NULL;
    }

  CoUninitialize();
  TRACE_EXIT();
}

bool
W32Mixer::set_mute(bool on)
{
  TRACE_ENTER_MSG("W32Mixer::set_mute", on);

  bool was_muted = false;

  if (endpoint_volume != NULL)
    {
      was_muted = set_mute_mmdevice(on);
    }
  else
    {
      was_muted = set_mute_mixer(on);
    }

  TRACE_EXIT();
  return was_muted;
}

void
W32Mixer::init()
{
  TRACE_ENTER("W32Mixer::init");

  CoInitialize(NULL);

  HRESULT hr;

  IMMDeviceEnumerator *device_enum = NULL;
  IMMDevice *default_device = NULL;

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

  TRACE_MSG(hr);
  TRACE_EXIT();
}


bool
W32Mixer::set_mute_mmdevice(bool on)
{
  TRACE_ENTER_MSG("W32Mixer::set_mute_mmdevice", on);

  HRESULT hr;

  BOOL mute = FALSE;
  hr = endpoint_volume->GetMute(&mute);

  if (hr == S_OK)
    {
      TRACE_MSG("current mute is: " <<  mute);
      hr = endpoint_volume->SetMute(on, NULL);
    }

  return mute;
}


bool
W32Mixer::set_mute_mixer(bool on)
{
  TRACE_ENTER_MSG("W32Mixer::set_mute_mixer", on);

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

      mixer_control_details.cbStruct	     = sizeof(MIXERCONTROLDETAILS);
      mixer_control_details.dwControlID    = mute_control;
      mixer_control_details.cMultipleItems = 0;
      mixer_control_details.cChannels      = 1;
      mixer_control_details.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
      mixer_control_details.paDetails      = &value;

      mixerGetControlDetails(NULL, &mixer_control_details, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER);
      ret = value.fValue;
      TRACE_MSG("current mute is: " <<  ret);

      value.fValue = on;
      mixerSetControlDetails(NULL, &mixer_control_details, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER);
    }

  if (mixer_control != NULL)
    {
      delete [] mixer_control;
    }

  TRACE_EXIT();
  return ret;
}
