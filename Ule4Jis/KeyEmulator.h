#pragma once

#include "EmulationStrategy.h"
#include "KeyHooker.h"
#include "KeyHookEventListener.h"
#include "KeyCondition.h"
#include <memory>

class KeyEmulator : public KeyHookEventListener {
private:
	EmulationMapType emulationMap;
	std::unique_ptr<KeyHooker> hooker;
	KeyCondition keyCondition;

private:
	bool isExtendedKey(BYTE vkey) const;

public:
	KeyEmulator(EmulationStrategy *strategy);
	~KeyEmulator();

	void changeEmulationStrategy(EmulationStrategy *strategy);
	void start();
	void end();
	virtual bool onKeyHookEvent(const KeyHookEventArgs &args);
	bool isStarted() const { return (this->hooker != nullptr); }

	void emulateKey(BYTE vkey, bool up = false) const;
};
