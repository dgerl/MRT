/*
 * Praktikum MRT2 
 * ART1 Türsteuerung
 * Institut für Automatisierungstechnik
 * letztes Update Mai 2018
 * Autor: M.Herhold
 * Version: r2
 */

#ifndef DOORCONTROL_HH
#define DOORCONTROL_HH

#include "ncurses.h"
#include "DoorInterface.h"


class DoorControl {
public:
	DoorControl();
	~DoorControl();
	
	void run();

	void getBetriebsart();
	void PositionsErmittlung();

	bool Lichtschranke();
	bool Bumper();
	bool Bewegungsmelder();
	
	bool test_schliessen();
	bool test_oeffnen();

	void oeffnenTuer();
	void schliessenTuer();
	void anhaltenTuer();

	void Steuerschleife();
	
private:
	DoorInterface door_if;
};


#endif // DOORCONTROL_HH
