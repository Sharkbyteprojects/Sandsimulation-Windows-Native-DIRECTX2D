#include "scene.hpp"
#include "sandph.hpp"

class Scene : public GraphicsScene
{
	CComPtr<ID2D1SolidColorBrush> m_pFill;
	CComPtr<ID2D1SolidColorBrush> m_pStroke;
	CComPtr<ID2D1SolidColorBrush> m_stC;

	D2D_RECT_F rect;

	HRESULT CreateDeviceIndependentResources() { return S_OK; }
	void    DiscardDeviceIndependentResources() { }
	HRESULT CreateDeviceDependentResources();
	void    DiscardDeviceDependentResources();
	void    CalculateLayout();
	void    RenderScene();
public:
	HWND hwnd;
	POINT mousePos, pc;
};

HRESULT Scene::CreateDeviceDependentResources()
{
	HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(1.f, .7f, 0),
		D2D1::BrushProperties(),
		&m_pFill
	);

	if (SUCCEEDED(hr))
	{
		hr = m_pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(0, 1, 1),
			D2D1::BrushProperties(),
			&m_pStroke
		);
		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Yellow),
				D2D1::BrushProperties(),
				&m_stC
			);
		}
	}
	return hr;
}

POINT mouser(HWND myh) {
	POINT p;
	if (GetCursorPos(&p))
	{
		ScreenToClient(myh, &p);
		return p;
	}
	p.x = 0;
	p.y = 0;
	return p;
}

Csand sandbox;
bool mb = false;

void Scene::RenderScene()
{
	D2D1_SIZE_F fSize = m_pRenderTarget->GetSize();
	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	if (mb)	mousePos = mouser(hwnd);
	pc.x = (fSize.width / sandx);
	pc.y = (fSize.height / sandy);
	for (size_t x = 0; x < sandx; x++) {
		for (size_t y = 0; y < sandy; y++) {
			if (sandbox.sand[x][y] == true) {
				m_pRenderTarget->SetTransform(
					D2D1::Matrix3x2F::Translation(x * pc.x, y * pc.y)
				);
				m_pRenderTarget->FillRectangle(rect, m_stC);
			}
			else if (mb) {
				if (
					x * pc.x <= mousePos.x && (x * pc.x) + pc.x >= mousePos.x &&
					y * pc.y <= mousePos.y && (y * pc.y) + pc.y >= mousePos.y
					) {
					sandbox.sand[x][y] = true;
				}
			}
		}
	}
	sandbox.step();
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

void Scene::CalculateLayout()
{
	D2D1_SIZE_F fSize = m_pRenderTarget->GetSize();

	rect.top = 0;
	rect.left = 0;
	rect.bottom = fSize.height / sandy;
	rect.right = fSize.width / sandx;
}


void Scene::DiscardDeviceDependentResources()
{
	m_pFill.Release();
	m_pStroke.Release();
}


class MainWindow : public BaseWindow<MainWindow>
{
	HANDLE  m_hTimer;
	Scene m_scene;

	BOOL    InitializeTimer();

public:

	MainWindow() : m_hTimer(NULL)
	{
	}

	void    WaitTimer();

	PCWSTR  ClassName() const { return L"SandSim Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Constants 
const WCHAR WINDOW_NAME[] = L"Sandsimulation";


INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, INT nCmdShow)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return 0;
	}

	MainWindow win;

	if (!win.Create(WINDOW_NAME, WS_OVERLAPPEDWINDOW))
	{
		return 0;
	}
	sandbox.clearsand();
	ShowWindow(win.Window(), nCmdShow);
	// Run the message loop.

	MSG msg = { };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		win.WaitTimer();
	}

	CoUninitialize();
	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = m_hwnd;

	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(m_scene.Initialize()) || !InitializeTimer())
		{
			return -1;
		}
		m_scene.hwnd = hwnd;
		return 0;

	case WM_DESTROY:
		CloseHandle(m_hTimer);
		m_scene.CleanUp();
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	case WM_DISPLAYCHANGE:
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);
		m_scene.Render(m_hwnd);
		EndPaint(m_hwnd, &ps);
	}
	return 0;

	case WM_SIZE:
	{
		int x = (int)(short)LOWORD(lParam);
		int y = (int)(short)HIWORD(lParam);
		m_scene.Resize(x, y);
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
	return 0;

	case WM_LBUTTONDOWN:
		mb = true;
		return 0;
	case WM_RBUTTONDOWN:
		sandbox.clearsand();
		return 0;
	case WM_LBUTTONUP:
		mb = false;
		return 0;

	case WM_ERASEBKGND:
		return 1;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


BOOL MainWindow::InitializeTimer()
{
	m_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (m_hTimer == NULL)
	{
		return FALSE;
	}

	LARGE_INTEGER li = { 0 };

	if (!SetWaitableTimer(m_hTimer, &li, (1000 / 60), NULL, NULL, FALSE))
	{
		CloseHandle(m_hTimer);
		m_hTimer = NULL;
		return FALSE;
	}

	return TRUE;
}

void MainWindow::WaitTimer()
{
	// Wait until the timer expires or any message is posted.
	if (MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT)
		== WAIT_OBJECT_0)
	{
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}
