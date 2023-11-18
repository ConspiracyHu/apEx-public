#include "WindowHandler.h"
#include "../LibCTiny/libcminimal.h"

HWND hWnd;
extern IDXGISwapChain *SwapChain;

LRESULT CALLBACK WndProc( HWND HWND, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    //case WM_ACTIVATE:				
    //	{
    //		coreWindow->Active=(wParam&0xFFFF)!=0;
    //		return 0;					
    //	}

  case WM_SYSCOMMAND:
  {
    switch ( wParam )
    {
      //case SC_SCREENSAVE:
    case SC_MONITORPOWER:
      return 0;
    }
    break;
  }

  case WM_KEYDOWN:
  {
    if ( wParam == VK_ESCAPE ) phxDone = true;
    break;
  }

  case WM_CLOSE:
  {
    phxDone = true;
    return 0;
  }
  }
  return DefWindowProcA( HWND, uMsg, wParam, lParam );
}

#ifdef PHOENIX_ENGINE_LINEAR_RENDER
static DXGI_SWAP_CHAIN_DESC scd = { 0, 0, 0, 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED, 1, 0, DXGI_USAGE_RENDER_TARGET_OUTPUT, 1, 0, 0, DXGI_SWAP_EFFECT_DISCARD, 0 };
#else
static DXGI_SWAP_CHAIN_DESC scd = { 0, 0, 0, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED, 1, 0, DXGI_USAGE_RENDER_TARGET_OUTPUT, 1, 0, 0, DXGI_SWAP_EFFECT_DISCARD, 0 };
#endif

void SetDenominator( int denom )
{
  scd.BufferDesc.RefreshRate.Denominator = denom;
}

void InitWindow( HINSTANCE h, int XRes, int YRes, int refreshrate, bool FullScreen, HICON Icon, bool antialias )
{
  static const char* clss = "PhoeniX";

  WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS, WndProc, 0, 0, h, Icon, 0, 0, 0, clss };
  //WNDCLASS wc;
  //memset(&wc, 0, sizeof(wc));
  //wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  //wc.lpfnWndProc = WndProc;
  //wc.hInstance = h;
  //wc.hIcon = Icon;
  //wc.lpszClassName = "PhoeniX";
  RegisterClass( &wc );

  RECT WindowRect = { 0, 0, XRes, YRes };

  DWORD dwStyle = !FullScreen ? WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED | WS_MINIMIZEBOX : WS_POPUP | WS_OVERLAPPED;
  AdjustWindowRect( &WindowRect, dwStyle, FALSE );

  hWnd = CreateWindowA( clss, nullptr, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, h, NULL );

  scd.BufferDesc.RefreshRate.Numerator = refreshrate;
  scd.OutputWindow = (HWND)hWnd;
  scd.Windowed = !FullScreen;
  scd.BufferDesc.Width = XRes;
  scd.BufferDesc.Height = YRes;

/*
#ifdef _DEBUG
  HRESULT res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &scd, &SwapChain, &phxDev, NULL, &phxContext );
  if ( res != S_OK )
  {
    DEBUGLOG( "FAILED TO INITIALIZE DEBUG DEVICE!" );
#endif
*/
    D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &scd, &SwapChain, &phxDev, NULL, &phxContext );
/*
#ifdef _DEBUG
  }
#endif
*/

  ID3D11Texture2D *bb;
  SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&bb );
  phxDev->CreateRenderTargetView( bb, NULL, &phxBackBufferView );
  //bb->Release();

  if ( FullScreen ) ShowCursor( false );

  ShowWindow( hWnd, SW_SHOWNORMAL );
  SetForegroundWindow( hWnd );
  SetFocus( hWnd );
}


