#include "Weapons.h"

bool WeaponComputer::next_instr()
{
	if (instructions.size() == 0) return false;

	while (instructions.at(programCounter).empty)
	{
		programCounter++;

		if (programCounter > instructions.size() - 1) programCounter = 0;

		emptyCnt++;
		if (emptyCnt == storage) { emptyCnt = 0; return false; }
	}
	emptyCnt = 0;
	return true;
}

void WeaponComputer::set_register_values()
{
	for (auto& reg : registers)
	{
		int bits = reg.bit - reg._signed_;

		int max = 0;
		for (size_t i = 0; i < bits; i++) max |= 1 << i;

		if (!reg._signed_)
		{
			if (reg.value < 0) reg.value = max - (-reg.value % (max + 1));
			if (reg.value > max) reg.value = reg.value % (max + 1);
		}
		else
		{
			if (reg.value > max) reg.value = -max + (reg.value % (max + 1));
			if (reg.value < -max) reg.value = max - (-reg.value % (max + 1));
		}
	}
}

void WeaponComputer::skip()
{
	skipCnt++;
	if (skipCnt > storage)
	{
		skipCnt = 0;
		return;
	}
	next_instr();
	cycle();
	skipCnt = 0;
}

void WeaponComputer::cycle()
{
	WeaponInstruction& instruction = instructions.at(programCounter);

	if (instruction.empty) return;

	for (size_t i = 0; i < instruction.inputs.size(); i++)
	{
		bool isReg = instruction.isRegisterValue.at(i);

		if (isReg)
		{
			if (instruction.inputs.at(i) < 0) continue;

			instruction.values.at(i) = registers.at(instruction.inputs.at(i)).value;
		}
		else
		{
			instruction.values.at(i) = instruction.inputs.at(i);
		}
	}

	instruction.instructionFunction(*this, instruction);

	if (!skipCounterIncr)
		programCounter++;
	else
		skipCounterIncr = false;
}

void WeaponComputer::tick()
{
	uint32_t timeSinceLast = SDL_GetTicks() - clockLast;

	if (timeSinceLast > clockSpeed)
	{
		cycle();
		set_register_values();
		next_instr();
		clockLast = SDL_GetTicks();
	}
}
