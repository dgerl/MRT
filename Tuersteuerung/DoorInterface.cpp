/*
 * Praktikum MRT2 
 * ART1 Türsteuerung
 * Institut für Automatisierungstechnik
 * letztes Update Mai 2018
 * Co-Autor: M. Herhold
 * Version: r3
 */

#include "DoorInterface.h"
#include <chrono>
#include "Library/niusb6501.h"
#include <ncurses.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>

// failsave if exit() function gets called by class user
// it is not encouraged to use exit()
static void end_ncurses(void)
{endwin();}

//static member for exit requests
std::atomic<bool> DoorInterface::quit_doorcontrol_flag(false);


DoorInterface::DoorInterface(bool real_door, bool show_ui) : real_door(real_door),
							     show_ui(show_ui),
							     ui_thread(),
							     channels(0xFFFF),
							     sim_channels(0)
{
	if(show_ui){
		InitNcurses();
	}
	
}

DoorInterface::~DoorInterface(void)
{
	if(show_ui){
		quit_doorcontrol_flag=true;
		if(ui_thread.joinable()) ui_thread.join();
		endwin();
	}
}

int DoorInterface::SecondLevelInit(void){
	if(show_ui)
		ui_thread = std::thread(&DoorInterface::HandleSimUI, this);

	if(!real_door)
		return 1;
	
	if( niusb6501_list_devices(&dev, 1) != 1) {
		fprintf(stderr, "Device not found\n");
		return -ENODEV;
	}
	handle = niusb6501_open_device(dev);
	if (handle == NULL){
		fprintf(stderr, "Unable to open the USB device: %s\n", strerror(errno));
		return -errno;
	}
	niusb6501_set_io_mode(handle, 0x03, 0x00, 0xFF);
	
	return 1;
}


void DoorInterface::HandleSimUI(void){
	while(!quit_doorcontrol_flag){
		HandleSimInput(getch());
		ShowSimUI();
		std::this_thread::sleep_for( std::chrono::milliseconds(40) );
	}
}

void DoorInterface::InitNcurses(void)
{
	atexit(end_ncurses);
	initscr();		// initialisiere die curses Bibliothek
	keypad(stdscr, TRUE);	// schalte das Tastatur-Mapping ein
	raw();			// take input chars one at a time, no wait for \n
	curs_set(0);		// Cursor ausschalten
	noecho();		// echo input
	nodelay(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);
	init_pair(4, COLOR_RED, COLOR_WHITE);
	bkgd(COLOR_PAIR(3));

	DebugString("unset");
}

void DoorInterface::DebugString(const std::string s)
{
	//sprintf(debug_string, "%.50s", s.c_str());
	int i=0, j=0;
	const char* tmp = s.c_str();
	int j_max = s.size();
	
	while(i < 50){ //needed to clean ncurses
		if( (j < j_max) && (isalnum( tmp[j] )) ){
			debug_string[i] = tmp[j];
		} else {
			debug_string[i] = ' ';
		}
		i++;
		j++;
	}
	debug_string[i]='\0';

	#if 0
	std::ofstream logfile;
	logfile.open ("interface.log", std::ios::out | std::ios::app);
	logfile << debug_string << std::endl;
	logfile.close();
	#endif
}


