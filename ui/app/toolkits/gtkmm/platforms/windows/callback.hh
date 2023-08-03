// Copyright (C) 2022 Rob Caelers <rob.caelers@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <atomic>
#include <functional>

#include <windows.h>

#include <wrl/client.h>

namespace details
{
  template<typename T, typename... Args>
  class InvokeHelper : public T
  {
  public:
    explicit InvokeHelper(std::function<HRESULT(Args...)> lambda)
      : ref_count_(0)
      , lambda_(lambda)
    {
    }
    virtual ~InvokeHelper() = default;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **obj) override
    {
      if (iid == __uuidof(T) || iid == IID_IUnknown)
        {
          *obj = this;
          AddRef();
          return S_OK;
        }

      *obj = nullptr;
      return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
      return ++ref_count_;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
      size_t ret = --ref_count_;
      if (ret == 0)
        {
          delete this;
        }
      return ret;
    }

    HRESULT STDMETHODCALLTYPE Invoke(Args... args) override
    {
      return lambda_(args...);
    }

  private:
    std::atomic<size_t> ref_count_;
    std::function<HRESULT(Args...)> lambda_;
  };

  template<typename T, typename InvokeT>
  class CallbackHelper;

  template<typename T, typename... Args>
  class CallbackHelper<T, HRESULT (STDMETHODCALLTYPE T::*)(Args...)>
  {
  public:
    template<typename LambdaT>
    static Microsoft::WRL::ComPtr<T> Callback(LambdaT &&lambda)
    {
      return Microsoft::WRL::ComPtr<T>(new InvokeHelper<T, Args...>(std::forward<LambdaT>(lambda)));
    }
  };
} // namespace details

template<typename T, typename LambdaT>
Microsoft::WRL::ComPtr<T>
Callback(LambdaT &&lambda)
{
  return details::CallbackHelper<T, decltype(&T::Invoke)>::Callback(std::forward<LambdaT>(lambda));
}
