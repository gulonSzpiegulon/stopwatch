#include <stdlib.h>	//dla funkcji exit();
#include <time.h>	
#include <ncurses.h>	//w przypadku zmiany systemu na windows nalezy zakomentowac 	
//#include <curses.h>	//a to odkomentowac
#include <stdarg.h>
#include <unistd.h>	//w przypadku zmiany systemu na windows nalezy zakomentowac
//#include <windows.h>	//a to odkomentowac
			//nalezy rowniez odkomentowac i zakomentowac funkcje sleep badz Sleep w funkcji wyswietlEkranPozegnalny(), sleep - linuxowa, Sleep - windowsowa

enum StanyStopera { WLASNIE_ROZPOCZYNA_ODLICZANIE, ODLICZA, ZATRZYMANY, ZRESETOWANY };
enum Klawisze { START = 1, STOP, RESET, ZAKONCZ_PRACE };

typedef struct {
    unsigned int setne, sekundy, minuty, godziny, stan;
} Stoper;

void inicjalizujNewCurses();							//funkcja wykonuje czynnosci inicjalizujace zwiazane z bibiloteka ncurses lub pdcurses
void zerujStoper(Stoper *stoper);						//funkcja wypelnia wszystkie pola struktury stroper, do ktorej wskaznik otrzymuje w parametrze, zerami za wyjatkiem pola
										//stan ktore ustalane jest na wartosc ZRESETOWANY
void wyswietlStoper();								//funkcja wyswietla interfejs programu
void sterujStoperem(Stoper *stoper);						//funkcja sprawdza czy zostal wcisniety klawisz 1, 2, 3 lub 4 i w zaleznosci od tego, ktory to byl zmienia stan stopera badz
										//zamyka program
int czyWcisnietoKlawisz();							//funkcja zwraca wartosc 1 jesli jakikolwiek klawisz zostal wcisniety oraz 0 jesli nie
void wyswietlEkranPozegnalny();							//funkcja wyswietla pozegnalny ekran programu
void zakonczPraceNewCurses();							//funkcja konczy prace biblioteki ncurses badz pdcurses
void obliczCzasIWyswietl(Stoper *stoper);					//funkcja na podstawie czasu pobranego z zegara systemowego przestawia stoper i wyswietla jego wskazania
void ustawRozniceCzasowNaSetnaSekundy(struct timespec *roznicaCzasow);		//funkcja ustawia strukture, do ktorej wskaznik otrzymuje w parametrze, na setna sekundy
void pobierzCzasSystemowy(unsigned int dlaIluZegarow, ...);			//funkcja wypelnia strukture badz struktury czasu timespec czasem systemowym, ilosc struktur wypelnianych zalezy od 
										//pierwszego parametru, kolejne parametry maja byc wskaznikami na te struktury

int czyRoznicaCzasowJestWiekszaNizBadzRownaZ(struct timespec *odjemna, struct timespec *odjemnik, struct timespec *czasZKtorymPorownujemy);	//funkcja sprawdza czy roznica czasu miedzy struktura 
																		//odjemna a odjemnik jest wieksza badz rowna ilosci czasu 
																		//zapisanej w strukturze czasZKtorymPorownujemy, 
																		//jesli tak jest zwraca 1, w przeciwnym razie 0
void obliczRozniceCzasow(struct timespec *roznica, const struct timespec *odjemna, const struct timespec *odjemnik);	//funkcja odejmuje czas podany w odjemniku od czasu podanego w odjemnej i wynik
															//zapisuje w roznicy
int czyCzasJestWiekszyNizBadzRownyZ(struct timespec *czas, struct timespec *czasZKtorymPorownujemy);	//funkcja sprawdza czy struktura czas ma w sobie wartosc wieksza badz rowna wartosci struktury
													//czasZKtorymPorownujemy, jesli tak to zwraca wartosc 1, jesli nie to zwraca 0 
void przestawCzasDoPrzoduO(struct timespec *czas, const struct timespec *oIlePrzestawic);		//funkcja przestawia do przodu czas struktury czas o ilosc czasu struktury oIlePrzestawic 

