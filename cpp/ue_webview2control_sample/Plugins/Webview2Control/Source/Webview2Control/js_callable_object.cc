#include "js_callable_object.h"

#include <cassert>

// 这里添加JS调用接口
std::vector<JSCallableObject::JSCallHandlerPair> JSCallableObject::s_handlers_ = {
    // 索引为0的handler是占位符，不可通过JavaScript调用, 0不能作为DispId值，保留使用，
    // 参见：https://learn.microsoft.com/en-us/windows/win32/api/oaidl/nf-oaidl-idispatch-getidsofnames
    {L"", &JSCallableObject::OnUnreachableHandler},
    {L"sub", &JSCallableObject::OnSubHandler},
    {L"add", &JSCallableObject::OnAddHandler},
};

HRESULT STDMETHODCALLTYPE JSCallableObject::GetTypeInfoCount(/* [out] */ __RPC__out UINT* pctinfo) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE JSCallableObject::GetTypeInfo(
    /* [in] */ UINT iTInfo,
    /* [in] */ LCID lcid,
    /* [out] */ __RPC__deref_out_opt ITypeInfo** ppTInfo) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE JSCallableObject::GetIDsOfNames(
    /* [in] */ __RPC__in REFIID riid,
    /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR* rgszNames,
    /* [range][in] */ __RPC__in_range(0, 16384) UINT cNames,
    /* [in] */ LCID lcid,
    /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID* rgDispId) {
  if (1 == cNames) {
    auto iter = std::find_if(s_handlers_.begin(), s_handlers_.end(),
                             [&](const auto& pair) { return pair.first == rgszNames[0]; });
    if (iter != s_handlers_.end()) {
      *rgDispId = std::distance(s_handlers_.begin(), iter);
      return S_OK;
    }
  }

  return DISP_E_UNKNOWNNAME;
}

HRESULT STDMETHODCALLTYPE JSCallableObject::Invoke(
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
    _Out_opt_ UINT* puArgErr) {
  static DISPID s_real_dispid_member = 0;

  // 为了简化实现逻辑没有实现`GetTypeInfoCount`及`GetTypeInfo`暴露对象元数据信息，一次JS调用会触发两次`Invoke`调用，
  // 第一次当做属性来获取值，不会携带任何参数，必须返回对象自身的`IDispatch`接口，才能触发第二次方法调用，
  // 但第二次调用`dispIdMember`为0，所以需要在第一次调用把正确的`dispIdMember`保存起来，在第二次调用中使用。
  if (DISPATCH_PROPERTYGET == wFlags) {
    s_real_dispid_member = dispIdMember;
    pVarResult->vt = VT_DISPATCH;
    pVarResult->pdispVal = this;
    pVarResult->pdispVal->AddRef();
    return S_OK;
  }

  if (0 == dispIdMember) {
    dispIdMember = s_real_dispid_member;
    s_real_dispid_member = 0;
  }

  auto iter = s_handlers_.begin();
  std::advance(iter, dispIdMember);
  if (iter != s_handlers_.end()) {
    return (this->*iter->second)(pDispParams, pVarResult, pExcepInfo, puArgErr);
  }

  return E_NOTIMPL;
}

HRESULT JSCallableObject::OnAddHandler(DISPPARAMS* pDispParams,
                                       VARIANT* pVarResult,
                                       EXCEPINFO* pExcepInfo,
                                       UINT* puArgErr) {
  if (!pDispParams || pDispParams->cArgs < 2) {
    return E_INVALIDARG;
  }

  // 注意：JS调用的参数传递顺序与这里得到的参数数组顺序是反向的
  int result = 0;
  for (int i = pDispParams->cArgs - 1; i >= 0; i--) {
    int item = V_INT(&pDispParams->rgvarg[i]);
    result += item;
  }

  pVarResult->vt = VT_INT;
  pVarResult->intVal = result;
  return S_OK;
}

HRESULT JSCallableObject::OnSubHandler(DISPPARAMS* pDispParams,
                                       VARIANT* pVarResult,
                                       EXCEPINFO* pExcepInfo,
                                       UINT* puArgErr) {
  if (!pDispParams || pDispParams->cArgs < 2) {
    return E_INVALIDARG;
  }

  // 注意：JS调用的参数传递顺序与这里得到的参数数组顺序是反向的
  int result = 0;
  for (int i = pDispParams->cArgs - 1; i >= 0; i--) {
    int item = V_INT(&pDispParams->rgvarg[i]);
    if (pDispParams->cArgs - 1 == i) {
      result = item;
    } else {
      result -= item;
    }
  }

  pVarResult->vt = VT_INT;
  pVarResult->intVal = result;
  return S_OK;
}

HRESULT JSCallableObject::OnUnreachableHandler(DISPPARAMS* pDispParams,
                                               VARIANT* pVarResult,
                                               EXCEPINFO* pExcepInfo,
                                               UINT* puArgErr) {
  assert(false);
  return E_NOTIMPL;
}
