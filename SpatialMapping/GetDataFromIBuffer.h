//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <streambuf>
#include <robuffer.h>
#include <wrl.h>

#include <winrt/Windows.Storage.Streams.h>

using namespace Microsoft::WRL;
using namespace winrt::Windows::Storage::Streams;
namespace DX {
	//template <typename t = byte>
	byte* GetDataFromIBuffer(winrt::Windows::Storage::Streams::IBuffer container)
	{
		if (container == NULL)
		{
			return nullptr;
		}

		unsigned int bufferLength = container.Length();

		if (!(bufferLength > 0))
		{
			return nullptr;
		}

		HRESULT hr = S_OK;
		ComPtr<IUnknown> pUnknown = reinterpret_cast<::IUnknown*>(winrt::get_abi(container));
		//ComPtr<IUnknown> pUnknown = reinterpret_cast<IUnknown*>(container);
		ComPtr<Windows::Storage::Streams::IBufferByteAccess> spByteAccess;
		hr = pUnknown.As(&spByteAccess);
		if (FAILED(hr))
		{
			return nullptr;
		}

		byte* pRawData = nullptr;
		hr = spByteAccess->Buffer(&pRawData);
		if (FAILED(hr))
		{
			return nullptr;
		}
		return pRawData;
		//return reinterpret_cast<t*>(pRawData);
	}
}