void aktualizujWskazaniaStopera(Stoper *stoper);				//funkcja zwieksza wskazanie stopera o 1 setna przy kazdym wywolaniu 
void wyswietlBiezacyCzas(Stoper *stoper);					//funkcja wyswietla biezace wskazanie stopera
void formatujIWyswietlSkladowaCzasu(const unsigned int skladowa);		//funkcja wyswietla skladowa biezacego wskazania stopera w odpowiednio sformatowany sposob

int kursorKonsoliPozycjaY = 0, kursorKonsoliPozycjaX = 0;			//zmienne globalne do przechowywania aktualnej wartosci pozycji kursora, ktory wskazuje miejsce na ekranie, w ktorym program 											//wypisuje tekst

int main()
{
    inicjalizujNewCurses();			//nastepuje inicjalizacja bibiloteki new curses
    Stoper stoper;				//tworzona jest struktura stoper
    zerujStoper(&stoper);			//jej pola: setne, sekundy, minuty oraz godziny sa zerowane, a stan ustawiany na ZRESETOWANY 
    wyswietlStoper();				//nastepuje wyswietlenie interfejsu programu
    while (1) {					//w nieskonczonej petli
        sterujStoperem(&stoper);		//sprawdzane jest czy zostal wcisniety przycisk, jesli tak i byl to jeden z: 1, 2, 3 lub 4, ustawiany jest odpowiedni do klawisza 
						//stan stopera
        obliczCzasIWyswietl(&stoper);		//w zaleznosci do stanu stopera nastepuja odpowiednie obliczenia czasu badz akcje oraz za kazdym razem wyswietlany jest biezacy czas 
    }
}

void inicjalizujNewCurses() {						//funkcja wykonuje czynnosci inicjalizacyjne zwiazane z bibiloteka ncurses lub pdcurses
    initscr();								//funkcja rozpoczyna prace z biblioteka ncurses / pdcurses									
    raw();								//funkcja sprawia ze znaki wprowadzane nie beda buforowane az do wprowadzenia znaku nowej lini oraz karetki powrotu 
    noecho();								//funkcja sprawia ze znaki po wpisaniu nie beda wyswietlane automatycznie
    nodelay(stdscr, TRUE);						//funkcja sprawia ze getch() nie bedzie blokowal programu czekajac na wcisniecie klawisza (w oknie stdscr)
    getmaxyx(stdscr, kursorKonsoliPozycjaY, kursorKonsoliPozycjaX);	//funkcja pobiera wysokosc oraz szerokosc okna stdscr i zapisuje te wymiary odpowiednio do kursorKonsoliPozycjaY oraz 
									//kursorKonsoliPozycjaX
    kursorKonsoliPozycjaY /= 2;						//pozycja y kursora ustawiana jest na srodek ekranu
    kursorKonsoliPozycjaX /= 2;						//pozycja x kursora ustawiana jest na srodek ekranu
}

void zerujStoper(Stoper *stoper) {		//funkcja wypelnia wszystkie pola struktury stroper, do ktorej wskaznik otrzymuje w parametrze, zerami za wyjatkiem pola
						//stan ktore ustalane jest na wartosc ZRESETOWANY
    stoper->setne = 0;
    stoper->sekundy = 0;
    stoper->minuty = 0;
    stoper->godziny = 0;
    stoper->stan = ZRESETOWANY;
}

