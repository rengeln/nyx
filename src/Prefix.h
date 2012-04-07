///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_PREFIX_H__
#define __NYX_PREFIX_H__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#include <d3d11.h>
#include <D3DX11.h>
#include <DXGI.h>
#include <DxErr.h>
#include <xnamath.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/fterrors.h>

typedef unsigned int uint;

#include "Vector.h"
#include "Matrix.h"

//
//  Generic exception class.
//
class Exception : public virtual std::exception,
                  public virtual boost::exception
{
public:
	//
	//	Constructor.
	//
	//	Parameters:
	//		[in] msg
	//			Error message.
	//
	Exception(const std::string& msg);
    Exception(const boost::format& msg);

	//
	//	Appends a string to the error message.
	//
	Exception& operator<<(const std::string& str);
	Exception& operator<<(const boost::format& str);

	//
	//	Returns the error message.
	//
	virtual const char* what() const throw();

private:
    //
    //  Properties.
    //
    std::string m_message;
};


inline Exception::Exception(const std::string& msg)
: m_message(msg)
{
}

inline Exception::Exception(const boost::format& msg)
: m_message(msg.str())
{
}

inline Exception& Exception::operator<<(const std::string& str)
{
    m_message.append(str);
    return *this;
}

inline Exception& Exception::operator<<(const boost::format& str)
{
    m_message.append(str.str());
    return *this;
}

inline const char* Exception::what() const throw()
{
    return m_message.c_str();
}

//
//	Template class for specializing exception types.
//	
//	Parameters:
//		[template] T
//			Tag type for specializing the exception; unused.
//		[template] Base
//			Type to inherit from.
//
template <typename T, typename Base = Exception>
class Error : public Base
{
public:
	//
	//	Constructor.
	//
	//	Parameters:
	//		[in] msg
	//			Error message.
	//
	Error(const std::string& msg);
	Error(const boost::format& msg);
};

template <typename T, typename Base>
Error<T, Base>::Error(const std::string& msg)
: Base(msg)
{
}

template <typename T, typename Base>
Error<T, Base>::Error(const boost::format& msg)
: Base(msg.str())
{
}


//
//  Generic base for system errors.
//
typedef Error<struct _SystemError> SystemError;

//
//  Specialized exception type for Win32 errors.
//
typedef Error<struct _Win32Error> Win32Error;

//
//  Specialized exception type for Direct3D errors.
//
typedef Error<struct _Direct3DError, SystemError> Direct3DError; 

//
//  Specialized exception type for Freetype errors.
//
typedef Error<struct _FreetypeError, SystemError> FreetypeError;

//
//  Enable Win32 error info to be attached to an exception.
//
typedef boost::error_info<struct _Win32ErrorInfo, HRESULT> Win32ErrorInfo;

inline Win32ErrorInfo GetLastErrorInfo()
{
    return Win32ErrorInfo(GetLastError());
}

inline std::string to_string(const Win32ErrorInfo& err)
{
    return (boost::format("[0x%08X] %s: %s") % err.value()
                                             % DXGetErrorStringA(err.value())
                                             % DXGetErrorDescriptionA(err.value())).str();
}

//
//  Enable DirectX error info to be attached to an exception.
//
typedef boost::error_info<struct _DirectXErrorInfo, HRESULT> DirectXErrorInfo;

inline std::string to_string(const DirectXErrorInfo& err)
{
    return (boost::format("[0x%08X] %s: %s") % err.value()
                                             % DXGetErrorStringA(err.value())
                                             % DXGetErrorDescriptionA(err.value())).str();
}

//
//  Exception throwing macro.
//
#define THROW(x) BOOST_THROW_EXCEPTION((x))
#define CHECK(e, x) do { if (!(x)) THROW(e("Assert failed: " #x)); } while(0)
#define D3DCHECK(x) do { HRESULT hr; hr = (x); if (FAILED(hr)) THROW(Direct3DError("Assert failed: " #x) << DirectXErrorInfo(hr)); } while(0)
#define WINCHECK(x) do { if (!(x)) THROW(Win32Error("Assert failed: " #x) << GetLastErrorInfo()); } while(0)
#define FTCHECK(x) do { FT_Error err; err = (x); if (err != 0) THROW(FreetypeError("Assert failed: " #x) << (boost::format(" - Error code %d") % err)); } while(0)

//
//  Enable intrusive_ptr to work with COM objects.
//
inline void intrusive_ptr_add_ref(IUnknown* ptr)
{
    ptr->AddRef();
}

inline void intrusive_ptr_release(IUnknown* ptr)
{
    ptr->Release();
}

//
//  Enable intrusive_ptr objects to be passed as parameters to functions expecting
//  a pointer-to-pointer which will be initialized by the function.
//
template <typename T>
class IntrusivePtrWrapper
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] ref
    //          Intrusive_ptr object to wrap.
    //      [in] addRef
    //          If true then the ptr's refcount will be incremented on acquisition.
    //
    IntrusivePtrWrapper(boost::intrusive_ptr<T>& ref, bool addRef);

    //
    //  Destructor.
    //
    ~IntrusivePtrWrapper();

    //
    //  Implicit conversion to T**.
    //
    operator T**();

    //
    //  Implicit conversion to void**.
    //
    operator void**();

private:
    //
    //  Properties.
    //
    boost::intrusive_ptr<T>& m_ref;
    T* m_ptr;
    bool m_addRef;
};

template <typename T>
inline IntrusivePtrWrapper<T>::IntrusivePtrWrapper(boost::intrusive_ptr<T>& ref, bool addRef)
: m_ref(ref), m_addRef(addRef), m_ptr(nullptr)
{
}

template <typename T>
inline IntrusivePtrWrapper<T>::~IntrusivePtrWrapper()
{
    m_ref = boost::intrusive_ptr<T>(m_ptr, m_addRef);
}

template <typename T>
inline IntrusivePtrWrapper<T>::operator T**()
{
    return &m_ptr;
}

template <typename T>
inline IntrusivePtrWrapper<T>::operator void**()
{
    return reinterpret_cast<void**>(&m_ptr);
}

//
//  Helper function for constructing an IntrusivePtrWrapper.
//
//  Parameters:
//      [in] ref
//          Intrusive_ptr object to wrap.
//      [in] addRef
//          If true then the ptr's refcount will be incremented on acquisition.
//          Default is false, which matches the behavior of COM functions.
//
template <typename T>
inline IntrusivePtrWrapper<T> AttachPtr(boost::intrusive_ptr<T>& ref, bool addRef = false)
{
    return IntrusivePtrWrapper<T>(ref, addRef);
}

#endif	// __NYX_PREFIX_H__
