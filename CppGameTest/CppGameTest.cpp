/* Oparte na filmach i repozytorium użytkownika javidx9 (OneLoneCoder)
	Github: https://github.com/OneLoneCoder/CommandLineFPS
	YouTube: https://www.youtube.com/watch?v=xW8skO7MFYw
	Licencja GNU GPLv3: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
*/
#include <vector>		// vector
#include <utility>		// pary
#include <algorithm>	// sort
#include <chrono>		// czas
#include <Windows.h>	// dojście do konsoli, wypisywanie bufferu ekranu
#include <fstream>		// wczytywanie pliku
#include <codecvt>		// formatowanie pliku w utf-16
#include <string>		// sczytywanie wstringów z pliku

int screenWidth = 120;	// szerokość konsoli w komórkach
int screenHeight = 40;	// wysokość konsoli w komórkach
int mapWidth = 16;		// szerokość mapy w kafelkach
int mapHeight = 16;		// wysokość mapy w kafelkach

float playerX = 1.5F;	// pozycja startowa gracza X
float playerY = 13.5F;	// pozycja startowa gracza Y
float playerRot = 0.0F; // startowa rotacja gracza

// pole widzenia gracza
float playerFOV = 3.14159F / 4.0F;
// szybkość obrotu gracza
float playerRotateSpeed = 5.0F;
// zasięg renderowania (maksymalna długość promienia)
float renderDistance = 16.0F;
// Czas od początku gry
float runTime = 0.0F;

// Podstawowe zagadnienia:
// DWORD, WORD i QWORD
// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
// HANDLE
// https://docs.microsoft.com/en-us/windows/console/getstdhandle
// L'c' i L"string" <- wchar_t i wstring (dynamiczny ciąg wchar_t-ów)
// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types

