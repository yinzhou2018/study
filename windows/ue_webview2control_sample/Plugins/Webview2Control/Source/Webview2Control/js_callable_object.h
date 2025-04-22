
#ifndef JS_CALLABLE_OBJECT_H_
#define JS_CALLABLE_OBJECT_H_
#include <wrl.h>
#include <string>
#include <vector>

using namespace Microsoft::WRL;

class JSCallableObject : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IDispatch> {
 public:
  // IDispatch methods
  HRESULT STDMETHODCALLTYPE GetTypeInfoCount(/* [out] */ __RPC__out UINT* pctinfo) override;
  HRESULT STDMETHODCALLTYPE GetTypeInfo(
      /* [in] */ UINT iTInfo,
      /* [in] */ LCID lcid,
      /* [out] */ __RPC__deref_out_opt ITypeInfo** ppTInfo) override;
  HRESULT STDMETHODCALLTYPE GetIDsOfNames(
      /* [in] */ __RPC__in REFIID riid,
      /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR* rgszNames,
      /* [range][in] */ __RPC__in_range(0, 16384) UINT cNames,
      /* [in] */ LCID lcid,
      /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID* rgDispId) override;
  HRESULT STDMETHODCALLTYPE Invoke(
      /* [annotation][in] */
      _In_ DISPID dispIdMember,
      /* [annotation][in] */
      _In_ REFIID riid,
      /* [annotation][in] */
      _In_ LCID lcid,
      /* [annotation][in] */
      _In_ WORD wFlags,
      /* [annotation][out][in] */
      _In_ DISPPARAMS* pDispParams,
      /* [annotation][out] */
      _Out_opt_ VARIANT* pVarResult,
      /* [annotation][out] */
      _Out_opt_ EXCEPINFO* pExcepInfo,
      /* [annotation][out] */
      _Out_opt_ UINT* puArgErr) override;

 private:
  HRESULT OnAddHandler(DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
  HRESULT OnSubHandler(DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
  HRESULT OnUnreachableHandler(DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

  using JSCallHandler = HRESULT (JSCallableObject::*)(DISPPARAMS* pDispParams,
                                                      VARIANT* pVarResult,
                                                      EXCEPINFO* pExcepInfo,
                                                      UINT* puArgErr);
  using JSCallHandlerPair = std::pair<std::wstring, JSCallHandler>;
  static std::vector<JSCallHandlerPair> s_handlers_;
};  // JSCallableObject

#endif  // JS_CALLABLE_OBJECT_H_