void wyswietlStoper() {								//funkcja wyswietla interfejs programu
    const char TYTUL[] =            "|----------STOPER---------|";		//najpierw tworzony jest on w tablicach znakow
    const char KLIKNIJ[] =          "|  Wybierz:               |";	
    const char PIERWSZA_OPCJA[] =   "|  1 - Start              |";
    const char DRUGA_OPCJA[] =      "|  2 - Stop               |";
    const char TRZECIA_OPCJA[] =    "|  3 - Reset              |";
    const char CZWARTA_OPCJA[] =    "|  4 - Exit               |";
    const char WYSWIETLACZ_GORA[] = "|-------------------------|";
    const char WYSWIETLACZ_BOKI[] = "|                         |";
    const char WYSWIETLACZ_SPOD[] = "|-------------------------|";

    kursorKonsoliPozycjaY -= 6;							//nastepnie pozycja y kursora przesuwana jest o 6 wierszy do gory
    kursorKonsoliPozycjaX -= sizeof(TYTUL) / 2;					//a pozycja x kursora przesuwana jest w lewo o polowe liczby znakow jednej z powyzszych tablic znakowych	

    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, TYTUL);		//tablica znakow TYTUL wyswietlana jest na wirtualnym ekranie poczawszy od biezacej pozycji kursora
    kursorKonsoliPozycjaY++;							//pozycja y kursora przesuwana jest o 1 wiersz nizej
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, KLIKNIJ);		//tablica znakow KLIKNIJ wyswietlana jest poczawszy od biezacej pozycji kursora	
    kursorKonsoliPozycjaY++;							//pozycja y kursora przesuwana jest na wirtualnym ekranie o 1 wiersz nizej
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, PIERWSZA_OPCJA);	//itd.
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, DRUGA_OPCJA);
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, TRZECIA_OPCJA);
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, CZWARTA_OPCJA);
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, WYSWIETLACZ_GORA);
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, WYSWIETLACZ_BOKI);
    kursorKonsoliPozycjaY++;
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, WYSWIETLACZ_SPOD);	//po wyswietleniu ostatniej tablicy znakowej interfejsu na wirtualnym ekranie pozycja nie jest przesuwana juz o 1 wiersz 
										//nizej
    refresh();									//funkcja wyswietla wszystko na rzeczywistym ekranie

    kursorKonsoliPozycjaY--;							//pozycja y kursora przesuwana jest o 1 wiersz do gory (aby wskazywac na wnetrze wyswietlacza dla pozniejszego wypisywania 
										//wskazan stopera)
    kursorKonsoliPozycjaX += 5;							//pozycja x kursora przesuwana jest o 5 kolumn w prawo (aby wskazywac na wnetrze wyswietlacza dla pozniejszego wypisywania 
										//wskazan stopera)	
}

void sterujStoperem(Stoper *stoper) {						//funkcja sprawdza czy zostal wcisniety klawisz 1, 2, 3 lub 4 i w zaleznosci od tego, ktory to byl zmienia stan stopera badz
										//zamyka program				
    if (czyWcisnietoKlawisz() == 1) {						//jesli zostal wcisniety jakis klawisz
        switch(getch() - 48) {							//jesli klawisz ten to:
        case START:								//1
            if (stoper->stan == ZATRZYMANY || stoper->stan == ZRESETOWANY) {	//oraz jesli stoper jest zatrzymany badz zresetowany
                stoper->stan = WLASNIE_ROZPOCZYNA_ODLICZANIE;			//to jego stan ustawiany jest na WLASNIE_ROZPOCZAL_ODLICZANIE
            }
            break;
        case STOP:								//2
            stoper->stan = ZATRZYMANY;						//to stan stopera ustawiany jest na ZATRZYMANY
            break;
        case RESET:								//3
            stoper->stan = ZRESETOWANY;						//to stan stopera ustawiany jest na ZRESETOWANY
            break;
        case ZAKONCZ_PRACE:							//4
            wyswietlEkranPozegnalny();						//to wyswietlany jest ekran pozegnalny
            zakonczPraceNewCurses();						//oraz konczona jest praca biblioteki ncurses / pdcurses
            exit(0);								//i nastepuje wyjscie z programu
        };
    }
}

int czyWcisnietoKlawisz() {		//funkcja zwraca wartosc 1 jesli jakikolwiek klawisz zostal wcisniety oraz 0 jesli nie		
	int znak = getch(); 		//pobierany jest kod znaku z wejscia do zmiennej znak	
    	if (znak != ERR) {		//jesli na wejsciu cos bylo
        	ungetch(znak);		//znak jest spowrotem odsylany na wejscie
       		return 1;		//zwracana jest wartosc 1
    	}
	else {				//jesli na wejsciu niczego nie bylo (zaden klawisz nie zostal wcisniety)
        	return 0;		//zwracana jest wartosc 0
    	}
}

