#pragma once
#include "Emulation.h"
#include "KeyEmulator.h"
#include <memory>

class ShiftReleaseDecorator : public Emulation {
private:
	std::unique_ptr<Emulation> emulation;

public:
	ShiftReleaseDecorator(std::unique_ptr<Emulation> emulation);
	virtual ~ShiftReleaseDecorator();
	virtual void executeDown(const KeyEmulator &emulator, const KeyCondition &cond);
	virtual void executeUp(const KeyEmulator &emulator, const KeyCondition &cond);
};
