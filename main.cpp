#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <direct.h>
#include <Windows.h>
extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}
const Uint8* state = SDL_GetKeyboardState(NULL);//zwraca wskaznik do tablicy stanow
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	800

struct ruchy
{
	int gora = 0;
	int dol = 0;
	int prawo = 0;
	int lewo = 0;
};
struct SDL
{

	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* car, * grass, * tree, * wybuch, * pauzagraf, * przeciwnik, * cywil;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};
struct kolory
{
	int czarny;
	int zielony;
	int czerwony;
	int niebieski;
	int kolor_drogi;
	int kolor_wypelnienia;
	int kolor_obramowania;
	int kolor_kraweznika;
};
struct gra
{
	float speed = 1.0, czas_pauzy = 0;
	int pozycje_otoczenia[8][2] = { {20,100},{40,200},{60,450},{560,40},{550,400 },{590,300},{32,260},{570,160} };
	double pozycja_startowa_x = SCREEN_WIDTH / 2, pozycja_startowa_y = SCREEN_HEIGHT / 1.40, wynik = 0.0;
	int	przemieszczenie = 0, czas_wybuchu = 0, czas_nowej_gry = 0, pauza = 0, liczba_zapisow = 0;
	double pozycja_przeciwnika_x = 200, pozycja_przeciwnika_y = 0, pozycja_cywila_x = 350, pozycja_cywila_y = -5;
	int czy_przeciwnik_w_dol = 0, czas_pojawienia_wroga = 0, czas_pojawienia_cywila = 0;
	int czy_uderzenie_cywila = 0, zycia_wroga = 150;
};
struct czas_gry
{
	double delta = 0, worldTime = 0, fpsTimer = 0, fps = 0, distance = 0;
	int t1 = 0, t2 = 0, quit = 0, frames = 0, rc = 0;
};
struct pocisk
{
	double pozycja_pocisku_x = 0, pozycja_wybuchu_x = 0;
	double pozycja_pocisku_y = 0, pozycja_wybuchu_y = 0;
	double pozycja_startowa_pocisku_x = 0;
	double pozycja_startowa_pocisku_y = 0;
	int czy_wystrzelony_pocisk = 0, czas_ekspozji = 0;

};
void nowa_gra(SDL SDL, gra& gra, czas_gry& czas_gry);
void przemieszczenie_w_lewo(double& pozycja_startowa_x);
void przemieszczenie_w_prawo(double& pozycja_startowa_x);
void czytaj_z_pliku(char* nazwa_pliku, gra& gra, czas_gry& czas_gry);
void strzal_do_cywila(pocisk& pocisk, gra& gra, SDL SDL, kolory kolory);
void strzal_do_wroga(pocisk& pocisk, gra& gra, SDL SDL, kolory kolory);
void wybuch_pojazdow(SDL SDL, pocisk& pocisk, gra gra);
void strzal(SDL SDL, gra& gra, kolory kolory, pocisk& pocisk);
void ruch_cywila(SDL SDL, gra& gra);
void ruch_przeciwnika(SDL SDL, gra& gra);
void wczytaj2(SDL SDL, gra& gra, kolory kolory, czas_gry& czas_gry);
void wczytaj(gra& gra);
void wczytaj(gra& gra);
void plynny_ruch(ruchy& ruchy, gra& gra);
void jaki_klawisz_zwolniony(ruchy& ruchy);
void przemieszczenie_w_gore(double& pozycja_startowa_y);
void przemieszczenie_w_dol(double& pozycja_startowa_y);
void rysowanie_drogi(SDL SDL, int x, int y, int l, int k, kolory kolory);
void ruch(SDL SDL, gra& gra);
void save(gra gra, czas_gry czas_gry);
void czy_wypadek(SDL SDL, gra& gra);
void wypisz_legende(SDL_Surface* screen, const char* text, SDL_Surface* charset);
void obsluga_klawiszy(SDL SDL, ruchy& ruchy, gra& gra, czas_gry& czas_gry, kolory kolory, pocisk& pocisk);
void naliczanie_pkt(gra& gra);
void czas_i_fps(czas_gry& czas_gry);
void warstwa_wizualna(SDL SDL, gra gra, char* text, czas_gry czas, kolory kolory);
void jezeli_zdjecie_charset_null(SDL SDL);
void DrawString(SDL_Surface* screen, double x, double y, const char* text,
	SDL_Surface* charset) {
	int c, px, py;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = (int)(x);
		d.y = (int)(y);
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt ?rodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, double x, double y) {
	SDL_Rect dest;
	dest.x = (int)(x)-sprite->w / 2;
	dest.y = (int)(y)-sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d?ugo?ci l w pionie (gdy dx = 0, dy = 1) 
// b?d? poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok?ta o d?ugo?ci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int rc;
	ruchy ruchy;
	SDL SDL;
	pocisk pocisk;

	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &SDL.window, &SDL.renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &SDL.window, &SDL.renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(SDL.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(SDL.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(SDL.window, "Hubert Szymczak");

	SDL.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL.scrtex = SDL_CreateTexture(SDL.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// wy??czenie widoczno?ci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	SDL.charset = SDL_LoadBMP("./cs8x8.bmp");

	SDL_SetColorKey(SDL.charset, true, 0x000000);
	kolory kolory;
	kolory.czarny = SDL_MapRGB(SDL.screen->format, 0x00, 0x00, 0x00);
	kolory.zielony = SDL_MapRGB(SDL.screen->format, 88, 135, 58);
	kolory.czerwony = SDL_MapRGB(SDL.screen->format, 0xFF, 0x00, 0x00);
	kolory.niebieski = SDL_MapRGB(SDL.screen->format, 0x11, 0x11, 0xCC);
	kolory.kolor_drogi = SDL_MapRGB(SDL.screen->format, 48, 48, 48);
	kolory.kolor_wypelnienia = SDL_MapRGB(SDL.screen->format, 26, 26, 26);
	kolory.kolor_obramowania = SDL_MapRGB(SDL.screen->format, 232, 209, 2);
	kolory.kolor_kraweznika = SDL_MapRGB(SDL.screen->format, 20, 20, 19);


	SDL.car = SDL_LoadBMP("./car2.bmp");
	SDL.grass = SDL_LoadBMP("./grass.bmp");
	SDL.tree = SDL_LoadBMP("./tree.bmp");
	SDL.wybuch = SDL_LoadBMP("./wybuch.bmp");
	SDL.pauzagraf = SDL_LoadBMP("./pauza.bmp");
	if (SDL.charset == NULL || SDL.car == NULL || SDL.grass == NULL || SDL.tree == NULL || SDL.wybuch == NULL || SDL.pauzagraf == NULL) {
		jezeli_zdjecie_charset_null(SDL);
	}

	char text[128];
	gra gra;
	czas_gry czas_gry;
	czas_gry.t1 = SDL_GetTicks();

	while (!czas_gry.quit) {
		czas_gry.t2 = SDL_GetTicks();
		if (gra.pauza == 0)
		{
			czas_i_fps(czas_gry);
			SDL_FillRect(SDL.screen, NULL, kolory.zielony);
			rysowanie_drogi(SDL, SCREEN_WIDTH / 4, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, kolory);
			ruch(SDL, gra);
			naliczanie_pkt(gra);
			if (pocisk.czy_wystrzelony_pocisk > 0)strzal(SDL, gra, kolory, pocisk);
			DrawSurface(SDL.screen, SDL.car, gra.pozycja_startowa_x, gra.pozycja_startowa_y);
			czy_wypadek(SDL, gra);
			wybuch_pojazdow(SDL, pocisk, gra);
			nowa_gra(SDL, gra, czas_gry);
			ruch_przeciwnika(SDL, gra);
			ruch_cywila(SDL, gra);
		}
		else if (gra.pauza == 1)
		{
			DrawSurface(SDL.screen, SDL.pauzagraf, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			czas_gry.t1 = czas_gry.t2;
		}
		else
		{
			wczytaj2(SDL, gra, kolory, czas_gry);
			gra.pauza = 0;
		}
		warstwa_wizualna(SDL, gra, text, czas_gry, kolory);

		SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
		SDL_RenderPresent(SDL.renderer);
		// obs?uga zdarze? (o ile jakie? zasz?y) / handling of events (if there were any)
		while (SDL_PollEvent(&SDL.event)) {

			obsluga_klawiszy(SDL, ruchy, gra, czas_gry, kolory, pocisk);
		}
		plynny_ruch(ruchy, gra);
		czas_gry.frames++;
	}

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(SDL.charset);
	SDL_FreeSurface(SDL.screen);
	SDL_DestroyTexture(SDL.scrtex);
	SDL_DestroyRenderer(SDL.renderer);
	SDL_DestroyWindow(SDL.window);

	SDL_Quit();
	return 0;
}
void przemieszczenie_w_gore(double& pozycja_startowa_y)
{
	pozycja_startowa_y--;

}
void przemieszczenie_w_dol(double& pozycja_startowa_y)
{
	pozycja_startowa_y++;
}
void przemieszczenie_w_lewo(double& pozycja_startowa_x)
{
	pozycja_startowa_x--;
}
void przemieszczenie_w_prawo(double& pozycja_startowa_x)
{
	pozycja_startowa_x++;
}
void rysowanie_drogi(SDL SDL, int x, int y, int l, int k, kolory kolory)
{

	for (int i = y + 1; i < y + k; i++)
		DrawLine(SDL.screen, x + 1, i, l - 2, 1, 0, kolory.kolor_drogi);
	for (int i = 0; i < 6; i++)
	{
		DrawLine(SDL.screen, x + i, y, k, 0, 1, kolory.kolor_kraweznika);
		DrawLine(SDL.screen, x + l - i, y, k, 0, 1, kolory.kolor_kraweznika);
		DrawLine(SDL.screen, SCREEN_WIDTH / 2 - i - 5, y, k, 0, 1, kolory.kolor_obramowania);
		DrawLine(SDL.screen, SCREEN_WIDTH / 2 + i + 5, y, k, 0, 1, kolory.kolor_obramowania);
	}
}
void ruch(SDL SDL, gra& gra)
{
	for (int i = 0; i < 8; ++i)
	{
		if (gra.pozycje_otoczenia[i][1] > SCREEN_HEIGHT)gra.pozycje_otoczenia[i][1] = 0;

		if (gra.speed == 1.0)
		{


			if (i > 5)DrawSurface(SDL.screen, SDL.tree, gra.pozycje_otoczenia[i][0], gra.pozycje_otoczenia[i][1]++);
			else
			{
				DrawSurface(SDL.screen, SDL.grass, gra.pozycje_otoczenia[i][0], gra.pozycje_otoczenia[i][1]++);
			}
		}
		else if (gra.speed == 2.0)
		{

			if (i > 5)DrawSurface(SDL.screen, SDL.tree, gra.pozycje_otoczenia[i][0], gra.pozycje_otoczenia[i][1] += 2);
			else
			{
				DrawSurface(SDL.screen, SDL.grass, gra.pozycje_otoczenia[i][0], gra.pozycje_otoczenia[i][1] += 2);
			}

		}
		else if (gra.speed == 3.0)
		{
			if (i > 5)DrawSurface(SDL.screen, SDL.tree, gra.pozycje_otoczenia[i][0], (gra.pozycje_otoczenia[i][1]++));
			else
			{
				DrawSurface(SDL.screen, SDL.grass, gra.pozycje_otoczenia[i][0], gra.pozycje_otoczenia[i][1]++);
			}
		}

	}


}
void czy_wypadek(SDL SDL, gra& gra)
{
	if (gra.czas_wybuchu <= 300 && gra.czas_wybuchu > 0)
	{
		DrawSurface(SDL.screen, SDL.wybuch, gra.pozycja_startowa_x, gra.pozycja_startowa_y);
		DrawString(SDL.screen, gra.pozycja_startowa_x + 50, gra.pozycja_startowa_y, "-1000pkt", SDL.charset);
		gra.czas_wybuchu--;
	}
	if (gra.pozycja_startowa_x < SCREEN_WIDTH / 4 - 35)
	{
		gra.pozycja_startowa_x = SCREEN_WIDTH / 2;
		gra.pozycja_startowa_y = SCREEN_HEIGHT / 1.40;
		gra.czas_wybuchu = 300;
		gra.wynik -= 1000;
	}
	else if (gra.pozycja_startowa_x > 3 * SCREEN_WIDTH / 4 + 35)
	{
		gra.pozycja_startowa_x = SCREEN_WIDTH / 2;
		gra.pozycja_startowa_y = SCREEN_HEIGHT / 1.40;
		gra.czas_wybuchu = 300;
		gra.wynik -= 1000;
	}
}
void wypisz_legende(SDL_Surface* screen, const char* text, SDL_Surface* charset)
{
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 95, "esc - wyjscie", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 85, "\030\031 - ruch", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 75, "\032\033 - ruch", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 65, "space - shoot", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 55, "n - nowa gra", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 45, "p - pauza", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 35, "s - zapisz", charset);
	DrawString(screen, SCREEN_WIDTH - 115, SCREEN_HEIGHT - 25, "l - wczytaj", charset);
}
void nowa_gra(SDL SDL, gra& gra, czas_gry& czas_gry)
{
	if (gra.czas_nowej_gry <= 500 && gra.czas_nowej_gry > 0)
	{
		gra.pozycja_startowa_x = SCREEN_WIDTH / 2;
		gra.pozycja_startowa_y = SCREEN_HEIGHT / 1.40;
		DrawString(SDL.screen, gra.pozycja_startowa_x - 200, gra.pozycja_startowa_y - 100, "Rozpoczeto nowa gre!!!", SDL.charset);
		gra.czas_nowej_gry--;
		gra.czy_przeciwnik_w_dol = 0;
		gra.czas_pojawienia_wroga = 1500;
		gra.pozycja_przeciwnika_y = -100;
		gra.zycia_wroga = 150;
		gra.wynik = 0;
		czas_gry.worldTime = 0;
		gra.czas_pojawienia_cywila = 1500;
		gra.pozycja_cywila_x = gra.pozycja_startowa_x;
		gra.pozycja_cywila_y = -150;
	}
}
void naliczanie_pkt(gra& gra)
{
	if (gra.pozycja_startowa_x > SCREEN_WIDTH / 4 + 10 && gra.pozycja_startowa_x < 3 * SCREEN_WIDTH / 4 - 10 && gra.czy_uderzenie_cywila == 0)
	{
		gra.wynik += 0.1;
	}
}

void save(gra gra, czas_gry czas_gry)
{
	FILE* plik = NULL;
	time_t t = time(NULL);//time_t typ zdolny do reprezentowania czasu zawiera liczbe sekund od okreslonej daty, time(NULL) przechowuje bierzacy czas
	struct tm tm = *localtime(&t);//deklaracja funkcji localtime
	char nazwa_pliku[60];
	if (_mkdir("save") == 0)
	{
		printf("Stworzono katalog save");
	}
	sprintf(nazwa_pliku, "save/Data %d-%02d-%02d Godzina %02d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf(nazwa_pliku);
	do
	{
		fopen_s(&plik, nazwa_pliku, "w");
	} while (plik == 0);
	fprintf(plik, "%lf\n", gra.pozycja_startowa_x);
	fprintf(plik, "%lf\n", gra.pozycja_startowa_y);
	fprintf(plik, "%f\n", gra.wynik);
	fprintf(plik, "%f\n", czas_gry.worldTime);
	fprintf(plik, "%lf\n", gra.pozycja_cywila_x);
	fprintf(plik, "%lf\n", gra.pozycja_cywila_y);
	fprintf(plik, "%lf\n", gra.pozycja_przeciwnika_x);
	fprintf(plik, "%lf\n", gra.pozycja_przeciwnika_y);
	fprintf(plik, "%d\n", gra.czas_pojawienia_cywila);
	fprintf(plik, "%d\n", gra.czas_pojawienia_wroga);
	fprintf(plik, "%d\n", gra.czy_przeciwnik_w_dol);
	fprintf(plik, "%d\n", gra.zycia_wroga);
	fclose(plik);
}
void wczytaj(gra& gra) {

	WIN32_FIND_DATA ffd;//Struktura WIN32_FIND_DATA opisuje plik znaleziony przez funkcjê FindFirstFile , FindFirstFileEx lub FindNextFile
	TCHAR szDir[MAX_PATH];//TCHAR do opisania ciagu znakow, MAX_PATH to maksymalna dlugosc sciezki(260)
	HANDLE hFind = INVALID_HANDLE_VALUE;//zmienna pod chwytanie pliku
	char path[] = "save\\*";

	int i = 0;
	do {
		szDir[i] = path[i];
		i++;
	} while (path[i]);
	szDir[i] = path[i];

	hFind = FindFirstFile(szDir, &ffd);
	char text[MAX_PATH];
	int j = 0;

	do {
		wcstombs(text, ffd.cFileName, wcslen(ffd.cFileName) + 1);//konwertuje do tablicy txt nazwe pliku, wcslen zwraca dlugosc nazwy w bitach
		if (j > 1)printf("%s\n", text);
		j++;
	} while (FindNextFile(hFind, &ffd) != 0);
	gra.liczba_zapisow = j - 4;
	printf("%s\n", "///////////////////////////////////////////");


}
void wczytaj2(SDL SDL, gra& gra, kolory kolory, czas_gry& czas_gry) {

	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char path[] = "save\\*";

	int i = 0;
	do {
		szDir[i] = path[i];
		i++;
	} while (path[i]);
	szDir[i] = path[i];

	hFind = FindFirstFile(szDir, &ffd);
	char* text = NULL;
	char textpom[MAX_PATH];
	char** zapisy;
	do
	{
		zapisy = (char**)malloc(3 * sizeof(char*));

	} while (zapisy == NULL);
	int z = 0;
	int j = 0, pozycja_napisu_x = SCREEN_WIDTH / 2 - 145, pozycja_napisu_y = SCREEN_HEIGHT / 2 + 15;
	DrawRectangle(SDL.screen, SCREEN_WIDTH / 2 - 155, SCREEN_HEIGHT / 2, 310, 60, kolory.kolor_obramowania, kolory.kolor_wypelnienia);
	DrawString(SDL.screen, pozycja_napisu_x + 120, pozycja_napisu_y, "ZAPISY", SDL.charset);
	do {
		wcstombs(textpom, ffd.cFileName, wcslen(ffd.cFileName) + 1);

		if (j > gra.liczba_zapisow)
		{

			text = (char*)malloc(MAX_PATH * sizeof(char));
			wcstombs(text, ffd.cFileName, wcslen(ffd.cFileName) + 1);
			DrawString(SDL.screen, pozycja_napisu_x, pozycja_napisu_y += 10, text, SDL.charset);
			zapisy[z] = text;
			++z;
		}
		j++;
	} while (FindNextFile(hFind, &ffd) != 0);
	SDL_Event event;
	int pom = 0;
	SDL_UpdateTexture(SDL.scrtex, NULL, SDL.screen->pixels, SDL.screen->pitch);
	SDL_RenderCopy(SDL.renderer, SDL.scrtex, NULL, NULL);
	SDL_RenderPresent(SDL.renderer);
	while (pom == 0)
	{
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_1 && gra.liczba_zapisow > 0)
				{
					printf("%s\n", zapisy[0]);
					czytaj_z_pliku(zapisy[0], gra, czas_gry);
					pom = 1;
				}
				else if (event.key.keysym.sym == SDLK_2 && gra.liczba_zapisow >= 0)
				{
					pom = 1;
					printf("%s\n", zapisy[1]);
					czytaj_z_pliku(zapisy[1], gra, czas_gry);

				}
				else if (event.key.keysym.sym == SDLK_3 && gra.liczba_zapisow >= -1)
				{
					pom = 1;
					printf("%s\n", zapisy[2]);
					czytaj_z_pliku(zapisy[2], gra, czas_gry);

				}
				else if (event.key.keysym.sym == SDLK_l)
				{
					pom = 1;

				}
				break;
			}
		}
	}
	for (int c = 0; c < z; ++c)
	{
		free(zapisy[c]);
	}
	free(zapisy);



}
void czytaj_z_pliku(char* nazwa_pliku, gra& gra, czas_gry& czas_gry)
{
	FILE* plik = NULL;
	char sciezka[100];
	sprintf_s(sciezka, "save\\%s", nazwa_pliku);
	do
	{
		fopen_s(&plik, sciezka, "r");
	} while (plik == NULL);
	fscanf_s(plik, "%lf", &gra.pozycja_startowa_x);
	fscanf_s(plik, "%lf", &gra.pozycja_startowa_y);
	fscanf_s(plik, "%lf", &gra.wynik);
	fscanf_s(plik, "%lf", &czas_gry.worldTime);
	fscanf_s(plik, "%lf", &gra.pozycja_cywila_x);
	fscanf_s(plik, "%lf", &gra.pozycja_cywila_y);
	fscanf_s(plik, "%lf", &gra.pozycja_przeciwnika_x);
	fscanf_s(plik, "%lf", &gra.pozycja_przeciwnika_y);
	fscanf_s(plik, "%d", &gra.czas_pojawienia_cywila);
	fscanf_s(plik, "%d", &gra.czas_pojawienia_wroga);
	fscanf_s(plik, "%d", &gra.czy_przeciwnik_w_dol);
	fscanf_s(plik, "%d", &gra.zycia_wroga);
}
void czas_i_fps(czas_gry& czas_gry)
{
	double delta = (czas_gry.t2 - czas_gry.t1) * 0.001;
	czas_gry.t1 = czas_gry.t2;

	czas_gry.worldTime += delta;

	czas_gry.fpsTimer += delta;
	if (czas_gry.fpsTimer > 0.5) {
		czas_gry.fps = czas_gry.frames * 2;
		czas_gry.frames = 0;
		czas_gry.fpsTimer -= 0.5;
	}
}
void warstwa_wizualna(SDL SDL, gra gra, char* text, czas_gry czas, kolory kolory)
{
	DrawRectangle(SDL.screen, 60, 4, SCREEN_WIDTH - 120, 36, kolory.kolor_obramowania, kolory.kolor_wypelnienia);
	sprintf(text, " czas trwania = %.1lf s  %.0lf klatek / s", czas.worldTime, czas.fps);
	DrawString(SDL.screen, (int)(SDL.screen->w / 2 - strlen(text) * 8 / 2), 10, text, SDL.charset);
	sprintf(text, " wynik = %.1lf pkt", gra.wynik);
	DrawString(SDL.screen, (int)(SDL.screen->w / 2 - strlen(text) * 8 / 2), 25, text, SDL.charset);
	DrawRectangle(SDL.screen, SCREEN_WIDTH - 120, SCREEN_HEIGHT - 100, 110, 90, kolory.kolor_obramowania, kolory.kolor_wypelnienia);
	wypisz_legende(SDL.screen, text, SDL.charset);
}
void obsluga_klawiszy(SDL SDL, ruchy& ruchy, gra& gra, czas_gry& czas_gry, kolory kolory, pocisk& pocisk)
{
	switch (SDL.event.type) {
	case SDL_KEYDOWN:
		if (SDL.event.key.keysym.sym == SDLK_ESCAPE) czas_gry.quit = 1;
		else if (SDL.event.key.keysym.sym == SDLK_UP && gra.pauza == 0)
		{

			przemieszczenie_w_gore(gra.pozycja_startowa_y);
			if (gra.pozycja_startowa_y < SCREEN_HEIGHT / 3)gra.pozycja_startowa_y = SCREEN_HEIGHT / 3;
			gra.speed = 2.0;
			ruchy.gora = 1;
		}
		else if (SDL.event.key.keysym.sym == SDLK_DOWN && gra.pauza == 0)
		{

			przemieszczenie_w_dol(gra.pozycja_startowa_y);
			if (gra.pozycja_startowa_y > SCREEN_HEIGHT - 45)gra.pozycja_startowa_y = SCREEN_HEIGHT - 45;
			gra.speed = 3.0;
			ruchy.dol = 1;
		}
		else if (SDL.event.key.keysym.sym == SDLK_LEFT && gra.pauza == 0)
		{
			przemieszczenie_w_lewo(gra.pozycja_startowa_x);
			ruchy.lewo = 1;
		}
		else if (SDL.event.key.keysym.sym == SDLK_RIGHT && gra.pauza == 0)
		{
			przemieszczenie_w_prawo(gra.pozycja_startowa_x);
			ruchy.prawo = 1;
		}
		else if (SDL.event.key.keysym.sym == SDLK_n && gra.pauza == 0)
		{
			gra.czas_nowej_gry = 500;//pozwala uruchomic funkcje od nowej gry

		}
		else if (SDL.event.key.keysym.sym == SDLK_p)
		{
			if (gra.pauza == 0)gra.pauza = 1;
			else gra.pauza = 0;
		}
		else if (SDL.event.key.keysym.sym == SDLK_s)
		{
			save(gra, czas_gry);
		}
		else if (SDL.event.key.keysym.sym == SDLK_l)
		{
			wczytaj(gra);
			if (gra.pauza == 0)gra.pauza = 2;
		}
		else if (SDL.event.key.keysym.sym == SDLK_SPACE)
		{
			pocisk.czy_wystrzelony_pocisk = 1;
		}
		break;
	case SDL_KEYUP:
		jaki_klawisz_zwolniony(ruchy);
		gra.speed = 1.0;
		break;
	case SDL_QUIT:
		czas_gry.quit = 1;
		break;

	};
}
void jaki_klawisz_zwolniony(ruchy& ruchy)
{
	if (!state[SDL_SCANCODE_UP])ruchy.gora = 0;
	if (!state[SDL_SCANCODE_DOWN])ruchy.dol = 0;
	if (!state[SDL_SCANCODE_LEFT])ruchy.lewo = 0;
	if (!state[SDL_SCANCODE_RIGHT])ruchy.prawo = 0;

}
void plynny_ruch(ruchy& ruchy, gra& gra)
{
	if (ruchy.gora == 1)
	{
		przemieszczenie_w_gore(gra.pozycja_startowa_y);
		if (gra.pozycja_startowa_y < SCREEN_HEIGHT / 3)gra.pozycja_startowa_y = SCREEN_HEIGHT / 3;
	}
	else if (ruchy.dol == 1)
	{
		przemieszczenie_w_dol(gra.pozycja_startowa_y);
		if (gra.pozycja_startowa_y > SCREEN_HEIGHT - 45)gra.pozycja_startowa_y = SCREEN_HEIGHT - 45;
	}
	else if (ruchy.prawo == 1)
	{
		przemieszczenie_w_prawo(gra.pozycja_startowa_x);
		ruchy.prawo = 1;
	}
	else if (ruchy.lewo == 1)
	{
		przemieszczenie_w_lewo(gra.pozycja_startowa_x);
	}
}
void jezeli_zdjecie_charset_null(SDL SDL)
{
	SDL_FreeSurface(SDL.charset);
	SDL_FreeSurface(SDL.screen);
	SDL_DestroyTexture(SDL.scrtex);
	SDL_DestroyWindow(SDL.window);
	SDL_DestroyRenderer(SDL.renderer);
	SDL_Quit();
}
void ruch_przeciwnika(SDL SDL, gra& gra)
{
	SDL.przeciwnik = SDL_LoadBMP("./wrog.bmp");
	if (gra.czas_pojawienia_wroga <= 1500 && gra.czas_pojawienia_wroga > 0)
	{
		--gra.czas_pojawienia_wroga;
	}
	else
	{

		if (gra.pozycja_startowa_y - 40 < gra.pozycja_przeciwnika_y && gra.pozycja_startowa_y + 40 > gra.pozycja_przeciwnika_y && gra.czy_przeciwnik_w_dol == 0)
		{
			if (gra.pozycja_startowa_x > gra.pozycja_przeciwnika_x)gra.pozycja_przeciwnika_x += 0.25;
			else if (gra.pozycja_startowa_x < gra.pozycja_przeciwnika_x)gra.pozycja_przeciwnika_x -= 0.25;

			if (gra.pozycja_startowa_y + 35 < gra.pozycja_przeciwnika_y)gra.czy_przeciwnik_w_dol = 1;
		}

		if (gra.czy_przeciwnik_w_dol == 1)
		{
			gra.pozycja_przeciwnika_y += 0.5;
		}
		else
		{
			if (gra.pozycja_startowa_x - 55 > gra.pozycja_przeciwnika_x)gra.pozycja_przeciwnika_x += 0.25;
			else if (gra.pozycja_startowa_x + 55 < gra.pozycja_przeciwnika_x)gra.pozycja_przeciwnika_x -= 0.25;

			if (gra.pozycja_startowa_y > gra.pozycja_przeciwnika_y)gra.pozycja_przeciwnika_y += 0.25;

			if (gra.pozycja_startowa_y - gra.pozycja_przeciwnika_y < 80 && fabs(gra.pozycja_startowa_x - gra.pozycja_przeciwnika_x) < 40)
			{
				gra.pozycja_przeciwnika_y = -100;
				gra.czy_przeciwnik_w_dol = 0;
				gra.czas_wybuchu = 300;//pozwala urochomic funkcje czy wypadek i wyswietlic grafike
				gra.wynik -= 1000;
				gra.czas_pojawienia_wroga = 1500;
				gra.zycia_wroga = 150;
			}
		}

		if (gra.pozycja_przeciwnika_y > SCREEN_HEIGHT + 150)
		{
			gra.czy_przeciwnik_w_dol = 0;
			gra.czas_pojawienia_wroga = 1500;
			gra.pozycja_przeciwnika_y = -100;
			gra.zycia_wroga = 150;
		}
	}
	DrawSurface(SDL.screen, SDL.przeciwnik, gra.pozycja_przeciwnika_x, gra.pozycja_przeciwnika_y);

}
void ruch_cywila(SDL SDL, gra& gra)
{
	SDL.cywil = SDL_LoadBMP("./cywil.bmp");
	if (gra.czas_pojawienia_cywila <= 1500 && gra.czas_pojawienia_cywila > 0)
	{
		--gra.czas_pojawienia_cywila;
	}
	else if (fabs(gra.pozycja_cywila_x - gra.pozycja_przeciwnika_x) < 80 && fabs(gra.pozycja_cywila_y - gra.pozycja_przeciwnika_y) < 100)
	{
		gra.pozycja_cywila_y--;
		DrawSurface(SDL.screen, SDL.cywil, gra.pozycja_cywila_x, gra.pozycja_cywila_y);

	}
	else
	{
		gra.czy_uderzenie_cywila = 0;
		gra.pozycja_cywila_y += 0.25;

		if (gra.pozycja_przeciwnika_x - gra.pozycja_cywila_x < 100 && gra.pozycja_cywila_x > SCREEN_WIDTH / 4 + 25)
		{
			gra.pozycja_cywila_x -= 0.25;
		}
		else if (gra.pozycja_cywila_x - gra.pozycja_przeciwnika_x < 100 && gra.pozycja_cywila_x < SCREEN_WIDTH / 1.25 - 50)
		{
			gra.pozycja_cywila_x += 0.25;
		}

		if (gra.pozycja_startowa_x > gra.pozycja_cywila_x && gra.pozycja_cywila_x > SCREEN_WIDTH / 4 + 25)gra.pozycja_cywila_x -= 0.25;
		else if (gra.pozycja_startowa_x < gra.pozycja_cywila_x && gra.pozycja_cywila_x < SCREEN_WIDTH / 1.25 - 50) gra.pozycja_cywila_x += 0.25;

		if (fabs(gra.pozycja_startowa_y - gra.pozycja_cywila_y) < 80 && fabs(gra.pozycja_startowa_x - gra.pozycja_cywila_x) < 40)
		{
			gra.czas_pojawienia_cywila = 1500;
			gra.pozycja_cywila_x = gra.pozycja_startowa_x;
			gra.pozycja_cywila_y = -150;
			gra.czas_wybuchu = 300;//pozwala urochomic funkcje czy wypadek i wyswietlic grafike
			gra.czy_uderzenie_cywila = 1;
		}

		DrawSurface(SDL.screen, SDL.cywil, gra.pozycja_cywila_x, gra.pozycja_cywila_y);
	}
	if (gra.pozycja_cywila_y > SCREEN_HEIGHT + 150 || gra.pozycja_cywila_y < -200)
	{
		gra.czas_pojawienia_cywila = 1500;
		gra.pozycja_cywila_x = SCREEN_WIDTH / 2;
		gra.pozycja_cywila_y = -150;
	}
	
}
void strzal(SDL SDL, gra& gra, kolory kolory, pocisk& pocisk)
{

	if (pocisk.czy_wystrzelony_pocisk == 1)
	{
		pocisk.pozycja_startowa_pocisku_x = gra.pozycja_startowa_x;
		pocisk.pozycja_startowa_pocisku_y = gra.pozycja_startowa_y;
		pocisk.pozycja_pocisku_x = gra.pozycja_startowa_x;
		pocisk.pozycja_pocisku_y = gra.pozycja_startowa_y;
		pocisk.czy_wystrzelony_pocisk = 2;
	}
	if (pocisk.czy_wystrzelony_pocisk == 2 && pocisk.pozycja_startowa_pocisku_y - pocisk.pozycja_pocisku_y < 200 && pocisk.pozycja_pocisku_y>0)
	{
		pocisk.pozycja_pocisku_y -= 2;
		strzal_do_wroga(pocisk, gra, SDL, kolory);
		strzal_do_cywila(pocisk, gra, SDL, kolory);
		DrawLine(SDL.screen, (int)(pocisk.pozycja_pocisku_x), (int)(pocisk.pozycja_pocisku_y), 20, 0, 1, kolory.czerwony);
	}
	else pocisk.czy_wystrzelony_pocisk = 0;
}
void strzal_do_wroga(pocisk& pocisk, gra& gra, SDL SDL, kolory kolory)
{
	if (pocisk.pozycja_pocisku_y - gra.pozycja_przeciwnika_y < 50 && gra.zycia_wroga == 0)
	{
		pocisk.pozycja_wybuchu_x = gra.pozycja_przeciwnika_x;
		pocisk.pozycja_wybuchu_y = gra.pozycja_przeciwnika_y;
		gra.pozycja_przeciwnika_y = -100;
		gra.czy_przeciwnik_w_dol = 0;
		pocisk.czas_ekspozji = 300;//pozwala urochomic funkcje i wyswietlic grafike
		gra.wynik += 1000;
		gra.czy_przeciwnik_w_dol = 0;
		gra.czas_pojawienia_wroga = 1500;
		gra.zycia_wroga = 150;

	}
	else if (pocisk.pozycja_pocisku_y - gra.pozycja_przeciwnika_y < 50 && pocisk.pozycja_pocisku_x <= gra.pozycja_przeciwnika_x + 25 && pocisk.pozycja_pocisku_x >= gra.pozycja_przeciwnika_x - 25)
	{
		gra.zycia_wroga -= 50;
		pocisk.czy_wystrzelony_pocisk = 0;
	}
}
void strzal_do_cywila(pocisk& pocisk, gra& gra, SDL SDL, kolory kolory)
{
	if (pocisk.pozycja_pocisku_y - gra.pozycja_cywila_y < 50 && pocisk.pozycja_pocisku_x <= gra.pozycja_cywila_x + 25 && pocisk.pozycja_pocisku_x >= gra.pozycja_cywila_x - 25)
	{
		pocisk.pozycja_wybuchu_x = gra.pozycja_cywila_x;
		pocisk.pozycja_wybuchu_y = gra.pozycja_cywila_y;
		gra.czas_pojawienia_cywila = 1500;
		gra.pozycja_cywila_x = gra.pozycja_startowa_x;
		gra.pozycja_cywila_y = -150;
		pocisk.czas_ekspozji = 300;
		gra.czy_uderzenie_cywila = 1;

	}
}
void wybuch_pojazdow(SDL SDL, pocisk& pocisk, gra gra)
{
	if (pocisk.czas_ekspozji <= 300 && pocisk.czas_ekspozji > 0)
	{
		DrawSurface(SDL.screen, SDL.wybuch, pocisk.pozycja_wybuchu_x, pocisk.pozycja_wybuchu_y);
		pocisk.czas_ekspozji--;
	}
}