void wyswietlEkranPozegnalny() {						//funkcja wyswietla pozegnalny ekran programu
    const char TEKST_POZEGNALNY[] = "bye (:";   				//tworzona jest tablica znakow przechowujaca tekst pozegnalny
    clear();									//ekran jest czyszczony
    getmaxyx(stdscr, kursorKonsoliPozycjaY, kursorKonsoliPozycjaX);		//funkcja ustawia pozycje y oraz x kursora na prawy dolny rog ekranu
    kursorKonsoliPozycjaY /= 2;							//pozycja y kursora ustawiana jest na srodek ekranu
    kursorKonsoliPozycjaX /= 2;							//pozycja x kursora ustawiana jest na srodek ekranu
    kursorKonsoliPozycjaX -= sizeof(TEKST_POZEGNALNY) / 2;			//pozycja x kursora przesuwana jest w lewo o polowe znakow z tablicy znakow TEKST_POZEGNALNY
    mvprintw(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX, TEKST_POZEGNALNY);	//na wirtualnym ekranie na pozycji okreslonej powyzej wypisywana jest tablica znakow 
    refresh();									//nastepuje wyswietlenie wirtualnego ekranu na rezczywistym ekranie
    sleep(1);									//usypia program na sekunde
    //Sleep(1000);								//w przypadku zmiany systemu na windows nalezy odkomentowac Sleep(1) i zakomentowac sleep(1);
}

void zakonczPraceNewCurses() {		//funkcja konczy prace biblioteki ncurses badz pdcurses
    endwin();				
}

void obliczCzasIWyswietl(Stoper *stoper) {				//funkcja na podstawie czasu pobranego z zegara systemowego przestawia stoper i wyswietla jego wskazania
    static struct timespec poczatkowoPobranyCzasSystemowy;		//tworzona jest struktura timespec poczatkowoPobranyCzasSystemowy - bedzie wypelniana czasem z zegara systemowego po uruchomieniu 
									//stopera oraz po kazdym jego wznowieniu oraz modyfikowana o setna sekundy za kazdym razem gdy stoper bedzie mial zmienic swoje 
									//wskazanie
    static struct timespec aktualniePobieranyCzasSystemowy;		//tworzona jest struktura timespec aktualniePobieranyCzasSystemowy - bedzie wypelniania czasem z zegara systemowego za kazdym razem 
									//gdy ta funkcja bedzie wywolywana - za wyjatkiem momentow gdy stoper jest zatrzymany 
    static struct timespec roznicaCzasow;				//tworzona jest struktura timespec roznicaCzasow - ktora w gruncie rzeczy bedzie setna sekundy 
    ustawRozniceCzasowNaSetnaSekundy(&roznicaCzasow);			//funkcja ustawia rozniceCzasow na setna seknundy	
    switch(stoper->stan) {						
    case WLASNIE_ROZPOCZYNA_ODLICZANIE:					//jesli stoper wlasnie rozpoczyna odliczanie - tzn zostal uruchomiony lub wznowiony po zatrzymaniu lub resecie
        pobierzCzasSystemowy(2, &poczatkowoPobranyCzasSystemowy, &aktualniePobieranyCzasSystemowy);	//funkcja pobiera czas systemowy i wypelnia nim poczatkowoPobranyCzasSystemowy oraz 
													//aktualniePobieranyCzasSystemowy
        stoper->stan = ODLICZA;						//stan stopera przestawiany jest na ODLICZA
        break;	
    case ODLICZA:							//jesli stoper odlicza
        pobierzCzasSystemowy(1, &aktualniePobieranyCzasSystemowy);	//funkcja pobiera czas systemowy i wypelnia nim aktualniePobieranyCzasSystemowy
        if (czyRoznicaCzasowJestWiekszaNizBadzRownaZ(&aktualniePobieranyCzasSystemowy, 
		&poczatkowoPobranyCzasSystemowy, &roznicaCzasow) == 1) {		//funkcja sprawdza czy roznica czasow miedzy przed chwila pobranym czasem aktualniePobieranyCzasSystemowy oraz 
											//pobranym albo zmodyfikowanym w poprzednim wywolaniu tej funkcji czasem poczatkowoPobranyCzasSystemowy jest wieksza 
											//lub rowna od roznicyCzasow, jesli tak to zwraca 1 jesli nie to 0
            przestawCzasDoPrzoduO(&poczatkowoPobranyCzasSystemowy, &roznicaCzasow);	//jesli powyzszy warunek byl spelniony to poczatkowoPobranyCzasSystemowy jest przestawiany zgodnie z ruchem 
											//wskazowek zegara o rozniceCzasu czyli setna sekundy
            aktualizujWskazaniaStopera(stoper);				//i wskazanie stopera rowniez zwieksza sie o setna sekundy
        }
        break;
    case ZRESETOWANY:				//jesli stoper jest resetowany
        zerujStoper(stoper);			//to jego wskazania sa zerowane
    };
    wyswietlBiezacyCzas(stoper);		//niezaleznie od stanu stopera za kazdym razem biezace wskazanie stopera jest wyswietlane na ekran
}

