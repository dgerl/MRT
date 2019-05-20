	/*
 * Praktikum MRT2 
 * ART1 Türsteuerung
 * Institut für Automatisierungstechnik
 * letztes Update Mai 2018
 * Autor: M.Herhold
 * Version: r2
 */

#include "DoorInterface.h"
#include "DoorControl.h"
#include <stdio.h>
#include <stdlib.h>

//Enums für Betriebsarten und Zustände

typedef enum{Automatik, Hand, Reparatur, PST_aus} Def_Betriebsart;
typedef enum{Tuer_schliessen, Tuer_zu, Tuer_oeffnen, Tuer_offen} Def_Zustand;
typedef enum{oeffnen, schliessen, warten} Def_Funktion;
typedef enum{offen, zu, undef} Def_Position;

//Kanäle der verschiedenen Schalter

const int
	//Schlüsselschalter 
	S1 = 1, 
	S2= 2,

	//Bedienfeld Taster
	E1= 4,
	E2= 8,

	//Endlagenschalter
	X1= 16,//Tuer zu
	X2= 32,//Tuer zu, Sicherheitsschalter
	X3= 64,//Tuer auf
	
	//Lichtschranken
	LS1 = 128,
	LS2 = 256,

	//Bewegungsmelder
	BE = 512,

	//Bumper
	B = 1024,

	//Ausgänge
	Y1 = 1, //Tür öffnen
	Y2 = 2, //Tür schliessen
	Y3 = 4; //Warnleuchte
	
//Variablen
int in_channels;
int out_channels;
int wartezeit = 0;


	
//Initialwerte
Def_Betriebsart Betriebsart = PST_aus;
Def_Zustand Zustand = Tuer_zu;
Def_Funktion Funktion = schliessen;
Def_Position Position = undef;

//Vorgegebene Methoden

DoorControl::DoorControl() : door_if(true, true)
{
	door_if.SecondLevelInit();

}

DoorControl::~DoorControl()
{
	door_if.quit_doorcontrol_flag = true;
}

void DoorControl::run()
{
	int i = 50;

	door_if.DebugString("Please wait 10 Seconds for auto exit, or press 'q'.");
	while( !door_if.quit_doorcontrol_flag && i-- ){
		door_if.StartTimer(0.2);
	}
}


//Betriebsart ermitteln
void DoorControl::getBetriebsart(){
	door_if.DIO_Read(&in_channels);
	//S1&&S2: Automatik
	if((S1&in_channels)&&(S2&in_channels)){
		Betriebsart = Automatik;
	//S1&&!S2: Hand
	}else if((S1&in_channels)&&(!(S2&in_channels))){
		Betriebsart = Hand;
	//!S1&&S2: Reparatur
	}else if((!(S1&in_channels))&&(S2&in_channels)){
		Betriebsart = Reparatur;
	//!S1&&!S2: Aus	
	}else if((!(S1&in_channels))&&(!(S2&in_channels))){
		Betriebsart = PST_aus;		
	}
}

//Zustand ermitteln
void DoorControl::PositionsErmittlung(){
	door_if.DIO_Read(&in_channels);
	
	if(!(in_channels&X3)){
		Position = offen;
	}else if((!(in_channels&X2))&&(!(in_channels&X1))){
		Position = zu;	
	}else{
		Position = undef;
	}
}

//Testen ob Lichtschranke auslöst
bool DoorControl::Lichtschranke(){
	door_if.DIO_Read(&in_channels);
	if((!(in_channels&LS1))||(!(in_channels&LS2))){
		return true;
	}else{
		return false;
	}
}

//Testen ob Bumper auslöst

bool DoorControl::Bumper(){
	door_if.DIO_Read(&in_channels);
	if((!(in_channels&B))){
		return true;
	}else{
		return false;
	}
}

//Testen ob Bewegungsmelder auslöst
bool DoorControl::Bewegungsmelder(){
	door_if.DIO_Read(&in_channels);
	if((!(in_channels&BE))){
		return true;
	}else{
		return false;
	}
}

//Schliesstest

bool DoorControl::test_schliessen(){
	if((!(in_channels&BE))){
		return true;
	}else{
		return false;
	}
}

bool DoorControl::test_oeffnen(){
	if((!(in_channels&BE))){
		return true;
	}else{
		return false;
	}
}


//Öffnen, Schliessen und Anhalten der Tür

void DoorControl::oeffnenTuer(){
	door_if.DIO_Write(Y1);
	Funktion = oeffnen;
}

void DoorControl::schliessenTuer(){
	door_if.DIO_Write(Y2|Y3);
	Funktion = schliessen;
}

void DoorControl::anhaltenTuer(){
	door_if.DIO_Write(~Y1&~Y2&~Y3);
	Funktion = warten;
}


//Steuerschleife
void DoorControl::Steuerschleife(){
	getBetriebsart();
	PositionsErmittlung();
	
	switch(Zustand){
		case Tuer_zu:{
			if(test_oeffnen()){
				oeffnenTuer();
				Zustand = Tuer_oeffnen;
			}
			break;
		}
		case Tuer_offen:{
			wartezeit++;
			if(test_schliessen()){
				schliessenTuer();
				Zustand = Tuer_schliessen;
			}
			break;
		}
		case Tuer_oeffnen:{
			wartezeit = 0;
			if(Position == offen){
				anhaltenTuer();
				Zustand = Tuer_offen;
			}
			if(test_oeffnen()){
				oeffnenTuer();
			}
			break;
		}
		case Tuer_schliessen:{
			if(Position == zu){
				anhaltenTuer();
				Zustand = Tuer_zu;
			}
			if(test_oeffnen()){
				oeffnenTuer();
				Zustand = Tuer_oeffnen;
			}
			if(test_schliessen()){
				schliessenTuer();
				Zustand = Tuer_schliessen;
			}
			break;		
		}
	}	

}
	


/* If "show_ui" of class DoorInterface is active use "External Tools" -> run in xterm"
 * to execute from Eclipse IDE */
int main (int argc, char *argv[])
{
	// ... insert your class initialisation and loop code here ...
	// example start:

	DoorControl control;
	control.run();
	
	

	
	
	
	return 0;
}


