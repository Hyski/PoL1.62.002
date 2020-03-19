

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for cdo.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

#if defined __cplusplus && !defined CDO_NO_NAMESPACE
namespace CDO {
#else
#undef IDataSource
#endif

MIDL_DEFINE_GUID(IID, IID_IDataSource,0xCD000029,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IMessage,0xCD000020,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IBodyPart,0xCD000021,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IConfiguration,0xCD000022,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IMessages,0xCD000025,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IDropDirectory,0xCD000024,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IBodyParts,0xCD000023,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_ISMTPScriptConnector,0xCD000030,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPEarlyScriptConnector,0xCD000034,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPPostScriptConnector,0xCD000031,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPFinalScriptConnector,0xCD000032,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_ISMTPOnArrival,0xCD000026,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPostEarly,0xCD000033,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPost,0xCD000027,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPostFinal,0xCD000028,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IProxyObject,0xCD000083,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IGetInterface,0xCD0ff000,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, LIBID_CDO,0xCD000000,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_Message,0xCD000001,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_Configuration,0xCD000002,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_DropDirectory,0xCD000004,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_SMTPConnector,0xCD000008,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPEarlyConnector,0xCD000011,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPPostConnector,0xCD000009,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPFinalConnector,0xCD000010,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);

#undef MIDL_DEFINE_GUID
#if defined __cplusplus && !defined CDO_NO_NAMESPACE
} // namespace CDO
#endif

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/



/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* Compiler settings for cdo.idl:
    Oicf, W1, Zp8, env=Win64 (32b run,appending)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if defined(_M_IA64) || defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

#if defined __cplusplus && !defined CDO_NO_NAMESPACE
namespace CDO {
#else
#undef IDataSource
#endif

MIDL_DEFINE_GUID(IID, IID_IDataSource,0xCD000029,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IMessage,0xCD000020,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IBodyPart,0xCD000021,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IConfiguration,0xCD000022,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IMessages,0xCD000025,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IDropDirectory,0xCD000024,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IBodyParts,0xCD000023,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_ISMTPScriptConnector,0xCD000030,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPEarlyScriptConnector,0xCD000034,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPPostScriptConnector,0xCD000031,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPFinalScriptConnector,0xCD000032,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_ISMTPOnArrival,0xCD000026,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPostEarly,0xCD000033,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPost,0xCD000027,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_INNTPOnPostFinal,0xCD000028,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IProxyObject,0xCD000083,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, IID_IGetInterface,0xCD0ff000,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(IID, LIBID_CDO,0xCD000000,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_Message,0xCD000001,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_Configuration,0xCD000002,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_DropDirectory,0xCD000004,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_SMTPConnector,0xCD000008,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPEarlyConnector,0xCD000011,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPPostConnector,0xCD000009,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);


MIDL_DEFINE_GUID(CLSID, CLSID_NNTPFinalConnector,0xCD000010,0x8B95,0x11D1,0x82,0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);

#undef MIDL_DEFINE_GUID
#if defined __cplusplus && !defined CDO_NO_NAMESPACE
} // namespace CDO
#endif

#ifdef __cplusplus
}
#endif



#endif /* defined(_M_IA64) || defined(_M_AMD64)*/

