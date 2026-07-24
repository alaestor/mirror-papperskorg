#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __STDC_NO_ATOMICS__
	#error this implementation needs atomics
#endif
#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef __GNUC__
	#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
	#define UNUSED(x) UNUSED_ ## x
#endif




typedef void *(*KeyEventCallback_t)(uint8_t keyCode, bool isKeyDownEvent);

struct InvokeCallbackOnInput_s
{
	uint8_t keyCode_condition; // 0 for any
	enum KeyState_e { any, up, down } keyState_condition;
	KeyEventCallback_t callback;
};

an error

/*
bool
//f1:1,f2:1,f3:1,f4:1,f5:1,f6:1,f7:1,f8:1,f9:1,
zero:1,one:1,two:1,three:1,four:1,five:1,six:1,seven:1,eight:1,nine:1,
a:1,b:1,c:1,d:1,e:1,f:1,h:1,i:1,j:1,k:1,l:1,m:1,n:1,o:1,p:1,q:1,r:1,
s:1,t:1,u:1,v:1,w:1,x:1,y:1,z:1,minus:1,plus:1,leftbracket:1,
rightbracket:1,backslash:1,semicolon:1,apostrophy:1,comma:1,period:1,
forwardslash:1,astrisk:1,esc:1,grave:1,tab:1,capslock:1,shift:1,
ctrl:1,alt:1,backspace:1,enter:1,up:1,down:1,left:1,right:1,
leftclick:1,rightclick:1;
//*/

static union State_u
{
	atomic_uint_fast32_t state;
	struct
	{
		bool
			w:1,a:1,s:1,d:1,q:1,e:1,f:1,r:1,space:1,
			shift:1,control:1,alt:1,
			rightclick:1,leftclick:1,
			reserved_one:1,
			reserved_two:1;
	} key;
	struct
	{
		const uint16_t padding;
	} reserved;

} global_State = { 0 };

atomic_uint_fast32_t* const atomic_State_ptr = &global_State.state;

_Static_assert(
	sizeof(union State_u) == sizeof(atomic_uint_fast32_t),
	"invalid size"
);

void macroBehaviorEngine(void)
{
	union State_u local;
	local.state = atomic_load(atomic_State_ptr);

	//some behavior
	(void)local;

	return;
}

#if defined(_WIN64) // ------------------------------------------------ Windows
#define WINVER _WIN32_WINNT_WIN10
#define WIN32_LEAN_AND_MEAN
#define NO_STRICT // FUCK OFF WINDOWS
#include <windows.h>

int64_t CALLBACK LowLevelKeyboardProc(
	const int    nCode,
	const WPARAM wParam,
	const LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	// load copy of global state
	union State_u local;
	local.state = 0;

	bool keyState = false;
	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
		keyState = true;
	}
	else if(wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
	{
		keyState = false;
	}
	else
	{
		fprintf(stderr, "LLKBH got an unexpected message.");
	}

	bool swallowMsg = false;
	const KBDLLHOOKSTRUCT* const kbhs_ptr = (PKBDLLHOOKSTRUCT)lParam;
	switch (kbhs_ptr->vkCode)
	{
	case 'W': local.key.w = keyState; break;
	case 'A': local.key.a = keyState; break;
	case 'S': local.key.s = keyState; break;
	case 'D': local.key.d = keyState; break;
	case 'Q': local.key.q = keyState; break;
	case 'E': local.key.e = keyState; break;
	case 'F': local.key.f = keyState; break;
	case 'R': local.key.r = keyState; break;
	case VK_SPACE: local.key.space = keyState; break;
	case VK_MENU: local.key.alt = keyState; break;
	case VK_SHIFT:
	case VK_RSHIFT:
	case VK_LSHIFT: local.key.shift = keyState; break;
	default: /*some other key*/ break;
	}

	// update global state
	atomic_store(atomic_State_ptr, local.state | atomic_load(atomic_State_ptr));

	return swallowMsg ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
}

void* macroController(void* UNUSED(param))
{
	for(;;)
	{
		macroBehaviorEngine();
		Sleep(0);
	}
}

int Run(void)
{
	printf("Hello, World!\n");

	pthread_t behaviorEngine;
	if (pthread_create(&behaviorEngine, NULL, macroController, NULL) == -1)
	{
		const char* eCode = "unknown error";
		switch (errno)
		{
		case EAGAIN: eCode = "EAGAIN"; break;
		case EINVAL: eCode = "EINVAL"; break;
		case ENOMEM: eCode = "ENOMEM"; break;
		default:break;
		}
		fprintf(stderr, "pthread_create() failed: %i , %s\n", errno, eCode);
	}

	HHOOK kbh_handle = SetWindowsHookExA(
		WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

	if (kbh_handle != NULL)
	{
		MSG msg;
		while (!GetMessageA(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UnhookWindowsHookEx(kbh_handle);
	}
	else
	{
		fprintf(stderr, "SetWindowsHookExA() failed: %lu \n", GetLastError());
		goto exit;
	}

	exit:
	return 0;
}
#else
#error Only supports Windows 10
#endif //-------------------------------------------------------------- Windows

int main(void)
{
	printf("size of State_u =  %llu\n", sizeof(union State_u));
	return Run();
}
