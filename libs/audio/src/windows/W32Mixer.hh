// Copyright (C) 2010, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef W32MIXER_HH
#define W32MIXER_HH

#include "IMixer.hh"

#if defined(HAVE_MMDEVICEAPI_H)
#  include <mmdeviceapi.h>
#  include <endpointvolume.h>
#else

typedef interface IMMDeviceCollection IMMDeviceCollection;
typedef interface IMMNotificationClient IMMNotificationClient;
typedef interface IAudioEndpointVolumeCallback IAudioEndpointVolumeCallback;
typedef interface IPropertyStore IPropertyStore;

typedef enum
{
  eRender = 0,
  eCapture = 1,
  eAll = 2,
  EDataFlow_enum_count = 3
} EDataFlow;

typedef enum
{
  eConsole = 0,
  eMultimedia = 1,
  eCommunications = 2,
  ERole_enum_count = 3,
} ERole;

class IMMDevice : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Activate(REFIID, DWORD, PROPVARIANT *, void **) = 0;
  virtual HRESULT STDMETHODCALLTYPE OpenPropertyStore(DWORD, IPropertyStore **) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetState(DWORD *) = 0;
};

class IMMDeviceEnumerator : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE EnumAudioEndpoints(EDataFlow, DWORD, IMMDeviceCollection **) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint(EDataFlow, ERole, IMMDevice **) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDevice(LPCWSTR, IMMDevice **) = 0;
  virtual HRESULT STDMETHODCALLTYPE RegisterEndpointNotificationCallback(IMMNotificationClient *) = 0;
  virtual HRESULT STDMETHODCALLTYPE UnregisterEndpointNotificationCallback(IMMNotificationClient *) = 0;
};

class IAudioEndpointVolume : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE RegisterControlChangeNotify(IAudioEndpointVolumeCallback *) = 0;
  virtual HRESULT STDMETHODCALLTYPE UnregisterControlChangeNotify(IAudioEndpointVolumeCallback *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetChannelCount(UINT *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevel(float, LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevelScalar(float, LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevel(float *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevelScalar(float *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevel(UINT, float, LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevelScalar(UINT, float, LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevel(UINT, float *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevelScalar(UINT, float *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetMute(BOOL, LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetMute(BOOL *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVolumeStepInfo(UINT *, UINT *) = 0;
  virtual HRESULT STDMETHODCALLTYPE VolumeStepUp(LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE VolumeStepDown(LPCGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVolumeRange(float *, float *, float *) = 0;
};
#endif

class W32Mixer : public IMixer
{
public:
  W32Mixer();
  virtual ~W32Mixer();

  void init();
  bool set_mute(bool on);

  bool set_mute_mmdevice(bool on);
  bool set_mute_mixer(bool on);

private:
  IAudioEndpointVolume *endpoint_volume;
};

#endif // W32MIXER_HH
