/*** NOTE:
	mmdeviceapi (et al?) requires linking against `uuid`
	if linker errors when using GCC, make sure youre passing `-luuid`

	This 'project' lacks structured documentation.

	I explain nothing and code pedantically. May god have mercy on you if
	you're trying to use this as a learning resource.

	There's error-checking, but no error-handling or reporting. You'll get a
	location as to what function threw the exception, but no description or
	Win32 HRESULT error information because I enjoy making people suffer
	(And by that, I mean I'm lazy).
*/

// import std;
// god fucking damn it...
#include <chrono> // seconds
#include <thread> // sleep_for
#include <stdexcept> // runtime_error
#include <memory> // unique_ptr
#include <source_location> // take a guess

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h> // COM nonsense
#include <initguid.h> // CLSID_MMDeviceEnumerator, IID_IMMDeviceEnumerator
#include <mmdeviceapi.h> // IMMDeviceEnumerator
#include <endpointvolume.h> // IAudioEndpointVolume

#define THROW_UNSPECIFIC_RUNTIME_ERROR \
	throw std::runtime_error(std::source_location::current().function_name())

template <class T>
using release_wrap = std::unique_ptr<T,
decltype([](T* const p) noexcept
		{
			if (p != nullptr)
				p->Release();
		})>;

// enjoy your wrap, sir
template <class T>
requires requires (T t) { t.Release(); }
[[nodiscard]] release_wrap<T> wrap(T* const resource) noexcept
{
	return release_wrap<T>{ resource };
};

struct raii_com
{
	[[nodiscard]] explicit raii_com(
		const tagCOINIT tag = COINIT_APARTMENTTHREADED)
	{
		if (CoInitializeEx(nullptr, tag) != S_OK) throw std::runtime_error(
			"A raii_com instance failed to initialize :(");
	}

	~raii_com() noexcept
	{ CoUninitialize(); }

	/// Only for RAII convenience; not idiot-proof
	template <class T>
	[[nodiscard]] release_wrap<T> create(
		const GUID clsid,
		const IID& iid,
		const tagCLSCTX clsctx = CLSCTX_INPROC_SERVER
	) const
	{
		if (T* new_resource;
			CoCreateInstance(
				clsid,
				nullptr,
				clsctx,
				iid,
				reinterpret_cast<void**>(&new_resource)
			) == S_OK)
		{
			return wrap(new_resource);
		}
		else THROW_UNSPECIFIC_RUNTIME_ERROR;
	}
};

// Not idiot-proof; doesn't validate EDataFlow range.
// param edf `eCapture` for input device (microphone), `eRender` for output.
[[nodiscard]] release_wrap<IMMDevice>
default_audio_endpoint(const raii_com& com, const EDataFlow edf)
{
	const auto device_enumerator{
		com.create<IMMDeviceEnumerator>(
			CLSID_MMDeviceEnumerator, IID_IMMDeviceEnumerator
		)
	};

	if (IMMDevice* p;
		device_enumerator->GetDefaultAudioEndpoint(edf, eConsole, &p) == S_OK)
	{
		return wrap(p);
	}
	else THROW_UNSPECIFIC_RUNTIME_ERROR;
}

[[nodiscard]] release_wrap<IAudioEndpointVolume>
audio_endpoint_volume(const release_wrap<IMMDevice>& device)
{
	if (IAudioEndpointVolume* p;
		device->Activate(
			__uuidof(IAudioEndpointVolume),
			CLSCTX_LOCAL_SERVER,
			nullptr,
			reinterpret_cast<void**>(&p)
		) == S_OK)
	{
		return wrap(p);
	}
	else THROW_UNSPECIFIC_RUNTIME_ERROR;
}

[[nodiscard]] release_wrap<IAudioEndpointVolume>
default_microphone_volume_endpoint(const raii_com& com)
{
	return audio_endpoint_volume(default_audio_endpoint(com, eCapture));
}

// #include <iostream>

int main()
{
	const raii_com com;
	const auto mic_volume_ctrl{ default_microphone_volume_endpoint(com) };
	for (;;)
	{
		// float volume = 0.0;
		// mic_volume_ctrl->GetMasterVolumeLevelScalar(&volume);
		// std::cout << " mic volume is: " << volume << "\n";
		mic_volume_ctrl->SetMasterVolumeLevelScalar(1.0, nullptr);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
