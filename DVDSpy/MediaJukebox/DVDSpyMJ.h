/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu Oct 03 10:54:40 2002
 */
/* Compiler settings for C:\Program Files\girder32\plugins\DVDSpy\MediaJukebox\DVDSpyMJ.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __DVDSpyMJ_h__
#define __DVDSpyMJ_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IDVDSpyMJCtrl_FWD_DEFINED__
#define __IDVDSpyMJCtrl_FWD_DEFINED__
typedef interface IDVDSpyMJCtrl IDVDSpyMJCtrl;
#endif 	/* __IDVDSpyMJCtrl_FWD_DEFINED__ */


#ifndef __DVDSpyMJCtrl_FWD_DEFINED__
#define __DVDSpyMJCtrl_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVDSpyMJCtrl DVDSpyMJCtrl;
#else
typedef struct DVDSpyMJCtrl DVDSpyMJCtrl;
#endif /* __cplusplus */

#endif 	/* __DVDSpyMJCtrl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IDVDSpyMJCtrl_INTERFACE_DEFINED__
#define __IDVDSpyMJCtrl_INTERFACE_DEFINED__

/* interface IDVDSpyMJCtrl */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDVDSpyMJCtrl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5FFB900C-BB18-4DB8-9066-FA905AF9BF04")
    IDVDSpyMJCtrl : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ LPDISPATCH pDisp) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RefreshInterval( 
            /* [retval][out] */ short __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RefreshInterval( 
            /* [in] */ short newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDVDSpyMJCtrlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDVDSpyMJCtrl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDVDSpyMJCtrl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ LPDISPATCH pDisp);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RefreshInterval )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [retval][out] */ short __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RefreshInterval )( 
            IDVDSpyMJCtrl __RPC_FAR * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IDVDSpyMJCtrl __RPC_FAR * This);
        
        END_INTERFACE
    } IDVDSpyMJCtrlVtbl;

    interface IDVDSpyMJCtrl
    {
        CONST_VTBL struct IDVDSpyMJCtrlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVDSpyMJCtrl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVDSpyMJCtrl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVDSpyMJCtrl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVDSpyMJCtrl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVDSpyMJCtrl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVDSpyMJCtrl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVDSpyMJCtrl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVDSpyMJCtrl_Init(This,pDisp)	\
    (This)->lpVtbl -> Init(This,pDisp)

#define IDVDSpyMJCtrl_get_RefreshInterval(This,pVal)	\
    (This)->lpVtbl -> get_RefreshInterval(This,pVal)

#define IDVDSpyMJCtrl_put_RefreshInterval(This,newVal)	\
    (This)->lpVtbl -> put_RefreshInterval(This,newVal)

#define IDVDSpyMJCtrl_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDVDSpyMJCtrl_Init_Proxy( 
    IDVDSpyMJCtrl __RPC_FAR * This,
    /* [in] */ LPDISPATCH pDisp);


void __RPC_STUB IDVDSpyMJCtrl_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IDVDSpyMJCtrl_get_RefreshInterval_Proxy( 
    IDVDSpyMJCtrl __RPC_FAR * This,
    /* [retval][out] */ short __RPC_FAR *pVal);


void __RPC_STUB IDVDSpyMJCtrl_get_RefreshInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IDVDSpyMJCtrl_put_RefreshInterval_Proxy( 
    IDVDSpyMJCtrl __RPC_FAR * This,
    /* [in] */ short newVal);


void __RPC_STUB IDVDSpyMJCtrl_put_RefreshInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IDVDSpyMJCtrl_Reset_Proxy( 
    IDVDSpyMJCtrl __RPC_FAR * This);


void __RPC_STUB IDVDSpyMJCtrl_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDVDSpyMJCtrl_INTERFACE_DEFINED__ */



#ifndef __DVDSPYMJLib_LIBRARY_DEFINED__
#define __DVDSPYMJLib_LIBRARY_DEFINED__

/* library DVDSPYMJLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DVDSPYMJLib;

EXTERN_C const CLSID CLSID_DVDSpyMJCtrl;

#ifdef __cplusplus

class DECLSPEC_UUID("983D279F-0C14-4E8E-A3C4-C5C128C5D536")
DVDSpyMJCtrl;
#endif
#endif /* __DVDSPYMJLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
