/*
 * Praktikum MRT2 
 * ART1 Türsteuerung
 * Institut für Automatisierungstechnik
 * letztes Update Mai 2018
 * Author: M. Herhold
 * Version: r2
 */

#ifndef DOOR_INTERFACE_HH
#define DOOR_INTERFACE_HH

#include <thread>
#include <string>
#include <usb.h>
#include <atomic>


/* 
 * DoorInterface deklariert Funktionen und atomare Variablen zur Benutzung des
 * Timers und des E/A Geraetes. */
class DoorInterface{
public:
	/* 
	 * Boolsche atomare Variable zum Detektieren des gewünschten Programmabbruchs
	 * set to true to request programm exit
	 * check value to know if programm exit is requested
	 */
	static std::atomic<bool> quit_doorcontrol_flag;


	/*
	 * Konstruktor des Interface zur Hardware / Simulation
	 * real_door: legt fest ob auf die Hardware zugegriffen werden soll
	 * show_ui:   legt fest, ob das ncurses-User-Interface genutzt wird
	 *
	 * If "show_ui" is true, then use
	 * "External Tools" -> run in xterm" to execute from Eclipse IDE
	 */
	DoorInterface(bool real_door=false, bool show_ui=true);
	
	/* D'tor */
	~DoorInterface(void);

	
	/*
	 * notwendige Initialisierung der Hardware / Simulation / UI mit
	 * Rückgabewert zu	- Erfolg positiver Wert
	 *			- Misserfolg: negativer errno Wert
	 * opens usb devices and starts UI thread,
	 * returnvalue: negative value means unrecoverable error
	 */
	int SecondLevelInit(void);


	/*
	 * Liest die Eingabe-Kanaele der Karte.
	 * Die Kanaele sind den Bits von in_channels direkt zugeordnet,
	 * d.h. Kanal 0 liegt auf Bit 0 (niederwertigstes Bit), usw.
	 */
	void DIO_Read(int *in_channels);
	

	/*
	 * Schreibt alle Ausgabe Kanaele auf die Digital-EA-Karte.
	 * Die Kanaele sind den Bits von out_channels direkt zugeordnet,
	 * d.h. Kanal 0 liegt auf Bit 0 (niederwertigstes Bit), usw.
	 */
	void DIO_Write(int out_channels);


	/* 
	 * Startet den Timer und wartet seconds Sekunden.
	 * (Richtwert: 0.03 ... 0.5 s)
	 * Werte darunter fuehren je nach Rechner zum Ueberlauf.
	 */
	int StartTimer(double seconds);

	
	/* 
	 * Startet den Timer und programmiert ihn auf das Zeitintervall
	 * seconds (in Sekunden) und ordnet dem Timer-Signal die Handler-
	 * Funktion void handler(int) zu. Diese wird periodisch bei jedem
	 * Timer-Ueberlauf aufgerufen.
	 */
	int StartTimerWithISR(double seconds, void (*handler)(int));
	

	/* 
	 * Methode zum Anzeigen eines String s auf dem ncurses-User-Interface
	 * (maximale Länge 50 alphanumerische Zeichen) 
	 * set debug string for showing in Simulation UI */
	void DebugString(const std::string s);

private:
	const bool real_door;
	const bool show_ui;
	
	std::thread ui_thread;
	char debug_string[100];
	
	struct usb_device *dev;
	struct usb_dev_handle *handle;
	
	int channels;
	int sim_channels;
	
	void HandleSimUI(void);
	void InitNcurses(void);
	void ShowSimUI(void);
	void HandleSimInput(int c);
};

#endif // DOOR_INTERFACE_HH
