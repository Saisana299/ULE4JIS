
#include "stdafx.h"
#include "PressAndReleaseDecorator.h"

PressAndReleaseDecorator::PressAndReleaseDecorator(std::unique_ptr<Emulation> emulation) : emulation(std::move(emulation)) {
}

PressAndReleaseDecorator::~PressAndReleaseDecorator() {
}

void PressAndReleaseDecorator::executeDown(const KeyEmulator &emulator, const KeyCondition &cond) {
	this->emulation->executeDown(emulator, cond);
	this->emulation->executeUp(emulator, cond);
}

void PressAndReleaseDecorator::executeUp(const KeyEmulator &emulator, const KeyCondition &cond) {
	this->emulation->executeDown(emulator, cond);
	this->emulation->executeUp(emulator, cond);
}