int main()
{
	// uchwyt (dojście) konsoli
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	// ustawienie 'hConsole' jako obecnego buffera
	SetConsoleActiveScreenBuffer(hConsole);

	// informacje na temat buffera
	CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;

	// buffer ekranu
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	// ilość wpisanych bajtów
	DWORD dwBytesWritten = 0;

	// buffer stylu ekranu
	WORD* lpAttribute = new WORD[screenWidth * screenHeight];
	// ilość zmienionych bajtów w bufferze stylu
	DWORD dwBytesChanged = 0;

	// ustawienie trybu konsoli na ENABLE_VIRTUAL_TERMINAL_PROCESSING, czyli umożliwia wpisywanie bezpośrednio do bufferu ekranu i stylu
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	// wypełnia buffer konsoli pustymi znakami
	for (int i = 0; i < screenWidth * screenHeight; i++)
		screen[i] = L' ';

	// sprawdzanie, czy konsola ma wlasciwe wymiary
	while (true)
	{
		// sczytanie informacji o konsoli
		GetConsoleScreenBufferInfo(hConsole, &lpConsoleScreenBufferInfo);
		// sprawdzenie czy konsola ma odpowiednie rozmiary
		if (lpConsoleScreenBufferInfo.dwSize.X == screenWidth && lpConsoleScreenBufferInfo.dwSize.Y == screenHeight) break;
		else
		{
			swprintf_s(screen, 100, L"Zły rozmiar konsoli. Wymagany: %i x %i", lpConsoleScreenBufferInfo.dwSize.X, lpConsoleScreenBufferInfo.dwSize.Y);
			// zamknięcie buffera znakiem terminacji stringa '\0'
			screen[lpConsoleScreenBufferInfo.dwSize.X * lpConsoleScreenBufferInfo.dwSize.Y - 1] = '\0';
			WriteConsoleOutputCharacter(hConsole, screen, lpConsoleScreenBufferInfo.dwSize.X * lpConsoleScreenBufferInfo.dwSize.Y, { 0,0 }, &dwBytesWritten);
		}
	}
	// mapa
	std::wstring map;
	// wczytanie mapy z pliku
	std::wifstream wif("./map.txt", std::ios::binary); // wifstream zamiast ifstreamu, bo wczytuje wchar_t/wstring
	// jeśli plik istnieje
	if (wif.is_open())
	{
		// ustawienie formatowania na 'utf-16'
		wif.imbue(std::locale(wif.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));

		// jedna linijka tekstu na raz
		std::wstring wline;
		// pozycja x w mapie
		int x = 0;
		// dopóki są kolejne linijki
		while (std::getline(wif, wline))
		{
			// y - pozycja y w mapie
			for (int y = 0; y < wline.size(); y++)
			{
				// sczytanie z pliku jednego wchar_t
				auto c = wline[y];
				// sprawdzenie czy obecny i następny znak to L'/' i przestaje czytać linijkę, \
					daje w ten sposób możliwość dodawania komentarzy do pliku "./map.txt".
				if (c == '/' && wline[y + 1] == '/')
					break;
				// jeśli sczytany znak to znacznik gracza, ustawiam pozycje gracza na pozycję znaku w ciągu, następnie zastępujemy znak 'O' znakiem '.'
				else if (c == 'O')
				{
					playerY = y;
					playerX = x-1;
					map += L'.';
				}
				// jeśli sczytany tekst to \r nic nie dodajemy do ciągu z mapą, zwiększamy zmienną x
				else if (c == '\r')
				{
					x++;
					break;
				}
				// dopóki ciąg się nie skończy dodajemy znaki
				else if (c != L'\0')
					map += c;
			}
		}
		
		// zamknięcie strumienia czytania pliku
		wif.close();
	}

	// zapisanie punktów czasowych
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	// główna pętla gry
	while (true)
	{
		// jeśli gracz wszedł w wyjście przejdź do następnej pętli
		if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == 'X')
			break;

		// wyliczenie delty czasu
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float deltaTime = elapsedTime.count();

		// dodanie delty czasu do czasu od rozpoczęcia
		runTime += deltaTime;

		// obrót w lewo
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			playerRot -= (playerRotateSpeed * 0.75F) * deltaTime;

		// obrót w prawo
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			playerRot += (playerRotateSpeed * 0.75F) * deltaTime;

		// chodzenie naprzód, sprawdzenie kolizji
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			playerX += sinf(playerRot) * playerRotateSpeed * deltaTime;;
			playerY += cosf(playerRot) * playerRotateSpeed * deltaTime;;
			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')
			{
				playerX -= sinf(playerRot) * playerRotateSpeed * deltaTime;;
				playerY -= cosf(playerRot) * playerRotateSpeed * deltaTime;;
			}
		}

		// chodzenie w tył, sprawdzenie kolizji
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			playerX -= sinf(playerRot) * playerRotateSpeed * deltaTime;;
			playerY -= cosf(playerRot) * playerRotateSpeed * deltaTime;;
			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')
			{
				playerX += sinf(playerRot) * playerRotateSpeed * deltaTime;;
				playerY += cosf(playerRot) * playerRotateSpeed * deltaTime;;
			}
		}

		// raycasting (śledzenie promieni, uproszczony raytracing)[https://pl.wikipedia.org/wiki/Ray_casting]
		for (int x = 0; x < screenWidth; x++)
		{
			// kąt promienia
			float rayAngle = (playerRot - playerFOV / 2.0F) + ((float)x / (float)screenWidth) * playerFOV;

			// wielkość "kroku"
			float fStepSize = 0.1F;

			// długość promienia
			float rayDistance = 0.0F;

			// czy coś trafiliśmy
			bool rayHit = false;
			// czy trafiono krawędź
			bool hitBoundary = false;
			// trafiony znak
			char rayOutput = ' ';

			// składowa x kąta
			float fEyeX = sinf(rayAngle);
			// składowa y kąta
			float fEyeY = cosf(rayAngle);

			// dopóki nie trafimy czegoś, lub nie osiągniemy limitu
			while (!rayHit && rayDistance < renderDistance)
			{
				// dodajemy wielkość "kroku" do długości promienia, potem obliczamy koordynaty końca promienia.
				rayDistance += fStepSize;
				int nTestX = (int)(playerX + fEyeX * rayDistance);
				int nTestY = (int)(playerY + fEyeY * rayDistance);

				// jeśli wyjdziemy poza mapę, przerywamy.
				if (nTestX < 0 || nTestX >= mapWidth || nTestY < 0 || nTestY >= mapHeight)
				{
					rayHit = true;
					rayDistance = renderDistance;
				}
				// jeśli nie jesteśmy poza zakresem
				else
				{
					// sprawdzamy jaki element znajduje się pod końcem promienia
					rayOutput = map.c_str()[nTestX * mapWidth + nTestY];
					// jeśli ten element to ściana - # lub wyjście - X
					if (rayOutput == '#' || rayOutput == 'X')
					{
						// trafiliśmy cel
						rayHit = true;
						// tworzymy vector par, który będzie przechowywał dystans od elementu i iloczyn skalarny
						std::vector<std::pair<float, float>> p;
						// dodajemy wszystkie krawędzie ściany do vectora.
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - playerY;		// składowa y wektora (fizycznego nie informatycznego) przechowujący odległość od gracza do krawędzi ściany
								float vx = (float)nTestX + tx - playerX;		// składowa x wektora (fizycznego nie informatycznego) przechowujący odległość od gracza do krawędzi ściany
								float d = sqrt(vx * vx + vy * vy);				// odległość od gracza do krawędzi w lini prostej
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);// iloczyn skalarny (cosinus kąta między krawędzią a graczem)
								p.push_back(std::make_pair(d, dot));			// wrzucamy całość na vectora (tym razem informatycznego)
							}

						// sortujemy wektor wg długości
						sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });

						// kąt (w radianach) przy jakim chcemy renderować krawędź ściany
						float fBound = 0.005F;
						// wybieramy 2 najbliższe krawędzie i oznaczamy je do narysowania
						if (acos(p.at(0).second) < fBound) hitBoundary = true;
						if (acos(p.at(1).second) < fBound) hitBoundary = true;
					}
				}
			}

			// wysokość na jakiej rysujemy sufit / niebo
			int nCeiling = (float)(screenHeight / 2.0F) - screenHeight / ((float)rayDistance);
			//	-||-	-||-	-||-	podłogę
			int nFloor = screenHeight - nCeiling;

			// znak którym rysujemy w konsoli
			short shadingChar = ' ';
			// kolor znaku
			short nColor = 0;

			// jeśli odległość promienia jest mniejsza/równa	zasięg / 4	-> znak: █ kolor: 255	(biały), 
			//													zasięg / 3	-> znak: ▓ kolor: 127	(jasno-szary), 
			//													zasięg / 1.5-> znak: ▒ kolor: 143	(ciemniejszy-szary),
			//													zasięg		-> znak: ░ kolor: 0		(czarny)
			// jeśli odległość promienia jest większa niż zasięg, znak zmienia się na ' ', kolor na domyślny (czarny)
			if (rayDistance <= renderDistance / 4.0f) { shadingChar = 0x2588; nColor = 255; }
			else if (rayDistance < renderDistance / 3.0f) { shadingChar = 0x2593; nColor = 127; }
			else if (rayDistance < renderDistance / 1.5f) { shadingChar = 0x2592; nColor = 143; }
			else if (rayDistance < renderDistance) {
				shadingChar = 0x2591; nColor = 0;
			}
			else { shadingChar = ' '; nColor = 0; }

			// krawędzie rysujemy na czarno
			if (hitBoundary) { shadingChar = ' '; nColor = 0; }

			// rysowanie komórek w pionie
			for (int y = 0; y < screenHeight; y++)
			{
				// jeśli rysujemy sufit / niebo
				if (y <= nCeiling)
				{
					screen[y * screenWidth + x] = ' ';
					lpAttribute[y * screenWidth + x] = BACKGROUND_BLUE;
				}
				// jeśli rysujemy ściany lub wyjście
				else if (y > nCeiling && y <= nFloor)
				{
					// ściana
					if (rayOutput == '#')
					{
						screen[y * screenWidth + x] = shadingChar;
						lpAttribute[y * screenWidth + x] = nColor;
					}
					// wyjście
					else if (rayOutput == 'X')
					{
						screen[y * screenWidth + x] = 'X';
						lpAttribute[y * screenWidth + x] = 0x0000 | FOREGROUND_RED;
					}
				}
				// jeśli rysujemy podłogę
				else
				{
					float b = 1.0f - (((float)y - screenHeight / 2.0F) / ((float)screenHeight / 2.0F));
					if (b < 0.25) { shadingChar = '#'; }
					else if (b < 0.5) { shadingChar = 'x'; }
					else if (b < 0.75) { shadingChar = '.'; }
					else if (b < 0.9) { shadingChar = '-'; }
					else { shadingChar = ' '; }
					screen[y * screenWidth + x] = shadingChar;
					lpAttribute[y * screenWidth + x] = BACKGROUND_GREEN;
				}

				// rysowanie informacji o grze
				if (y * screenWidth + x < 47)
					// odwracanie kolorów, gdy jest taka potrzeba
					if (lpAttribute[y * screenWidth + x] != 255)
						lpAttribute[y * screenWidth + x] |= 0x000f; // negacja koloru
					else lpAttribute[y * screenWidth + x] = 240;
			}
		}

		// wypisywanie danych o grze.
		swprintf_s(screen, 60, L"X=%2.2f, Y=%2.2f, A=%3.2f, FPS=%3.2f, Time=%3.2f\0", playerX, playerY, playerRot, 1.0f / deltaTime, runTime);

		// wypisywanie minimapy
		for (int nx = 0; nx < mapWidth; nx++)
			for (int ny = 0; ny < mapWidth; ny++)
			{
				screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
				// odwracanie kolorów, gdy jest taka potrzeba
				if (lpAttribute[(ny + 1) * screenWidth + nx] != 255)
					lpAttribute[(ny + 1) * screenWidth + nx] |= 0x000f;
				else lpAttribute[(ny + 1) * screenWidth + nx] = 240;
			}
		// rysowanie gracza na minimapie
		screen[((int)playerX + 1) * screenWidth + (int)playerY] = L'O';

		// zamknięcie buffera znakiem terminacji stringa '\0'
		screen[screenWidth * screenHeight - 1] = '\0';
		// wpisanie bufferu ekranu bezpośrednio do konsoli
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
		// wpisanie bufferu stylu bezpośrednio do konsoli
		WriteConsoleOutputAttribute(hConsole, lpAttribute, screenWidth * screenHeight, { 0,0 }, &dwBytesChanged);
	}

	// wyczyszczenie ekranu, ustawienie tła na czarne, liter na zielone
	for (int i = 0; i < screenWidth * screenHeight; i++)
	{
		screen[i] = ' ';
		lpAttribute[i] = 0x0000 | FOREGROUND_GREEN;
	}

	// wypisanie czasu, koniec gry
	while (true)
	{
		swprintf_s(screen, 45, L"Kliknij 'ESC' aby zamknąć, czas: %5.2f", runTime);
		// zamknięcie buffera znakiem terminacji stringa '\0'
		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth* screenHeight, { 0,0 }, & dwBytesWritten);
		WriteConsoleOutputAttribute(hConsole, lpAttribute, screenWidth* screenHeight, { 0,0 }, & dwBytesChanged);
		// jeśli wciśnięto 'ESC', zakończ
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
			break;
	}

	return 0;
}