/* Albert Skalinski - D00248346
   Dylan Fennelly - D00248176 */

#include <SFML/Graphics.hpp>
#include "ResourceIdentifiers.hpp"
#include <windows.h>  // Required for WinMain
#include "Application.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		Application app;
		app.Run();
	}
	catch (std::runtime_error& e)
	{
		MessageBoxA(nullptr, e.what(), "Application Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}