void ustawRozniceCzasowNaSetnaSekundy(struct timespec *roznicaCzasow) {	//funkcja ustawia strukture, do ktorej wskaznik otrzymuje w parametrze, na setna sekundy 
    roznicaCzasow->tv_sec = 0;						//liczba sekund parametru przyrownywana jest do zera
    roznicaCzasow->tv_nsec = 10000000;					//liczba milisekund parametru przyrownywana jest do 10000000 cyzli 1/100 sekundy
}

void pobierzCzasSystemowy(unsigned int dlaIluZegarow, ...) {	//funkcja wypelnia strukture badz struktury czasu timespec czasem systemowym, ilosc struktur wypelnianych zalezy od pierwszego parametru, 
								//kolejne parametry maja byc wskaznikami na te struktury
    va_list listaZegarow;					//listaZegarow bedzie przechowywac zmienna liste argumentow
    va_start(listaZegarow, dlaIluZegarow);			//nastepuje inicjalizacja listyZegarow za pomoca makra va_start, wymagane jest podanie ostatniego argumentu nie bedacego ze zmiennej listy 
								//argumentow - czyli w tym przypadku dlaIluZegarow
    unsigned int i;
    for (i = 0; i < dlaIluZegarow; i++) {			//dla kazdego z zegarow
        clock_gettime(CLOCK_MONOTONIC, va_arg(listaZegarow, struct timespec*));	//makro va_arg zwraca dany zegar tj. wskaznik do niego (w tym miejscu bylo konieczne okreslenie jego typu czyli struct 
										//timespec*), po takim zwroceniu makro samo przestawia sie na nastepny argument - tj. przy nastepnym zwracanu wartosci 
										//zwroci wartosc kolejnego argumentu z listy
										//funkcja clock_gettime pobiera czas z zegara monotonicznego i wypelnia nim wartosc zwrocona przez va_arg tj wskaznik na 
										//zegar
										//czyli zeby nie komplikowac zbytnio, przy kazdej iteracji petli clock_gettime wypelnia kolejny zegar z listy czasem 	
										//pobranym z zegara monotoniczengo
    }
    va_end(listaZegarow);					//va_start wymaga uzycia va_end przed wyjsciem z funkcji
}

