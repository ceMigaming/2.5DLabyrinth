/*
 *	Oparte na filmach i repozytorium użytkownika javidx9 (OneLoneCoder)
 *	Github: https://github.com/OneLoneCoder/CommandLineFPS
 *	YouTube: https://www.youtube.com/watch?v=xW8skO7MFYw
 *	Licencja: https://github.com/OneLoneCoder/Javidx9/blob/master/LICENCE.md
*/

#include <vector>		// vector
#include <utility>		// pary
#include <algorithm>	// sort
#include <chrono>		// czas
#include <Windows.h>	// dojście do konsoli, wypisywanie bufferu ekranu
#include <fstream>		// wczytywanie pliku
#include <codecvt>		// formatowanie pliku w utf-16
#include <string>		// sczytywanie wstringów z pliku

short screenWidth = 120;	// szerokość konsoli w komórkach
short screenHeight = 40;	// wysokość konsoli w komórkach
short mapWidth = 16;		// szerokość mapy w kafelkach
short mapHeight = 16;		// wysokość mapy w kafelkach

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
	// wczytanie obecnych danych o konsoli
	GetConsoleScreenBufferInfo(hConsole, &lpConsoleScreenBufferInfo);
	// sprawdzanie, czy konsola ma wlasciwe wymiary
	if (lpConsoleScreenBufferInfo.dwSize.X != screenWidth - 1 || lpConsoleScreenBufferInfo.dwSize.Y != screenHeight - 1)
	{
		COORD coord = {};
		coord.X = screenWidth - 2;
		coord.Y = screenHeight - 1;

		SetConsoleScreenBufferSize(hConsole, coord);
	}

	// wypełnienie bufferu konsoli pustymi znakami
	for (int i = 0; i < screenWidth * screenHeight; i++)
		screen[i] = L' ';

	// mapa
	std::wstring map;
	// wczytanie mapy z pliku
	std::wifstream wif("./map.txt", std::ios::binary); // wifstream zamiast ifstreamu, bo wczytuje wchar_t/wstring

	bool tryAgain = true;
	while (tryAgain) {
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
					// sprawdzenie czy obecny i następny znak to L'/' i przestaje czytać linijkę, 
					// daje w ten sposób możliwość dodawania komentarzy do pliku "./map.txt".
					if (c == '/' && wline[y + 1] == '/')
						break;
					// jeśli sczytany znak to znacznik gracza, ustawiam pozycje gracza na pozycję znaku w ciągu, następnie zastępujemy znak 'O' znakiem '.'
					else if (c == 'O')
					{
						playerY = (float)y;
						playerX = (float)x - 1.f;
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
		// jesli plik nie istnieje
		else {
			// wyświetlenie powiadomienie o brakującym pliku z poziomem
			int msgboxID = MessageBox(
				NULL,
				(LPCWSTR)L"Nie znaleziono pliku \"map.txt\"\nCzy chcesz spróbować ponownie?",
				(LPCWSTR)L"Nie znaleziono pliku \"map.txt\"",
				MB_ICONWARNING | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2
			);
			switch (msgboxID)
			{
				// jeśli użytkownik chce przerwać działanie programu
			case IDABORT:
				return -1;
				// jeżeli użytkownik chce spróbować ponownie
			case IDTRYAGAIN:
				continue;
				// jeżeli użytkownik chce zignorować ostrzeżenie o brakującym pliku
			case IDIGNORE:
				tryAgain = false;
				// ustawienie domyslnej planszy
				map =
					L"################"
					L"#.............X#"
					L"##.#############"
					L"#.........#....#"
					L"#.#######.#.####"
					L"#.#.....#.#....#"
					L"#.#.#####.####.#"
					L"#.#.......#....#"
					L"###.#.#.#.#.##.#"
					L"#.#.#..####.#.##"
					L"#.#.#..#....#..#"
					L"#.#.#..##.####.#"
					L"#.#.#.....#....#"
					L"#.#.#######....#"
					L"#..............#"
					L"################";
				playerX = 1;
				playerY = 13;
				break;
			}
		}
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

		playerRot = remainder(playerRot, 2 * 3.14159F);


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
		for (int x = 0; x <= screenWidth; x++)
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
			wchar_t rayOutput = ' ';

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
					rayOutput = map[nTestX * mapWidth + nTestY];
					// jeśli ten element to ściana - # lub wyjście - X
					if (rayOutput == L'#' || rayOutput == L'X')
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
			int nCeiling = (int)((float)(screenHeight / 2.0F) - (float)screenHeight / ((float)rayDistance));
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
					if (rayOutput == L'#')
					{
						screen[y * screenWidth + x] = shadingChar;
						lpAttribute[y * screenWidth + x] = nColor;
					}
					// wyjście
					else if (rayOutput == L'X')
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
				if (y * screenWidth + x < 39)
					// odwracanie kolorów, gdy jest taka potrzeba
					if (lpAttribute[y * screenWidth + x] != 255)
						lpAttribute[y * screenWidth + x] |= 0x000f; // negacja koloru
					else lpAttribute[y * screenWidth + x] = 240;
			}
		}

		swprintf_s(screen, 39, L"X=%02.0f, Y=%02.0f, A=%+03.2f, FPS=%03.0f, Time=%03.0f", min(playerX, 99), min(playerY, 99), playerRot, min(1.0f / deltaTime, 999.F), min(runTime, 999.F));

		// wypisywanie minimapy
		for (short nx = 0; nx < mapWidth; nx++)
			for (short ny = 0; ny < mapHeight; ny++)
			{
				screen[(ny + 1) * screenWidth + nx + 1] = map[ny * mapWidth + nx];
				// odwracanie kolorów, gdy jest taka potrzeba
				if (lpAttribute[(ny + 1) * screenWidth + nx + 1] != (short)0x00ff)
					lpAttribute[(ny + 1) * screenWidth + nx + 1] |= 0x000f;
				else lpAttribute[(ny + 1) * screenWidth + nx + 1] = (short)0x00f0;
			}
		// rysowanie gracza na minimapie
		screen[((int)playerX + 1) * screenWidth + (int)playerY] = L'O';

		// zamknięcie buffera znakiem terminacji stringa '\0'
		screen[screenWidth * screenHeight - 1] = '\0';
		lpAttribute[screenWidth * screenHeight - 1] = '\0';

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
		swprintf_s(screen, 39, L"Kliknij 'ESC' aby zamknąć, czas: %5.2f", runTime);
		// zamknięcie buffera znakiem terminacji stringa '\0'
		screen[screenWidth * screenHeight - 1] = '\0';
		lpAttribute[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
		WriteConsoleOutputAttribute(hConsole, lpAttribute, screenWidth * screenHeight, { 0,0 }, &dwBytesChanged);
		// jeśli wciśnięto 'ESC', zakończ
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
			break;
	}

	return 0;
}