// zeigt die Simulatoroberfläche
// TODO entferne MagicNumbers
void DoorInterface::ShowSimUI(void)
{
	//Anzeige des Betriebsart
	color_set(4, 0);
	if( ((channels >> 0) & 1)) {
		if( ((channels >> 1) & 1))
			mvprintw(1, 30, "Automatikbetrieb    ");
		else
			mvprintw(1, 30,"Handbetrieb         ");
	} else {
		if( ((channels >> 1) & 1))
			mvprintw(1, 30, "Reparaturbetrieb    ");
		else
			mvprintw(1, 30, "Prozesssteuerung aus");
	}
	
	color_set(3, 0);
	attron(A_BOLD);
	color_set(3, 0);
	mvprintw(5, 9, "Eingänge");
	mvprintw(5, 53, "Ausgänge");
	attroff(A_BOLD);

	//Eingänge
	color_set(1, 0);
	mvprintw(7, 9, " S1  S2  E1  E2  X1  X2  X3  LS1 LS2 BE  B  ");	
	move(8, 9);
	for (int i=0; i<11; i++)
		printw(" %ld  ", (channels>>i)&1);
	printw("");

	//Ausgänge
	color_set(2, 0);			
	mvprintw(7, 55, " Y1 Y2 Y3  Bedeutung");
	mvprintw(8, 55, "");
	for (int i=0; i<3; i++)
		printw(" %ld ", (sim_channels>>i)&1);
	printw("  ");

	//Bedeutung der Ausgänge
	if( ((sim_channels >> 0) & 1) == 1) {
		mvprintw(8, 66, " open  <-");
	} else {
		if(  ((sim_channels >> 1) & 1) &&
		    (((sim_channels >> 2) & 1) == 1) ) {
			mvprintw(8, 66, " close ->");
		} else
			mvprintw(8, 66, " stop    ");
	}
	
	color_set(3,0);
	//Tastenbelegung
	mvprintw(9, 2, " Shift+ F1  F2  F3  F4  F5  F6  F7  F8  F9  F10 F11");

	//Erklärung
	attron(A_BOLD);

	mvprintw(11, 15, "Kürzel - Taste - Bedeutung");
	attroff(A_BOLD);
	mvprintw(12, 15, "  S1   -  F1   - Schlüsselschalter 1");
	mvprintw(13, 15, "  S2   -  F2   - Schlüsselschalter 2");
	mvprintw(14, 15, "  E1   -  F3   - Taster am Rahmen AUF");
	mvprintw(15, 15, "  E2   -  F4   - Taster am Rahmen ZU");
	mvprintw(16, 15, "  X1   -  F5   - Endlagenschalter links");
	mvprintw(17, 15, "  X2   -  F6   - Sicherheitsschalter");
	mvprintw(18, 15, "  X3   -  F7   - Endlagenschalter rechts");
	mvprintw(19, 15, "  LS1  -  F8   - Lichtschranke zylindrisch");
	mvprintw(20, 15, "  LS2  -  F9   - Lichtschranke kubisch");
	mvprintw(21, 15, "  BE   -  F10  - Bewegungsmelder");
	mvprintw(22, 15, "  B    -  F11  - Bumper");

	mvprintw(24, 15, "Debug  - %.50s", debug_string);
	
	move(2, 14);
	refresh();

}

//TODO magic numbers entfernen
void DoorInterface::HandleSimInput(int c)
{
	if (c == 'q') { 	//'q' = 113
		// quit request via keyboard
		quit_doorcontrol_flag=true;		
	}
	
	if(real_door)
		return;
	
	
	int chan, cbit;
	// Switch channels
	// SHIFT+{F1-F12}
	//mvprintw(15, 10, "%i ", c); //Debug
	if( (c >= 277) && (c <= 288) ){
		chan = c - 277;
		cbit = (channels >> chan) & 1; // ist der Channel z.Zt. gesetzt?
		if (cbit)
			channels = channels ^ (0x1L << chan);
		else
			channels = channels | (0x1L << chan);
	}

}

void DoorInterface::DIO_Read(int *in_channels)
{
	if( !real_door ){
		*in_channels = channels;
		return;
	}
	
	unsigned char p0, p1;
	//char s0[9], s1[9];
	int status;
	
	//niusb6501_set_io_mode(handle, 0xff, 0x00, 0xFF);
	status = niusb6501_read_port(handle, 0, &p0);
	if(status) {
		fprintf(stderr, "error read port 0: %s\n", strerror(-status));
		//return status;
	}
	status = niusb6501_read_port(handle, 1, &p1);
	if(status) {
		fprintf(stderr, "error read port 1: %s\n", strerror(-status));
		//return status;
	}
	*in_channels = (int)p0 + ((int)p1 << 8);
	channels = *in_channels;
	
	//return 1;
}

void DoorInterface::DIO_Write(int out_channels)
{
	sim_channels = out_channels;

	if( !real_door ){
		return;
	}
	
	unsigned char p;
	int status;
	
	p = ((char)out_channels ^ 0xFF);
	status = niusb6501_write_port(handle, 2, p);
	if(status) {
		fprintf(stderr, "error write port 2: %s\n", strerror(-status));
		//return status;
	}

	//return 1;
}