int czyRoznicaCzasowJestWiekszaNizBadzRownaZ(struct timespec *odjemna, struct timespec *odjemnik, struct timespec *czasZKtorymPorownujemy) {	//funkcja sprawdza czy roznica czasu miedzy struktura 
													//odjemna a odjemnik jest wieksza badz rowna ilosci czasu zapisanej w strukturze 
													//czasZKtorymPorownujemy, jesli tak jest zwraca 1, w przeciwnym razie 0
    struct timespec roznica;								//roznica sluzy do przechowywania roznicy miedzy czasami odjemna i odjemnik
    obliczRozniceCzasow(&roznica, odjemna, odjemnik);					//funkcja odejmuje odjemnik od odjemnej i zapisuje wartosc do roznicy
    if (czyCzasJestWiekszyNizBadzRownyZ(&roznica, czasZKtorymPorownujemy) == 1) {	//roznica jest porownywana z czasemZKtorymPorownujemy
        return 1;									//jesli jest wieksza badz rowna temu czasu zwracane jest 1
    }	
    else {
        return 0;									//jesli jest mniejsza to zwracane jest 0
    }
}

void obliczRozniceCzasow(struct timespec *roznica, const struct timespec *odjemna, const struct timespec *odjemnik) {	//funkcja odejmuje czas podany w odjemniku od czasu podanego w odjemnej i wynik
															//zapisuje w roznicy
    roznica->tv_sec = odjemna->tv_sec - odjemnik->tv_sec;			//najpierw odejmowane sa sekundy odjemnika od odjemnej i wynik tego dzialania zapisywany jest do sekund roznicy
    roznica->tv_nsec = odjemna->tv_nsec - odjemnik->tv_nsec;			//nastepnie to samo dzieje sie z nanosekundami			
    if (roznica->tv_sec > 0 && roznica->tv_nsec < 0) {				//jesli liczba sekund roznicy jest wieksza od zera ale liczba jej nanosekund jest mniejsza od zera
        roznica->tv_sec--;							//nalezy zmniejszyc ilosc sekund roznicy o 1
        roznica->tv_nsec = 1000000000 + roznica->tv_nsec;			//odebrac te usunieta sekunde w nanosekundach i od niej odjac (dodac ujemna) liczbe nanosekund i wynik tego dzialania 
										//zapisac do nanosekund roznicy
    }
    else if(roznica->tv_sec < 0 && roznica->tv_nsec > 0) {			//jesli liczba sekund roznicy jest mniejsza od zera ale liczba jej nanosekund jest wieksza od zera
        roznica->tv_sec++;							//nalezy dodac jedna sekunde do puli sekund roznicy
        roznica->tv_nsec = -1000000000 + roznica->tv_nsec;			//a od liczby nanosekund odjac sekunde w nanosekundach
										//ten drugi przypadek nie powinien sie zdarzyc w programie jako ze korzystamy z monotonicznego zrodla czasu i za kazdym 										//razem odejmujemy teoretycznie czas wczesniej pobrany od czasu pozniej pobranego co zawsze musi dac pierwszy przypadek albo 
										//cos innego ale nie drugi
    }
}


int czyCzasJestWiekszyNizBadzRownyZ(struct timespec *czas, struct timespec *czasZKtorymPorownujemy) {		//funkcja sprawdza czy struktura czas ma w sobie wartosc wieksza badz rowna wartosci 	
														//struktury czasZKtorymPorownujemy, jesli tak to zwraca wartosc 1, jesli nie to zwraca 0 
    if (czas->tv_sec > czasZKtorymPorownujemy->tv_sec) {							//jesli liczba sekund czasu jest wieksza niz czasuZKtorymPorownujemy
        return 1;												//to zwroc 1
    }		
    else if (czas->tv_sec == czasZKtorymPorownujemy->tv_sec && czas->tv_nsec >= czasZKtorymPorownujemy->tv_nsec) {	//jesli liczba sekund czasu jest rowna z czasemZKtorymPorownujemy oraz liczba 
															//milisekund czasu jest wieksza badz rowna z czasemZKtorymPorownujemy	
        return 1;	//to zwroc 1
    }
    else {		//w przeciwnym wypadku 
        return 0;	//zwroc 0
    }
}

