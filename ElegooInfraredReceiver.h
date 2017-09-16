#ifndef __ELEGOO_INFRARED_RECEIVER_H__
#define __ELEGOO_INFRARED_RECEIVER_H__

#include <Arduino.h>
#include <IRremote.h>

#include "ElegooMoveCommand.h"
#include "ElegooInfraredConfigInterface.h"
#include "ElegooCarConfig.h"

class ElegooInfraredReceiver
{
private:

	IRrecv * irrecv = 0;

	ElegooCarConfig::InfraredReceiverConfig & config;

	ElegooInfraredConfigInterface ** infraredConfigs;

	int numInfraredConfigs = 0;

public:

	ElegooInfraredReceiver(ElegooCarConfig::InfraredReceiverConfig & pConfig) :
			config(pConfig)
	{
		int size = config.MAX_NUM_RECEIVERS;
		infraredConfigs = new ElegooInfraredConfigInterface*[size];
	}

	void setup()
	{
		pinMode(config.RECEIVER_PIN, INPUT);
		irrecv = new IRrecv(config.RECEIVER_PIN);
		irrecv->enableIRIn();
	}

	void registerInfraredConfig(ElegooInfraredConfigInterface * infraredConfig)
	{
		infraredConfigs[numInfraredConfigs] = infraredConfig;
		numInfraredConfigs++;
	}

	// May also return UNKNOWN_CMD or NO_COMMAND
	ElegooMoveCommand readCommand()
	{
		ElegooMoveCommand resultsCommand = ElegooMoveCommand::NO_COMMAND;

		decode_results results;
		while (irrecv->decode(&results)) // read all infrared input which we have
		{
			unsigned long resultsValue = results.value;
			irrecv->resume();
			delay(150);

			Serial.print("Infrared result: ");
			Serial.println(resultsValue);

			ElegooMoveCommand moveCommand = checkInfraredProviders(resultsValue); // check for known code
			if (!ElegooMoveCommandUtil::isValidCommand(resultsCommand)) // if the search result is still not valid
			{
				// we don't return yet, we continue to empty the currently pending queue of IR signals,
				// the below assignment, is performed until we have found an 'isValidCommand'
				resultsCommand = moveCommand;
			}
		}

		return resultsCommand;
	}

private:

	// May return UNKNOWN_CMD, will never return NO_COMMAND
	ElegooMoveCommand checkInfraredProviders(unsigned long resultsValue)
	{
		for (int i = 0; i < numInfraredConfigs; i++)
		{
			ElegooInfraredConfigInterface * infraredConfig = infraredConfigs[i];
			ElegooMoveCommand moveCommand = infraredConfig->checkCommand(resultsValue);
			if (moveCommand != ElegooMoveCommand::UNKNOWN_CMD)
			{
				return moveCommand;
			}
		}

		return ElegooMoveCommand::UNKNOWN_CMD;
	}
};

#endif