int DoorInterface::StartTimer(double seconds)
{
	// Der Teil vor dem Komma: Sekunden
	int vor_dem_komma = (int)seconds;

	// Der Teil nach dem Komma: Nanosekunden
	int nach_dem_komma = (int)((seconds - (double)vor_dem_komma) * 1000000000) ;

	// Speichert den aktuellen Stand des Timers
	int akt_s, akt_ns = 0;

	// Datenstruktur zur näheren Klassifikation eines „Signalereignisses“
	struct sigevent ereignis;

	// Der Timer (ID des Timers)
	timer_t wecker_id;

	// wecker_sett: 	nimmt Startzeitpunkt und die Wiederholunsperiode auf
	// wecker_status:	nimmt die aktuelle Position des Timers auf
	struct itimerspec wecker_sett, wecker_status;

	// Dauer der ersten Runde nach starten des Timers
	wecker_sett.it_value.tv_sec = vor_dem_komma;
	wecker_sett.it_value.tv_nsec = nach_dem_komma;

	// Dauer der nachfolgenden Runden (beides 0: Timer läuft nur einmal)
	wecker_sett.it_interval.tv_sec = 0;
	wecker_sett.it_interval.tv_nsec = 0;

	// Das Signalereignis wird initialisiert und dann ausgeschaltet
	ereignis.sigev_value.sival_int = 0; // initialisieren
	ereignis.sigev_notify = SIGEV_NONE; // ausschalten


	// Den Timer erstellen
	timer_create((int)CLOCK_REALTIME, &ereignis, &wecker_id);

	// In wecker_sett gespeicherte Einstellungen anwenden
	timer_settime(wecker_id, 0, &wecker_sett, NULL);

	// Abfragen, ob die Zeit bereits abgelaufen ist
	do{	
		timer_gettime(wecker_id, &wecker_status);
		akt_s = wecker_status.it_value.tv_sec;
		akt_ns = wecker_status.it_value.tv_nsec;
		// Debug
		// printw("Zeitpunkt: s.ns %i.%i\n",akt_s,akt_ns);
	}while( (akt_s > 0) || (akt_ns > 0) );

	// Timer löschen
	timer_delete(wecker_id);
	
	return 0;
}

int StartTimerWithISR(double seconds, void (*handler)(int)) {

	// Der Teil vor dem Komma: Sekunden
	int vor_dem_komma = (int)seconds;

	// Der Teil nach dem Komma: Nanosekunden
	int nach_dem_komma = (int)((seconds - (double)vor_dem_komma) * 1000000000) ;

	// Datenstruktur zur näheren Klassifikation eines „Signalereignisses“
	struct sigevent isr_ereignis;

	// Datenstruktur zur Anmeldung von Signalbehandlungen
	struct sigaction isr_sig_aktion;

	// Der Timer (ID des Timers)
	timer_t isr_wecker_id;

	// wecker_sett: 	nimmt Startzeitpunkt und die Wiederholunsperiode auf
	// wecker_status:	nimmt die aktuelle Position des Timers auf
	struct itimerspec isr_wecker_sett;

	// Dauer der ersten Runde nach starten des Timers
	isr_wecker_sett.it_value.tv_sec = vor_dem_komma;
	isr_wecker_sett.it_value.tv_nsec = nach_dem_komma;

	// Dauer der nachfolgenden Runden (beides 0: Timer läuft nur einmal)
	isr_wecker_sett.it_interval.tv_sec = vor_dem_komma;
	isr_wecker_sett.it_interval.tv_nsec = nach_dem_komma;

	// Das Signalereignis wird initialisiert und dann das zu sendene Signal eingestellt
	isr_ereignis.sigev_value.sival_int = 0; // spezifiziert einen anwendungsdefinierten Wert, der bei der
						// Auslieferung übergeben wird
	isr_ereignis.sigev_signo = SIGUSR1; // dieses Signal senden
	isr_ereignis.sigev_notify = SIGEV_SIGNAL; // Signalisieren einschalten

	sigemptyset(&isr_sig_aktion.sa_mask); // Signalset erstellen
	isr_sig_aktion.sa_handler = handler; // diese Funktion aufrufen wenn das Signal kommt
	isr_sig_aktion.sa_flags = 0; // keine zusätzlichen Flags

	sigaction(SIGUSR1, &isr_sig_aktion, 0);

	// Den Timer erstellen
	timer_create((int)CLOCK_REALTIME, &isr_ereignis, &isr_wecker_id);

	// In wecker_sett gespeicherte Einstellungen anwenden
	timer_settime(isr_wecker_id, 0, &isr_wecker_sett, NULL);

	return 1;
}