void przestawCzasDoPrzoduO(struct timespec *czas, const struct timespec *oIlePrzestawic) {	//funkcja przestawia do przodu czas struktury czas o ilosc czasu struktury oIlePrzestawic
    czas->tv_sec += oIlePrzestawic->tv_sec;							//ilosc sekund czasu jest przestawiana zgodnie ze wskazowkami zegara o ilosc sekund oIlePrzestawic
    unsigned int nanosekundy = czas->tv_nsec + oIlePrzestawic->tv_nsec;				//suma nanosekund czasu oraz oIlePrzestawic zapisywana jest do zmiennej nanosekundy
    if (nanosekundy >= 1000000000) {								//jesli zmienna nanosekundy jest wieksza lub rowna 1000000000
        czas->tv_sec++;										//to ilosc sekund czasu jest zwiekszana o 1
        czas->tv_nsec = nanosekundy - 1000000000;						//a ilosc nanosekund czasu to roznica miedzy zmienna nanosekundy i 1000000000
    }
    else {											//w przeciwnym wypadku 
      czas->tv_nsec = nanosekundy;								//liczba nanosekund czasu jest rowna zmiennej nanosekundy
    }
}

void aktualizujWskazaniaStopera(Stoper *stoper) {	//funkcja zwieksza wskazanie stopera o 1 setna przy kazdym wywolaniu
    stoper->setne++;					//setne stopera zwiekszane sa o 1
    if (stoper->setne > 99) {				//jesli liczba setnych po inkrementacji jest wieksza niz 99
        stoper->setne = 0;				//to ustawiana jest na 0
        stoper->sekundy++;				//i sekundy stopera zwiekszane sa o 1
        if (stoper->sekundy > 59) {			//jesli liczba sekund po inkrementacji jest wieksza niz 59 	
            stoper->sekundy = 0;			//to ustawiana jest na 0
            stoper->minuty++;				//i minuty stopera zwiekszane sa o 1
            if (stoper->minuty > 59) {			//jesli liczba minut po inkrementacji jest wieksza niz 59	
                stoper->minuty = 0;			//to ustawiana jest na 0
                stoper->godziny++;			//i godziny stopera zwiekszane sa o 1
                if (stoper->godziny > 23) {		//jesli liczba godzin po inkrementacji przekroczy wartosc 23	
                    stoper->godziny = 0;		//to ustawiana jest na 0
                    stoper->stan = ZRESETOWANY;		//i stan stopera automatycznie ustawiany jest na ZRESETOWANY	
                }
            }
        }
    }
}

void wyswietlBiezacyCzas(Stoper *stoper) {			//funkcja wyswietla biezace wskazanie stopera	
    move(kursorKonsoliPozycjaY, kursorKonsoliPozycjaX);		//pozycja kursora Konsoli ustawiana jest na taka jaka byla podczas wyjscia z funkcji wyswietlStoper
    formatujIWyswietlSkladowaCzasu(stoper->godziny);		//wyswietlane sa godziny stopera	
    printw(" : ");						//wyswietlany jest dwukropek
    formatujIWyswietlSkladowaCzasu(stoper->minuty);		//wyswietlane sa minuty stopera
    printw(" : ");						//wyswietlany jest przecinek
    formatujIWyswietlSkladowaCzasu(stoper->sekundy);		//wyswietlana sa sekundy stopera	
    printw(" : ");						//wyswietlany jest przecinek
    formatujIWyswietlSkladowaCzasu(stoper->setne);		//wyswietlane sa setne stopera  	
    refresh();							//wszystkie powyzsze wyswietlenia tyczyly sie ekranu wirtualnego a teraz wyswietlane sa na rzeczywistym
}

void formatujIWyswietlSkladowaCzasu(const unsigned int skladowa) {	//funkcja wyswietla skladowa biezacego wskazania stopera w odpowiednio sformatowany sposob
    if (skladowa < 10) {						//jesli skladowa jest jednocyfrowa
        printw("0");							//nalezy wyswietlic przed nia 0
    }
    printw("%u", skladowa);						//nastepnie wyswietlana jest skladowa
}
